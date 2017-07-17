#include <mongoc.h>
#include <mongoc-uri-private.h>

#include "mongoc-client-private.h"
#include "mongoc-util-private.h"
#include "TestSuite.h"

#include "test-libmongoc.h"
#include "mock_server/mock-server.h"
#include "mock_server/future.h"
#include "mock_server/future-functions.h"
#include "test-conveniences.h"

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "topology-test"


static void
test_topology_client_creation (void)
{
   mongoc_uri_t *uri;
   mongoc_topology_scanner_node_t *node;
   mongoc_topology_t *topology_a;
   mongoc_topology_t *topology_b;
   mongoc_client_t *client_a;
   mongoc_client_t *client_b;
   mongoc_stream_t *topology_stream;
   mongoc_server_stream_t *server_stream;
   bson_error_t error;

   uri = test_framework_get_uri ();
   mongoc_uri_set_option_as_int32 (uri, "localThresholdMS", 42);
   mongoc_uri_set_option_as_int32 (uri, "connectTimeoutMS", 12345);
   mongoc_uri_set_option_as_int32 (uri, "serverSelectionTimeoutMS", 54321);

   /* create two clients directly */
   client_a = mongoc_client_new_from_uri (uri);
   client_b = mongoc_client_new_from_uri (uri);
   assert (client_a);
   assert (client_b);

#ifdef MONGOC_ENABLE_SSL
   test_framework_set_ssl_opts (client_a);
   test_framework_set_ssl_opts (client_b);
#endif

   /* ensure that they are using different topologies */
   topology_a = client_a->topology;
   topology_b = client_b->topology;
   assert (topology_a);
   assert (topology_b);
   assert (topology_a != topology_b);

   assert (topology_a->local_threshold_msec == 42);
   assert (topology_a->connect_timeout_msec == 12345);
   assert (topology_a->server_selection_timeout_msec == 54321);

   /* ensure that their topologies are running in single-threaded mode */
   assert (topology_a->single_threaded);
   assert (topology_a->scanner_state == MONGOC_TOPOLOGY_SCANNER_OFF);

   /* ensure that we are sharing streams with the client */
   server_stream = mongoc_cluster_stream_for_reads (&client_a->cluster,
                                                    NULL, &error);

   ASSERT_OR_PRINT (server_stream, error);
   node = mongoc_topology_scanner_get_node (client_a->topology->scanner,
                                            server_stream->sd->id);
   assert (node);
   topology_stream = node->stream;
   assert (topology_stream);
   assert (topology_stream == server_stream->stream);

   mongoc_server_stream_cleanup (server_stream);
   mongoc_client_destroy (client_a);
   mongoc_client_destroy (client_b);
   mongoc_uri_destroy (uri);
}

static void
test_topology_client_pool_creation (void)
{
   mongoc_client_pool_t *pool;
   mongoc_client_t *client_a;
   mongoc_client_t *client_b;
   mongoc_topology_t *topology_a;
   mongoc_topology_t *topology_b;

   /* create two clients through a client pool */
   pool = test_framework_client_pool_new ();
   client_a = mongoc_client_pool_pop (pool);
   client_b = mongoc_client_pool_pop (pool);
   assert (client_a);
   assert (client_b);

   /* ensure that they are using the same topology */
   topology_a = client_a->topology;
   topology_b = client_b->topology;
   assert (topology_a);
   assert (topology_a == topology_b);

   /* ensure that that topology is running in a background thread */
   assert (!topology_a->single_threaded);
   assert (topology_a->scanner_state != MONGOC_TOPOLOGY_SCANNER_OFF);

   mongoc_client_pool_push (pool, client_a);
   mongoc_client_pool_push (pool, client_b);
   mongoc_client_pool_destroy (pool);
}

