/*
 * Copyright 2014 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bson.h>
#include <mongoc.h>

#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>

#include "mongoc-log.h"
#include "mongoc-log-private.h"

#if defined(__APPLE__)
# include <mach/mach_time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
# include <sys/types.h>
# include <inttypes.h>
# include <sys/utsname.h>
# include <sys/wait.h>
# include <unistd.h>
# include <sys/time.h>
#else
# include <windows.h>
#endif

#if defined(BSON_HAVE_CLOCK_GETTIME)
# include <time.h>
# include <sys/time.h>
#endif

#include "test-libmongoc.h"
#include "TestSuite.h"


static int test_flags;


#define TEST_VERBOSE     (1 << 0)
#define TEST_NOFORK      (1 << 1)
#define TEST_HELPONLY    (1 << 2)
#define TEST_DEBUGOUTPUT (1 << 3)
#define TEST_TRACE       (1 << 4)
#define TEST_VALGRIND    (1 << 5)


#define NANOSEC_PER_SEC 1000000000UL

#if !defined(BSON_HAVE_TIMESPEC)
struct timespec
{
   time_t tv_sec;
   long   tv_nsec;
};
#endif


#if defined(_WIN32) && !defined(BSON_HAVE_SNPRINTF)
static int
snprintf (char *str,
          size_t size,
          const char *format,
          ...)
{
   int r = -1;
   va_list ap;

   va_start (ap, format);
   if (size != 0) {
      r = _vsnprintf_s (str, size, _TRUNCATE, format, ap);
   }
   if (r == -1) {
      r = _vscprintf (format, ap);
   }
   va_end (ap);

   return r;
}
#endif


void
_Print_StdOut (const char *format,
               ...)
{
   va_list ap;

   va_start (ap, format);
   vprintf (format, ap);
   fflush (stdout);   
   va_end (ap);
}


void
_Print_StdErr (const char *format,
               ...)
{
   va_list ap;

   va_start (ap, format);
   vfprintf (stderr, format, ap);
   fflush (stderr);
   va_end (ap);
}


void
_Clock_GetMonotonic (struct timespec *ts) /* OUT */
{
#if defined(BSON_HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
   clock_gettime (CLOCK_MONOTONIC, ts);
#elif defined(__APPLE__)
   static mach_timebase_info_data_t info = { 0 };
   static double ratio;
   uint64_t atime;

   if (!info.denom) {
      mach_timebase_info(&info);
      ratio = info.numer / info.denom;
   }

   atime = mach_absolute_time() * ratio;
   ts->tv_sec = atime * 1e-9;
   ts->tv_nsec = atime - (ts->tv_sec * 1e9);
#elif defined(_WIN32)
   /* GetTickCount64() returns milliseconds */
   ULONGLONG ticks = GetTickCount64 ();
   ts->tv_sec = ticks / 1000;

   /* milliseconds -> microseconds -> nanoseconds*/
   ts->tv_nsec = (ticks % 1000) * 1000 * 1000;
#else
# warning "Monotonic clock is not yet supported on your platform."
#endif
}


void
_Clock_Subtract (struct timespec *ts, /* OUT */
                 struct timespec *x,  /* IN */
                 struct timespec *y)  /* IN */
{
   struct timespec r;

   ASSERT (x);
   ASSERT (y);

   r.tv_sec = (x->tv_sec - y->tv_sec);

   if ((r.tv_nsec = (x->tv_nsec - y->tv_nsec)) < 0) {
      r.tv_nsec += NANOSEC_PER_SEC;
      r.tv_sec -= 1;
   }

   *ts = r;
}


static void
TestSuite_SeedRand (TestSuite *suite, /* IN */
                    Test *test)       /* IN */
{
#ifndef BSON_OS_WIN32
   int fd = open ("/dev/urandom", O_RDONLY);
   int n_read;
   unsigned seed;
   if (fd != -1) {
      n_read = read (fd, &seed, 4);
      assert (n_read == 4);
      close (fd);
      test->seed = seed;
      return;
   } else {
      test->seed = (unsigned)time (NULL) * (unsigned)getpid ();
   }
#else
   test->seed = (unsigned)time (NULL);
#endif
}