static void
test_server_selection_try_once_option (void *ctx)
{
   const char *uri_strings[3] = {
      "mongodb://a",
      "mongodb://a/?serverSelectionTryOnce=true",
      "mongodb://a/?serverSelectionTryOnce=false" };

   unsigned long i;
   mongoc_client_t *client;
   mongoc_uri_t *uri;
   mongoc_client_pool_t *pool;

   /* try_once is on by default for non-pooled, can be turned off */
   client = mongoc_client_new (uri_strings[0]);
   assert (client->topology->server_selection_try_once);
   mongoc_client_destroy (client);

   client = mongoc_client_new (uri_strings[1]);
   assert (client->topology->server_selection_try_once);
   mongoc_client_destroy (client);

   client = mongoc_client_new (uri_strings[2]);
   assert (! client->topology->server_selection_try_once);
   mongoc_client_destroy (client);
   
   /* off for pooled clients, can't be enabled */
   for (i = 0; i < sizeof (uri_strings) / sizeof (char *); i++) {
      uri = mongoc_uri_new ("mongodb://a");
      pool = mongoc_client_pool_new (uri);
      client = mongoc_client_pool_pop (pool);
      assert (!client->topology->server_selection_try_once);
      mongoc_client_pool_push (pool, client);
      mongoc_client_pool_destroy (pool);
      mongoc_uri_destroy (uri);
   }
}

static void
_test_server_selection (bool try_once)
{
   mock_server_t *server;
   char *secondary_response;
   char *primary_response;
   mongoc_uri_t *uri;
   mongoc_client_t *client;
   mongoc_read_prefs_t *primary_pref;
   future_t *future;
   bson_error_t error;
   request_t *request;
   mongoc_server_description_t *sd;

   server = mock_server_new ();
   mock_server_set_request_timeout_msec (server, 600);
   mock_server_run (server);

   secondary_response = bson_strdup_printf (
      "{'ok': 1, "
      " 'ismaster': false,"
      " 'secondary': true,"
      " 'setName': 'rs',"
      " 'hosts': ['%s']}",
      mock_server_get_host_and_port (server));

   primary_response = bson_strdup_printf (
      "{'ok': 1, "
      " 'ismaster': true,"
      " 'setName': 'rs',"
      " 'hosts': ['%s']}",
      mock_server_get_host_and_port (server));

   uri = mongoc_uri_copy (mock_server_get_uri (server));
   mongoc_uri_set_option_as_utf8 (uri, "replicaSet", "rs");
   mongoc_uri_set_option_as_int32 (uri, "heartbeatFrequencyMS", 500);
   mongoc_uri_set_option_as_int32 (uri, "serverSelectionTimeoutMS", 100);
   if (!try_once) {
      /* serverSelectionTryOnce is on by default */
      mongoc_uri_set_option_as_bool (uri, "serverSelectionTryOnce", false);
   }

   client = mongoc_client_new_from_uri (uri);
   primary_pref = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   /* no primary, selection fails after one try */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);
   request = mock_server_receives_ismaster (server);
   assert(request);
   mock_server_replies_simple (request, secondary_response);
   request_destroy (request);

   /* the selection timeout is 100 ms, and we can't rescan until a half second
    * passes, so selection fails without another ismaster call */
   assert (!mock_server_receives_ismaster (server));

   /* selection fails */
   assert (!future_get_mongoc_server_description_ptr (future));
   ASSERT_CMPINT (error.domain, ==, MONGOC_ERROR_SERVER_SELECTION);
   ASSERT_CMPINT (error.code, ==, MONGOC_ERROR_SERVER_SELECTION_FAILURE);
   ASSERT_STARTSWITH (error.message, "No suitable servers found");

   if (try_once) {
      ASSERT_CONTAINS (error.message, "serverSelectionTryOnce");
   } else {
      ASSERT_CONTAINS (error.message, "serverselectiontimeoutms");
   }

   assert (client->topology->stale);
   future_destroy (future);

   _mongoc_usleep (510 * 1000);  /* one heartbeat, plus a few milliseconds */

   /* second selection, now we try ismaster again */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);
   request = mock_server_receives_ismaster (server);
   assert (request);

   /* the secondary is now primary, selection succeeds */
   mock_server_replies_simple (request, primary_response);
   sd = future_get_mongoc_server_description_ptr (future);
   assert (sd);
   assert (!client->topology->stale);
   request_destroy (request);
   future_destroy (future);

   mongoc_server_description_destroy (sd);
   mongoc_read_prefs_destroy (primary_pref);
   mongoc_client_destroy (client);
   mongoc_uri_destroy (uri);
   bson_free (secondary_response);
   bson_free (primary_response);
   mock_server_destroy (server);
}

static void
test_server_selection_try_once (void *ctx)
{
   _test_server_selection (true);
}

static void
test_server_selection_try_once_false (void *ctx)
{
   _test_server_selection (false);
}

static void
host_list_init (mongoc_host_list_t *host_list,
                int family,
                const char *host,
                uint16_t port)
{
   memset (host_list, 0, sizeof *host_list);
   host_list->family = family;
   bson_snprintf (host_list->host, sizeof host_list->host, "%s", host);
   bson_snprintf (host_list->host_and_port, sizeof host_list->host_and_port,
                  "%s:%hu", host, port);
}

static void
_test_topology_invalidate_server (bool pooled)
{
   mongoc_server_description_t *fake_sd;
   mongoc_server_description_t *sd;
   mongoc_topology_description_t *td;
   mongoc_client_t *client;
   mongoc_client_pool_t *pool = NULL;
   bson_error_t error;
   mongoc_host_list_t fake_host_list;
   uint32_t fake_id = 42;
   uint32_t id;
   mongoc_server_stream_t *server_stream;

   if (pooled) {
      pool = test_framework_client_pool_new ();
      client = mongoc_client_pool_pop (pool);

      /* background scanner complains about failed connection */
      capture_logs (true);
   } else {
      client = test_framework_client_new ();
   }

   td = &client->topology->description;

   /* call explicitly */
   server_stream = mongoc_cluster_stream_for_reads (&client->cluster,
                                                    NULL, &error);
   ASSERT_OR_PRINT (server_stream, error);
   id = server_stream->sd->id;
   sd = (mongoc_server_description_t *)mongoc_set_get(td->servers, id);
   assert (sd);
   assert (sd->type == MONGOC_SERVER_STANDALONE ||
           sd->type == MONGOC_SERVER_RS_PRIMARY ||
           sd->type == MONGOC_SERVER_MONGOS);

   mongoc_topology_invalidate_server (client->topology, id, NULL);
   sd = (mongoc_server_description_t *)mongoc_set_get(td->servers, id);
   assert (sd);
   assert (sd->type == MONGOC_SERVER_UNKNOWN);

   fake_sd = (mongoc_server_description_t *)bson_malloc0 (sizeof (*fake_sd));

   /* insert a 'fake' server description and ensure that it is invalidated by driver */
   host_list_init (&fake_host_list, AF_INET, "fakeaddress", 27033);
   mongoc_server_description_init (fake_sd,
                                   fake_host_list.host_and_port,
                                   fake_id);

   fake_sd->type = MONGOC_SERVER_STANDALONE;
   mongoc_set_add(td->servers, fake_id, fake_sd);
   mongoc_topology_scanner_add (client->topology->scanner,
                                &fake_host_list,
                                fake_id);
   assert (!mongoc_cluster_stream_for_server (&client->cluster, fake_id, true,
                                              &error));
   sd = (mongoc_server_description_t *)mongoc_set_get(td->servers, fake_id);
   assert (sd);
   assert (sd->type == MONGOC_SERVER_UNKNOWN);
   assert (sd->error.domain != 0);

   mongoc_server_stream_cleanup (server_stream);

   if (pooled) {
      mongoc_client_pool_push (pool, client);
      mongoc_client_pool_destroy (pool);
   } else {
      mongoc_client_destroy (client);
   }
}

static void
test_topology_invalidate_server_single (void)
{
   _test_topology_invalidate_server (false);
}

static void
test_topology_invalidate_server_pooled (void)
{
   _test_topology_invalidate_server (true);
}