void
TestSuite_Init (TestSuite *suite,
                const char *name,
                int argc,
                char **argv)
{
   const char *filename;
   int i;

   memset (suite, 0, sizeof *suite);

   suite->name = strdup (name);
   suite->flags = 0;
   suite->prgname = strdup (argv [0]);
   suite->silent = false;

   for (i = 0; i < argc; i++) {
      if (0 == strcmp ("-v", argv [i])) {
         suite->flags |= TEST_VERBOSE;
      } else if (0 == strcmp ("-d", argv [i])) {
         suite->flags |= TEST_DEBUGOUTPUT;
      } else if ((0 == strcmp ("-f", argv [i])) ||
                 (0 == strcmp ("--no-fork", argv [i]))) {
         suite->flags |= TEST_NOFORK;
      } else if ((0 == strcmp ("-t", argv [i])) ||
                 (0 == strcmp ("--trace", argv [i]))) {
#ifdef MONGOC_TRACE
         suite->flags |= TEST_TRACE;
#else
         _Print_StdErr ("-t requires mongoc compiled with --enable-tracing.\n");
         exit (EXIT_FAILURE);
#endif
      } else if (0 == strcmp ("-F", argv [i])) {
         if (argc - 1 == i) {
            _Print_StdErr ("-F requires a filename argument.\n");
            exit (EXIT_FAILURE);
         }
         filename = argv [++i];
         if (0 != strcmp ("-", filename)) {
#ifdef _WIN32
            if (0 != fopen_s (&suite->outfile, filename, "w")) {
               suite->outfile = NULL;
            }
#else
            suite->outfile = fopen (filename, "w");
#endif
            if (!suite->outfile) {
               _Print_StdErr ("Failed to open log file: %s\n", filename);
            }
         }
      } else if ((0 == strcmp ("-h", argv [i])) ||
                 (0 == strcmp ("--help", argv [i]))) {
         suite->flags |= TEST_HELPONLY;
      } else if ((0 == strcmp ("-s", argv [i])) ||
                 (0 == strcmp ("--silent", argv [i]))) {
         suite->silent = true;
      } else if ((0 == strcmp ("-l", argv [i]))) {
         if (argc - 1 == i) {
            _Print_StdErr ("-l requires an argument.\n");
            exit (EXIT_FAILURE);
         }
         suite->testname = strdup (argv [++i]);
      }
   }
   
   if (test_framework_getenv_bool ("MONGOC_TEST_VALGRIND")) {
      suite->flags |= TEST_VALGRIND;
   }

   if (suite->silent) {
      if (suite->outfile) {
         _Print_StdErr ("Cannot combine -F with --silent\n");
         abort ();
      }

      suite->flags &= ~(TEST_DEBUGOUTPUT | TEST_VERBOSE);
   }

   /* HACK: copy flags to global var */
   test_flags = suite->flags;
}


int
TestSuite_CheckLive (void)
{
   return test_framework_getenv_bool ("MONGOC_TEST_SKIP_LIVE") ? 0 : 1;
}

static int
TestSuite_CheckDummy (void)
{
   return 1;
}

static void
TestSuite_AddHelper (void *cb_)
{
   TestFunc cb = (TestFunc)cb_;

   cb();
}

void
TestSuite_Add (TestSuite  *suite, /* IN */
               const char *name,  /* IN */
               TestFunc    func)  /* IN */
{
   TestSuite_AddFull (suite, name, TestSuite_AddHelper, NULL, (void *)func, TestSuite_CheckDummy);
}


void
TestSuite_AddLive (TestSuite  *suite, /* IN */
                   const char *name,  /* IN */
                   TestFunc    func)  /* IN */
{
   TestSuite_AddFull (suite, name, TestSuite_AddHelper, NULL, (void *)func, TestSuite_CheckLive);
}


void
TestSuite_AddWC (TestSuite  *suite, /* IN */
                 const char *name,  /* IN */
                 TestFuncWC  func,  /* IN */
                 TestFuncDtor dtor,  /* IN */
                 void       *ctx)   /* IN */
{
   TestSuite_AddFull (suite, name, func, dtor, ctx, TestSuite_CheckDummy);
}