static void
test_invalid_cluster_node (void *ctx)
{
   mongoc_client_pool_t *pool;
   mongoc_cluster_node_t *cluster_node;
   mongoc_topology_scanner_node_t *scanner_node;
   bson_error_t error;
   mongoc_client_t *client;
   mongoc_cluster_t *cluster;
   mongoc_server_stream_t *server_stream;
   uint32_t id;

   /* use client pool, this test is only valid when multi-threaded */
   pool = test_framework_client_pool_new ();
   client = mongoc_client_pool_pop (pool);
   cluster = &client->cluster;

   _mongoc_usleep (100 * 1000);

   /* load stream into cluster */
   server_stream = mongoc_cluster_stream_for_reads (&client->cluster,
                                                    NULL, &error);
   ASSERT_OR_PRINT (server_stream, error);
   id = server_stream->sd->id;
   mongoc_server_stream_cleanup (server_stream);

   cluster_node = (mongoc_cluster_node_t *)mongoc_set_get (cluster->nodes, id);
   scanner_node = mongoc_topology_scanner_get_node (client->topology->scanner, id);
   assert (cluster_node);
   assert (scanner_node);
   assert (cluster_node->stream);
   ASSERT_CMPINT64 (cluster_node->timestamp, >, scanner_node->timestamp);

   /* update the scanner node's timestamp */
   _mongoc_usleep (1000 * 1000);
   scanner_node->timestamp = bson_get_monotonic_time ();
   ASSERT_CMPINT64 (cluster_node->timestamp, <, scanner_node->timestamp);
   _mongoc_usleep (1000 * 1000);

   /* cluster discards node and creates new one */
   server_stream = mongoc_cluster_stream_for_server (&client->cluster,
                                                     id, true, &error);
   ASSERT_OR_PRINT (server_stream, error);
   cluster_node = (mongoc_cluster_node_t *)mongoc_set_get (cluster->nodes, id);
   ASSERT_CMPINT64 (cluster_node->timestamp, >, scanner_node->timestamp);

   mongoc_server_stream_cleanup (server_stream);
   mongoc_client_pool_push (pool, client);
   mongoc_client_pool_destroy (pool);
}

static void
test_max_wire_version_race_condition (void *ctx)
{
   mongoc_topology_scanner_node_t *scanner_node;
   mongoc_server_description_t *sd;
   mongoc_database_t *database;
   mongoc_client_pool_t *pool;
   mongoc_client_t *client;
   bson_error_t error;
   mongoc_server_stream_t *server_stream;
   uint32_t id;

   /* connect directly and add our user, test is only valid with auth */
   client = test_framework_client_new ();
   database = mongoc_client_get_database(client, "test");
   mongoc_database_remove_user (database, "pink", &error);
   ASSERT_OR_PRINT (1 == mongoc_database_add_user (
      database, "pink", "panther", NULL, NULL, &error), error);
   mongoc_database_destroy (database);
   mongoc_client_destroy (client);

   /* use client pool, test is only valid when multi-threaded */
   pool = test_framework_client_pool_new ();
   client = mongoc_client_pool_pop (pool);

   /* load stream into cluster */
   server_stream = mongoc_cluster_stream_for_reads (&client->cluster,
                                                    NULL, &error);
   ASSERT_OR_PRINT (server_stream, error);
   id = server_stream->sd->id;
   mongoc_server_stream_cleanup (server_stream);

   /* "disconnect": invalidate timestamp and reset server description */
   scanner_node = mongoc_topology_scanner_get_node (client->topology->scanner, id);
   assert (scanner_node);
   scanner_node->timestamp = bson_get_monotonic_time ();
   sd = (mongoc_server_description_t *)mongoc_set_get (client->topology->description.servers, id);
   assert (sd);
   mongoc_server_description_reset (sd);

   /* new stream, ensure that we can still auth with cached wire version */
   server_stream = mongoc_cluster_stream_for_server (&client->cluster,
                                                     id, true, &error);
   ASSERT_OR_PRINT (server_stream, error);
   assert (server_stream);

   mongoc_server_stream_cleanup (server_stream);
   mongoc_client_pool_push (pool, client);
   mongoc_client_pool_destroy (pool);
}


static void
test_cooldown_standalone (void *ctx)
{
   mock_server_t *server;
   mongoc_uri_t *uri;
   mongoc_client_t *client;
   mongoc_read_prefs_t *primary_pref;
   future_t *future;
   bson_error_t error;
   request_t *request;
   mongoc_server_description_t *sd;

   server = mock_server_new ();
   mock_server_set_request_timeout_msec (server, 100);
   mock_server_run (server);
   uri = mongoc_uri_copy (mock_server_get_uri (server));
   /* anything less than minHeartbeatFrequencyMS=500 is irrelevant */
   mongoc_uri_set_option_as_int32 (uri, "serverSelectionTimeoutMS", 100);
   client = mongoc_client_new_from_uri (uri);
   primary_pref = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   /* first ismaster fails, selection fails */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);
   request = mock_server_receives_ismaster (server);
   assert (request);
   mock_server_hangs_up (request);
   assert (!future_get_mongoc_server_description_ptr (future));
   request_destroy (request);
   future_destroy (future);

   _mongoc_usleep (1000 * 1000);  /* 1 second */

   /* second selection doesn't try to call ismaster: we're in cooldown */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);
   assert (!mock_server_receives_ismaster (server));  /* no ismaster call */
   assert (!future_get_mongoc_server_description_ptr (future));
   future_destroy (future);

   _mongoc_usleep (5100 * 1000);  /* 5.1 seconds */

   /* cooldown ends, now we try ismaster again, this time succeeding */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);
   request = mock_server_receives_ismaster (server);  /* not in cooldown now */
   assert (request);
   mock_server_replies_simple (request, "{'ok': 1, 'ismaster': true}");
   sd = future_get_mongoc_server_description_ptr (future);
   assert (sd);
   request_destroy (request);
   future_destroy (future);

   mongoc_server_description_destroy (sd);
   mongoc_read_prefs_destroy (primary_pref);
   mongoc_client_destroy (client);
   mongoc_uri_destroy (uri);
   mock_server_destroy (server);
}


static void
test_cooldown_rs (void *ctx)
{
   mock_server_t *servers[2];  /* two secondaries, no primary */
   char *uri_str;
   int i;
   mongoc_client_t *client;
   mongoc_read_prefs_t *primary_pref;
   char *secondary_response;
   char *primary_response;
   future_t *future;
   bson_error_t error;
   request_t *request;
   mongoc_server_description_t *sd;

   for (i = 0; i < 2; i++) {
      servers[i] = mock_server_new ();
      mock_server_set_request_timeout_msec (servers[i], 600);
      mock_server_run (servers[i]);
   }

   uri_str = bson_strdup_printf (
      "mongodb://localhost:%hu/?replicaSet=rs"
         "&serverSelectionTimeoutMS=100"
         "&connectTimeoutMS=100",
      mock_server_get_port (servers[0]));

   client = mongoc_client_new (uri_str);
   primary_pref = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   secondary_response = bson_strdup_printf (
      "{'ok': 1, 'ismaster': false, 'secondary': true, 'setName': 'rs',"
      " 'hosts': ['localhost:%hu', 'localhost:%hu']}",
      mock_server_get_port (servers[0]),
      mock_server_get_port (servers[1]));

   primary_response = bson_strdup_printf (
      "{'ok': 1, 'ismaster': true, 'setName': 'rs',"
      " 'hosts': ['localhost:%hu', 'localhost:%hu']}",
      mock_server_get_port (servers[0]),
      mock_server_get_port (servers[1]));

   /* server 0 is a secondary. */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);

   request = mock_server_receives_ismaster (servers[0]);
   assert (request);
   mock_server_replies_simple (request, secondary_response);
   request_destroy (request);

   /* server 0 told us about server 1. we check it immediately but it's down. */
   request = mock_server_receives_ismaster (servers[1]);
   assert (request);
   mock_server_hangs_up (request);
   request_destroy (request);

   /* selection fails. */
   assert (!future_get_mongoc_server_description_ptr (future));
   future_destroy (future);

   _mongoc_usleep (1000 * 1000);  /* 1 second */

   /* second selection doesn't try ismaster on server 1: it's in cooldown */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);

   request = mock_server_receives_ismaster (servers[0]);
   assert (request);
   mock_server_replies_simple (request, secondary_response);
   request_destroy (request);

   assert (!mock_server_receives_ismaster (servers[1]));  /* no ismaster call */

   /* still no primary */
   assert (!future_get_mongoc_server_description_ptr (future));
   future_destroy (future);

   _mongoc_usleep (5100 * 1000);  /* 5.1 seconds */

   /* cooldown ends, now we try ismaster on server 1, this time succeeding */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);

   request = mock_server_receives_ismaster (servers[1]);
   assert (request);
   mock_server_replies_simple (request, primary_response);
   request_destroy (request);

   /* server 0 doesn't need to respond */
   sd = future_get_mongoc_server_description_ptr (future);
   assert (sd);
   future_destroy (future);

   mongoc_server_description_destroy (sd);
   mongoc_read_prefs_destroy (primary_pref);
   mongoc_client_destroy (client);
   bson_free (secondary_response);
   bson_free (primary_response);
   bson_free (uri_str);
   mock_server_destroy (servers[0]);
}