void
TestSuite_AddFull (TestSuite  *suite,   /* IN */
                   const char *name,    /* IN */
                   TestFuncWC  func,    /* IN */
                   TestFuncDtor dtor,    /* IN */
                   void       *ctx,
                   int (*check) (void)) /* IN */
{
   Test *test;
   Test *iter;

   test = (Test *)calloc (1, sizeof *test);
   test->name = strdup (name);
   test->func = func;
   test->check = check;
   test->next = NULL;
   test->dtor = dtor;
   test->ctx = ctx;
   TestSuite_SeedRand (suite, test);

   if (!suite->tests) {
      suite->tests = test;
      return;
   }

   for (iter = suite->tests; iter->next; iter = iter->next) { }

   iter->next = test;
}


#if defined(_WIN32)
static void
_print_getlasterror_win (const char *msg)
{
   LPTSTR err_msg;

   FormatMessage (
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      GetLastError (),
      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
      &err_msg,
      0,
      NULL);

   _Print_StdErr ("%s: %s\n", msg, err_msg);

   LocalFree (err_msg);
}


static int
TestSuite_RunFuncInChild (TestSuite *suite, /* IN */
                          Test *test)       /* IN */
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   char *cmdline;
   DWORD exit_code = -1;

   ZeroMemory (&si, sizeof (si));
   si.cb = sizeof (si);
   ZeroMemory (&pi, sizeof (pi));

   cmdline = bson_strdup_printf ("%s --silent --no-fork -l %s",
                                 suite->prgname, test->name);

   if (!CreateProcess (NULL,
                       cmdline,
                       NULL,    /* Process handle not inheritable  */
                       NULL,    /* Thread handle not inheritable   */
                       FALSE,   /* Set handle inheritance to FALSE */
                       0,       /* No creation flags               */
                       NULL,    /* Use parent's environment block  */
                       NULL,    /* Use parent's starting directory */
                       &si,
                       &pi)) {
      _Print_StdErr ("CreateProcess failed (%d).\n", GetLastError ());
      bson_free (cmdline);

      return -1;
   }

   if (WaitForSingleObject (pi.hProcess, INFINITE) != WAIT_OBJECT_0) {
      _print_getlasterror_win ("Couldn't await process");
      goto done;
   }

   if (!GetExitCodeProcess (pi.hProcess, &exit_code)) {
      _print_getlasterror_win ("Couldn't get exit code");
      goto done;
   }

done:
   CloseHandle (pi.hProcess);
   CloseHandle (pi.hThread);
   bson_free (cmdline);

   return exit_code;
}
#else  /* Unix */
static int
TestSuite_RunFuncInChild (TestSuite *suite, /* IN */
                          Test *test)       /* IN */
{
   pid_t child;
   int exit_code = -1;
   int fd;

   if (suite->outfile) {
      fflush (suite->outfile);
   }

   if (-1 == (child = fork())) {
      return -1;
   }

   if (!child) {
      if (suite->outfile) {
         fclose (suite->outfile);
         suite->outfile = NULL;
      }
      fd = open ("/dev/null", O_WRONLY);
      dup2 (fd, STDOUT_FILENO);
      close (fd);
      execl (suite->prgname, suite->prgname, "--no-fork",
             "-l", test->name, (char *) 0);
      exit (-1);
   }

   if (-1 == waitpid (child, &exit_code, 0)) {
      perror ("waitpid()");
   }

   return exit_code;
}
#endif


static int
TestSuite_RunTest (TestSuite *suite,       /* IN */
                   Test *test,             /* IN */
                   int *count)             /* INOUT */
{
   struct timespec ts1;
   struct timespec ts2;
   struct timespec ts3;
   char name[MAX_TEST_NAME_LENGTH];
   char buf[MAX_TEST_NAME_LENGTH + 500];
   int status = 0;

   snprintf (name, sizeof name, "%s%s", suite->name, test->name);
   name [sizeof name - 1] = '\0';

   if (!test->check || test->check ()) {
      _Clock_GetMonotonic (&ts1);

      /*
       * TODO: If not verbose, close()/dup(/dev/null) for stdout.
       */

      if (suite->flags & TEST_DEBUGOUTPUT) {
         _Print_StdOut ("Begin %s\n", name);
      }

      if ((suite->flags & TEST_NOFORK)) {
#ifdef MONGOC_TRACE
         if (suite->flags & TEST_TRACE) {
            mongoc_log_set_handler (mongoc_log_default_handler, NULL);
            mongoc_log_trace_enable ();
         } else {
            mongoc_log_trace_disable ();
         }
#endif

         srand (test->seed);
         test->func (test->ctx);
         status = 0;
      } else {
         status = TestSuite_RunFuncInChild (suite, test);
      }

      capture_logs (false);

      if (suite->silent) {
         return status;
      }

      _Clock_GetMonotonic (&ts2);
      _Clock_Subtract (&ts3, &ts2, &ts1);

      snprintf (buf, sizeof buf,
                "    { \"status\": \"%s\", "
                      "\"test_file\": \"%s\", "
                      "\"seed\": \"%u\", "
                      "\"start\": %u.%09u, "
                      "\"end\": %u.%09u, "
                      "\"elapsed\": %u.%09u }%s\n",
               (status == 0) ? "PASS" : "FAIL",
               name,
               test->seed,
               (unsigned)ts1.tv_sec,
               (unsigned)ts1.tv_nsec,
               (unsigned)ts2.tv_sec,
               (unsigned)ts2.tv_nsec,
               (unsigned)ts3.tv_sec,
               (unsigned)ts3.tv_nsec,
               ((*count) == 1) ? "" : ",");
      buf [sizeof buf - 1] = 0;
      _Print_StdOut ("%s", buf);
      if (suite->outfile) {
         fprintf (suite->outfile, "%s", buf);
         fflush (suite->outfile);
      }
   } else if (!suite->silent) {
      status = 0;
      snprintf (buf, sizeof buf,
                "    { \"status\": \"SKIP\", \"test_file\": \"%s\" }%s\n",
                test->name,
                ((*count) == 1) ? "" : ",");
      buf [sizeof buf - 1] = '\0';
      _Print_StdOut ("%s", buf);
      if (suite->outfile) {
         fprintf (suite->outfile, "%s", buf);
         fflush (suite->outfile);
      }
   }

   return status ? 1 : 0;
}


static void
TestSuite_PrintHelp (TestSuite *suite, /* IN */
                     FILE *stream)     /* IN */
{
   Test *iter;

   fprintf (stream,
"usage: %s [OPTIONS]\n"
"\n"
"Options:\n"
"    -h, --help    Show this help menu.\n"
"    -f, --no-fork Do not spawn a process per test (abort on first error).\n"
"    -l NAME       Run test by name, e.g. \"/Client/command\" or \"/Client/*\".\n"
"    -v            Be verbose with logs.\n"
"    -s, --silent  Suppress all output.\n"
"    -F FILENAME   Write test results (JSON) to FILENAME.\n"
"    -d            Print debug output (useful if a test hangs).\n"
"    -t, --trace   Enable mongoc tracing (useful to debug tests).\n"
"\n"
"Tests:\n",
            suite->prgname);

   for (iter = suite->tests; iter; iter = iter->next) {
      fprintf (stream, "    %s%s\n", suite->name, iter->name);
   }

   fprintf (stream, "\n");
}