static void
_test_connect_timeout (bool pooled, bool try_once)
{
   const int32_t connect_timeout_ms = 200;
   const int32_t server_selection_timeout_ms = 10 * 1000;  /* 10 seconds */
   const int64_t min_heartbeat_ms = MONGOC_TOPOLOGY_MIN_HEARTBEAT_FREQUENCY_MS;

   mock_server_t *servers[2];
   int i;
   char *secondary_response;
   char *uri_str;
   mongoc_uri_t *uri;
   mongoc_client_pool_t *pool = NULL;
   mongoc_client_t *client;
   mongoc_read_prefs_t *primary_pref;
   future_t *future;
   int64_t start;
   int64_t server0_last_ismaster;
   int64_t duration_usec;
   int64_t expected_duration_usec;
   bool server0_in_cooldown;
   bson_error_t error;
   request_t *request;

   assert (!(pooled && try_once));  /* not supported */

   for (i = 0; i < 2; i++) {
      servers[i] = mock_server_new ();
      mock_server_run (servers[i]);
   };

   secondary_response = bson_strdup_printf ("{'ok': 1,"
                                            " 'ismaster': false,"
                                            " 'secondary': true,"
                                            " 'setName': 'rs'}");

   uri_str = bson_strdup_printf (
      "mongodb://localhost:%hu,localhost:%hu/"
         "?replicaSet=rs&connectTimeoutMS=%d&serverSelectionTimeoutMS=%d",
      mock_server_get_port (servers[0]),
      mock_server_get_port (servers[1]),
      connect_timeout_ms,
      server_selection_timeout_ms);

   uri = mongoc_uri_new (uri_str);
   assert (uri);

   if (!pooled && !try_once) {
      /* override default */
      mongoc_uri_set_option_as_bool (uri, "serverSelectionTryOnce", false);
   }

   if (pooled) {
      pool = mongoc_client_pool_new (uri);
      client = mongoc_client_pool_pop (pool);
   } else {
      client = mongoc_client_new_from_uri (uri);
   }

   primary_pref = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   /* start waiting for a server */
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    primary_pref, &error);

   server0_last_ismaster = start = bson_get_monotonic_time ();

   if (mock_server_get_verbose (servers[0])) {
      printf ("server on port %hu down, server on port %hu secondary\n",
              mock_server_get_port (servers[0]),
              mock_server_get_port (servers[1]));
   }

   /* server 0 doesn't respond */
   request = mock_server_receives_ismaster (servers[0]);
   assert (request);
   request_destroy (request);

   /* server 1 is a secondary */
   request = mock_server_receives_ismaster (servers[1]);
   assert (request);
   mock_server_replies_simple (request, secondary_response);
   request_destroy (request);

   if (!try_once) {
      /* driver retries every minHeartbeatFrequencyMS + connectTimeoutMS */
      server0_in_cooldown = true;
      expected_duration_usec = 0;

      if (!pooled) {
         /* single-threaded client starts counting minHeartbeatFrequencyMS
          * AFTER each connection timeout */
         expected_duration_usec = 1000 * connect_timeout_ms;
      }

      while (expected_duration_usec / 1000 + min_heartbeat_ms
             < server_selection_timeout_ms) {
         request = mock_server_receives_ismaster (servers[1]);
         assert (request);
         mock_server_replies_simple (request, secondary_response);
         request_destroy (request);

         duration_usec = bson_get_monotonic_time () - start;
         expected_duration_usec += 1000 * min_heartbeat_ms;

         /* single client puts server 0 in cooldown for 5 sec */
         if (pooled || !server0_in_cooldown) {
            request = mock_server_receives_ismaster (servers[0]);
            assert (request);
            server0_last_ismaster = bson_get_monotonic_time ();
            request_destroy (request);  /* don't respond */
            expected_duration_usec += 1000 * connect_timeout_ms;
         }

         if (!test_suite_valgrind ()) {
            ASSERT_ALMOST_EQUAL (duration_usec, expected_duration_usec);
         }

         server0_in_cooldown =
            (bson_get_monotonic_time () - server0_last_ismaster) <
            5 * 1000 * 1000;
      }
   }

   /* selection fails */
   assert (!future_get_mongoc_server_description_ptr (future));
   future_destroy (future);

   duration_usec = bson_get_monotonic_time () - start;

   if (!test_suite_valgrind ()) {
      if (try_once) {
         ASSERT_ALMOST_EQUAL (duration_usec / 1000, connect_timeout_ms);
      } else {
         ASSERT_ALMOST_EQUAL (duration_usec / 1000, server_selection_timeout_ms);
      }
   }

   if (pooled) {
      mongoc_client_pool_push (pool, client);
      mongoc_client_pool_destroy (pool);
   } else {
      mongoc_client_destroy (client);
   }

   mongoc_read_prefs_destroy (primary_pref);
   mongoc_uri_destroy (uri);
   bson_free (uri_str);
   bson_free (secondary_response);

   for (i = 0; i < 2; i++) {
      mock_server_destroy (servers[i]);
   };
}


static void
test_connect_timeout_pooled (void *ctx)
{
   _test_connect_timeout (true, false);
}

static void
test_connect_timeout_single(void *ctx)
{
   _test_connect_timeout (false, true);
}


static void
test_connect_timeout_try_once_false(void *ctx)
{
   _test_connect_timeout (false, false);
}


static void
_test_select_succeed (bool try_once)
{
   const int32_t connect_timeout_ms = 200;

   mock_server_t *primary;
   mock_server_t *secondary;
   mongoc_server_description_t *sd;
   char *uri_str;
   mongoc_uri_t *uri;
   mongoc_client_t *client;
   future_t *future;
   int64_t start;
   bson_error_t error;
   request_t *request;
   int64_t duration_usec;

   primary = mock_server_new ();
   mock_server_run (primary);

   secondary = mock_server_new ();
   mock_server_run (secondary);

   mock_server_auto_ismaster (primary, "{'ok': 1,"
      " 'ismaster': true,"
      " 'setName': 'rs',"
      " 'hosts': ['localhost:%hu', 'localhost:%hu']}",
      mock_server_get_port (primary),
      mock_server_get_port (secondary));

   uri_str = bson_strdup_printf (
      "mongodb://localhost:%hu,localhost:%hu/"
         "?replicaSet=rs&connectTimeoutMS=%d"
         "&serverSelectionTryOnce=false",
      mock_server_get_port (primary),
      mock_server_get_port (secondary),
      connect_timeout_ms);

   uri = mongoc_uri_new (uri_str);
   assert (uri);
   if (!try_once) {
      /* override default */
      mongoc_uri_set_option_as_bool (uri, "serverSelectionTryOnce", false);
   }

   client = mongoc_client_new_from_uri (uri);

   /* start waiting for a primary (NULL read pref) */
   start = bson_get_monotonic_time ();
   future = future_topology_select (client->topology, MONGOC_SS_READ,
                                    NULL, &error);

   /* secondary doesn't respond */
   request = mock_server_receives_ismaster (secondary);
   assert (request);
   request_destroy (request);

   /* selection succeeds */
   sd = future_get_mongoc_server_description_ptr (future);
   ASSERT_OR_PRINT (sd, error);
   future_destroy (future);

   duration_usec = bson_get_monotonic_time () - start;

   if (!test_suite_valgrind ()) {
      if (mock_server_get_verbose (primary)) {
         printf ("expected duration %" PRId32 "ms, actual %" PRId64 "ms\n",
                 connect_timeout_ms, duration_usec / 1000);
      }
      ASSERT_ALMOST_EQUAL (duration_usec / 1000, connect_timeout_ms);
   }

   mongoc_client_destroy (client);
   mongoc_uri_destroy (uri);
   bson_free (uri_str);
   mongoc_server_description_destroy (sd);
   mock_server_destroy (primary);
   mock_server_destroy (secondary);
}


/* CDRIVER-1219: a secondary is unavailable, scan should take connectTimeoutMS,
 * then we select primary */
static void
test_select_after_timeout (void)
{
   _test_select_succeed (false);
}


/* CDRIVER-1219: a secondary is unavailable, scan should try it once,
 * then we select primary */
static void
test_select_after_try_once (void)
{
   _test_select_succeed (true);
}