static void
TestSuite_PrintJsonSystemHeader (FILE *stream)
{
#ifdef _WIN32
#  define INFO_BUFFER_SIZE 32767

   SYSTEM_INFO si;
   DWORD version = 0;
   DWORD major_version = 0;
   DWORD minor_version = 0;
   DWORD build = 0;

   GetSystemInfo(&si);
   version = GetVersion();

   major_version = (DWORD)(LOBYTE(LOWORD(version)));
   minor_version = (DWORD)(HIBYTE(LOWORD(version)));

   if (version < 0x80000000) {
      build = (DWORD)(HIWORD(version));
   }

   fprintf (stream,
            "  \"host\": {\n"
            "    \"sysname\": \"Windows\",\n"
            "    \"release\": \"%ld.%ld (%ld)\",\n"
            "    \"machine\": \"%ld\",\n"
            "    \"memory\": {\n"
            "      \"pagesize\": %ld,\n"
            "      \"npages\": %d\n"
            "    }\n"
            "  },\n",
            major_version, minor_version, build,
            si.dwProcessorType,
            si.dwPageSize,
            0
   );
#else
   struct utsname u;
   uint64_t pagesize;
   uint64_t npages = 0;

   if (uname (&u) == -1) {
      perror ("uname()");
      return;
   }

   pagesize = sysconf (_SC_PAGE_SIZE);

#  if defined(_SC_PHYS_PAGES)
   npages = sysconf (_SC_PHYS_PAGES);
#  endif
   fprintf (stream,
            "  \"host\": {\n"
            "    \"sysname\": \"%s\",\n"
            "    \"release\": \"%s\",\n"
            "    \"machine\": \"%s\",\n"
            "    \"memory\": {\n"
            "      \"pagesize\": %"PRIu64",\n"
            "      \"npages\": %"PRIu64"\n"
            "    }\n"
            "  },\n",
            u.sysname,
            u.release,
            u.machine,
            pagesize,
            npages
   );
#endif
}

char *egetenv (const char *name)
{
   return getenv(name) ? getenv(name) : "";
}
static void
TestSuite_PrintJsonHeader (TestSuite *suite, /* IN */
                           FILE *stream)     /* IN */
{
   char *hostname = test_framework_get_host ();
   char *udspath  = test_framework_get_unix_domain_socket_path_escaped ();
   int   port     = test_framework_get_port ();
   bool  ssl      = test_framework_get_ssl ();

   ASSERT (suite);

   fprintf (stream, "{\n");
   TestSuite_PrintJsonSystemHeader (stream);
   fprintf (stream,
            "  \"auth\": { \"user\": \"%s\", \"pass\": \"%s\" }, \n"
            "  \"addr\": { \"host\": \"%s\", \"port\": %d, \"uri\": \"%s\" },\n"
            "  \"gssapi\": { \"host\": \"%s\", \"user\": \"%s\" }, \n"
            "  \"uds\": \"%s\", \n"
            "  \"SSL\": {\n"
            "    \"enabled\": %s,\n"
            "    \"weak_cert_validation\": %s,\n"
            "    \"pem_file\": \"%s\",\n"
            "    \"pem_pwd\": \"%s\",\n"
            "    \"ca_file\": \"%s\",\n"
            "    \"ca_dir\": \"%s\",\n"
            "    \"crl_file\": \"%s\"\n"
            "  },\n"
            "  \"framework\": {\n"
            "    \"verbose\": { \"monitoring\": %s, \"server\": %s },\n"
            "    \"futureTimeoutMS\": %"PRIu64",\n"
            "    \"majorityReadConcern\": %s,\n"
            "    \"IPv6\": %s\n"
            "  },\n"
            "  \"options\": {\n"
            "    \"fork\": %s,\n"
            "    \"tracing\": %s\n"
            "  },\n"
            "  \"results\": [\n",
            egetenv ("MONGOC_TEST_USER"), egetenv ("MONGOC_TEST_PASSWORD"),
            hostname, port, egetenv ("MONGOC_TEST_URI"),
            egetenv ("MONGOC_TEST_GSSAPI_HOST"), egetenv ("MONGOC_TEST_GSSAPI_USER"),
            udspath,
            ssl ? "true" : "false",
            test_framework_getenv_bool ("MONGOC_TEST_SSL_WEAK_CERT_VALIDATION") ? "true": "false",
            egetenv ("MONGOC_TEST_SSL_PEM_FILE"),
            egetenv ("MONGOC_TEST_SSL_PEM_PWD"),
            egetenv ("MONGOC_TEST_SSL_CA_FILE"),
            egetenv ("MONGOC_TEST_SSL_CA_DIR"),
            egetenv ("MONGOC_TEST_SSL_CRL_FILE"),
            getenv ("MONGOC_TEST_MONITORING_VERBOSE") ? "true" : "false",
            getenv ("MONGOC_TEST_SERVER_VERBOSE") ? "true" : "false",
            get_future_timeout_ms (),
            test_framework_getenv_bool ("MONGOC_ENABLE_MAJORITY_READ_CONCERN") ? "true" : "false",
            test_framework_getenv_bool ("MONGOC_CHECK_IPV6") ? "true" : "false",
            (suite->flags & TEST_NOFORK) ? "false" : "true",
            (suite->flags & TEST_TRACE) ? "true" : "false"
   );

   bson_free (hostname);
   bson_free (udspath);

   fflush (stream);
}


static void
TestSuite_PrintJsonFooter (FILE *stream) /* IN */
{
   fprintf (stream, "  ]\n}\n");
   fflush (stream);
}


static int
TestSuite_RunSerial (TestSuite *suite) /* IN */
{
   Test *test;
   int count = 0;
   int status = 0;

   for (test = suite->tests; test; test = test->next) {
      count++;
   }

   for (test = suite->tests; test; test = test->next) {
      status += TestSuite_RunTest (suite, test, &count);
      count--;
   }

   if (suite->silent) {
      return status;
   }

   TestSuite_PrintJsonFooter (stdout);
   if (suite->outfile) {
      TestSuite_PrintJsonFooter (suite->outfile);
   }

   return status;
}


static int
TestSuite_RunNamed (TestSuite *suite,     /* IN */
                    const char *testname) /* IN */
{
   char name[128];
   Test *test;
   int count = 1;
   bool star = strlen (testname) && testname[strlen (testname) - 1] == '*';
   bool match;
   int status = 0;

   ASSERT (suite);
   ASSERT (testname);

   for (test = suite->tests; test; test = test->next) {
      snprintf (name, sizeof name, "%s%s",
                suite->name, test->name);
      name [sizeof name - 1] = '\0';
      if (star) {
         /* e.g. testname is "/Client*" and name is "/Client/authenticate" */
         match = (0 == strncmp (name, testname, strlen (testname) - 1));
      } else {
         match = (0 == strcmp (name, testname));
      }

      if (match) {
         status += TestSuite_RunTest (suite, test, &count);
      }
   }

   if (suite->silent) {
      return status;
   }

   TestSuite_PrintJsonFooter (stdout);
   if (suite->outfile) {
      TestSuite_PrintJsonFooter (suite->outfile);
   }

   return status;
}


int
TestSuite_Run (TestSuite *suite) /* IN */
{
   int failures = 0;

   if ((suite->flags & TEST_HELPONLY)) {
      TestSuite_PrintHelp (suite, stderr);
      return 0;
   }

   if (!suite->silent) {
      TestSuite_PrintJsonHeader (suite, stdout);
      if (suite->outfile) {
         TestSuite_PrintJsonHeader (suite, suite->outfile);
      }
   }

   if (suite->tests) {
      if (suite->testname) {
         failures += TestSuite_RunNamed (suite, suite->testname);
      } else {
         failures += TestSuite_RunSerial (suite);
      }
   } else if (!suite->silent) {
      TestSuite_PrintJsonFooter (stdout);
      if (suite->outfile) {
         TestSuite_PrintJsonFooter (suite->outfile);
      }
   }

   return failures;
}


void
TestSuite_Destroy (TestSuite *suite)
{
   Test *test;
   Test *tmp;

   for (test = suite->tests; test; test = tmp) {
      tmp = test->next;

      if (test->dtor) {
         test->dtor(test->ctx);
      }
      free (test->name);
      free (test);
   }

   if (suite->outfile) {
      fclose (suite->outfile);
   }

   free (suite->name);
   free (suite->prgname);
   free (suite->testname);
}


int
test_suite_debug_output (void)
{
   return 0 != (test_flags & TEST_DEBUGOUTPUT);
}


int
test_suite_valgrind (void)
{
   return 0 != (test_flags & TEST_VALGRIND);
}