static void
test_multiple_selection_errors (void *context)
{
   const char *uri = "mongodb://doesntexist,example.com:2/?replicaSet=rs"
      "&connectTimeoutMS=100";
   mongoc_client_t *client;
   bson_t reply;
   bson_error_t error;

   client = mongoc_client_new (uri);
   mongoc_client_command_simple (client, "test", tmp_bson ("{'ping': 1}"),
                                 NULL, &reply, &error);

   ASSERT_CMPINT (MONGOC_ERROR_SERVER_SELECTION, ==, error.domain);
   ASSERT_CMPINT (MONGOC_ERROR_SERVER_SELECTION_FAILURE, ==, error.code);

   /* Like:
    * "No suitable servers found (`serverselectiontryonce` set):
    *  [Failed to resolve 'doesntexist'] [connection error]"
    */
   ASSERT_CONTAINS (error.message,
                    "No suitable servers found");
   ASSERT_CONTAINS (error.message,
                    "[connection error calling ismaster on 'example.com:2']");
   ASSERT_CONTAINS (error.message,
                    "[Failed to resolve 'doesntexist']");

   mongoc_client_destroy (client);
}


static void
test_invalid_server_id (void)
{
   mongoc_client_t *client;
   bson_error_t error;

   client = test_framework_client_new ();

   BSON_ASSERT (!mongoc_topology_server_by_id (client->topology, 99999, &error));
   ASSERT_STARTSWITH (error.message, "Could not find description for node");

   mongoc_client_destroy (client);
}


void
test_topology_install (TestSuite *suite)
{
   bool windows;

   TestSuite_AddLive (suite, "/Topology/client_creation", test_topology_client_creation);
   TestSuite_AddLive (suite, "/Topology/client_pool_creation", test_topology_client_pool_creation);
   TestSuite_AddFull (suite, "/Topology/server_selection_try_once_option",
                      test_server_selection_try_once_option, NULL, NULL, test_framework_skip_if_slow);
   TestSuite_AddFull (suite, "/Topology/server_selection_try_once",
                      test_server_selection_try_once, NULL, NULL, test_framework_skip_if_slow);
   TestSuite_AddFull (suite, "/Topology/server_selection_try_once_false",
                      test_server_selection_try_once_false, NULL, NULL, test_framework_skip_if_slow);
   TestSuite_AddLive (suite, "/Topology/invalidate_server/single", test_topology_invalidate_server_single);
   TestSuite_AddLive (suite, "/Topology/invalidate_server/pooled", test_topology_invalidate_server_pooled);
   TestSuite_AddFull (suite, "/Topology/invalid_cluster_node",
                      test_invalid_cluster_node, NULL, NULL, test_framework_skip_if_slow);
   TestSuite_AddFull (suite, "/Topology/max_wire_version_race_condition",
                      test_max_wire_version_race_condition,
                      NULL, NULL, test_framework_skip_if_no_auth);

   /* CDRIVER-1305: disable tests that hang on 32-bit Unix */
#ifdef _MSC_VER
   windows = true;
#else
   windows = false;
#endif

   if (sizeof (int *) == 4 && !windows) {
      TestSuite_AddFull (suite, "/Topology/cooldown/standalone",
                         test_cooldown_standalone, NULL, NULL,
                         test_framework_skip_if_slow);
      TestSuite_AddFull (suite, "/Topology/cooldown/rs",
                         test_cooldown_rs, NULL, NULL,
                         test_framework_skip_if_slow);
      TestSuite_AddFull (suite, "/Topology/connect_timeout/pooled",
                         test_connect_timeout_pooled, NULL, NULL,
                         test_framework_skip_if_slow);
      TestSuite_AddFull (suite, "/Topology/connect_timeout/single/try_once",
                         test_connect_timeout_single, NULL, NULL,
                         test_framework_skip_if_slow);
      TestSuite_AddFull (suite,
                         "/Topology/connect_timeout/single/try_once_false",
                         test_connect_timeout_try_once_false, NULL, NULL,
                         test_framework_skip_if_slow);
      TestSuite_AddFull (suite, "/Topology/multiple_selection_errors",
                         test_multiple_selection_errors,
                         NULL, NULL, test_framework_skip_if_offline);
      TestSuite_Add (suite, "/Topology/connect_timeout/succeed",
                     test_select_after_timeout);
      TestSuite_Add (suite, "/Topology/try_once/succeed",
                     test_select_after_try_once);
   }

   TestSuite_AddLive (suite, "/Topology/invalid_server_id", test_invalid_server_id);
}
