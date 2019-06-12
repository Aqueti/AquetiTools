#pragma once
#include <cstdint>

// let's start with a clean slate
#undef AQT_USE_WINSOCK_SOCKETS

// Does cygwin use winsock sockets or unix sockets?  Define this before
// compiling the library if you want it to use WINSOCK sockets.
//#define CYGWIN_USES_WINSOCK_SOCKETS

#if defined(_WIN32) &&                                                         \
    (!defined(__CYGWIN__) || defined(CYGWIN_USES_WINSOCK_SOCKETS))
#define AQT_USE_WINSOCK_SOCKETS
#endif

#ifndef AQT_USE_WINSOCK_SOCKETS
#define SOCKET int
// On Win32, this constant is defined as ~0 (sockets are unsigned ints)
static const int INVALID_SOCKET = -1;
#endif

#if !(defined(_WIN32) && defined(AQT_USE_WINSOCK_SOCKETS))
#include <sys/select.h> // for select
#include <netinet/in.h> // for htonl, htons
#endif

//--------------------------------------------------------------
// Timeval defines.

#if (!defined(AQT_USE_WINSOCK_SOCKETS))
	#include <sys/time.h> // for timeval, timezone, gettimeofday
#else // winsock sockets
	// These are a pair of horrible hacks that instruct Windows include
	// files to (1) not define min() and max() in a way that messes up
	// standard-library calls to them, and (2) avoids pulling in a large
	// number of Windows header files.  They are not used directly within
	// the Sockets library, but rather within the Windows include files to
	// change the way they behave.

	#ifndef NOMINMAX
	#define NOMINMAX
	#endif
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <winsock2.h> // struct timeval is defined here
#endif

namespace atl {	namespace Sockets {

#if !(defined(_WIN32) && defined(AQT_USE_WINSOCK_SOCKETS))
#include <sys/select.h> // for fd_set
#endif

/**
 *      This routine will write a block to a file descriptor.  It acts just
 * like the write() system call does on files, but it will keep sending to
 * a socket until an error or all of the data has gone.
 *      This will also take care of problems caused by interrupted system
 * calls, retrying the write when they occur.  It will also work when
 * sending large blocks of data through socket connections, since it will
 * send all of the data before returning.
 *	This routine will either write the requested number of bytes and
 * return that or return -1 (in the case of an error) or 0 (in the case
 * of EOF being reached before all the data is sent).
 */

int noint_block_write(SOCKET outsock, const char* buffer, size_t length);

/**
 *      This routine will read in a block from the file descriptor.
 * It acts just like the read() routine does on normal files, so that
 * it hides the fact that the descriptor may point to a socket.
 *      This will also take care of problems caused by interrupted system
 * calls, retrying the read when they occur.
 *	This routine will either read the requested number of bytes and
 * return that or return -1 (in the case of an error) or 0 (in the case
 * of EOF being reached before all the data arrives).
 */

int noint_block_read(SOCKET insock, char* buffer, size_t length);

/**
 *	This routine will perform like a normal select() call, but it will
 * restart if it quit because of an interrupt.  This makes it more robust
 * in its function, and allows this code to perform properly on pxpl5, which
 * sends USER1 interrupts while rendering an image.
 */
int noint_select(int width, fd_set* readfds, fd_set* writefds,
	fd_set* exceptfds, struct timeval* timeout);

/**
 *   This routine will read in a block from the file descriptor.
 * It acts just like the read() routine on normal files, except that
 * it will time out if the read takes too long.
 *   This will also take care of problems caused by interrupted system
 * calls, retrying the read when they occur.
 *   This routine will either read the requested number of bytes and
 * return that or return -1 (in the case of an error or in the case
 * of EOF being reached before all the data arrives), or return the number
 * of characters read before timeout (in the case of a timeout).
 */

int noint_block_read_timeout(SOCKET infile, char buffer[], size_t length,
	struct timeval* timeout);

/**
 * This routine will check the listen socket to see if there has been a
 * connection request. If so, it will accept a connection on the accept
 * socket and set TCP_NODELAY on that socket. The attempt will timeout
 * in the amount of time specified.  If the accept and set are
 * successful, it returns 1. If there is nothing asking for a connection,
 * it returns 0. If there is an error along the way, it returns -1.
 */

int poll_for_accept(SOCKET listen_sock, SOCKET* accept_sock,
	double timeout = 0.0);

/**
	* This routine opens a socket with the requested port number.
	* The routine returns -1 on failure and the file descriptor on success.
	* The portno parameter is filled in with the actual port that is opened
	* (this is useful when the address INADDR_ANY is used, since it tells
	* what port was opened).
	* To select between multiple NICs, we can specify the IP address of the
	* socket to open;  NULL selects the default NIC.
	*/

SOCKET open_socket(int type, unsigned short* portno, const char* IPaddress);

/**
 * Create a UDP socket and bind it to its local address.
 */

SOCKET open_udp_socket(unsigned short* portno, const char* IPaddress);

/**
 * Create a TCP socket and bind it to its local address.
 */

SOCKET open_tcp_socket(unsigned short* portno = NULL, const char* NIC_IP = NULL);

/**
 * Create a UDP socket and connect it to a specified port.
 */

SOCKET connect_udp_port(const char* machineName, int remotePort,
	const char* NIC_IP = NULL);

/**
 * Retrieves the IP address or hostname of the local interface used to connect
 * to the specified remote host.
 * XXX: This does not always work.  See the Github issue with the report from
 * Isop W. Alexander showing that a machine with two ports (172.28.0.10 and
 * 192.168.191.130) sent a connection request that came from the 172 IP address
 * but that had the name of the 192 interface in the message as the host to
 * call back.  This turned out to be unroutable, so the server failed to call
 * back on the correct IP address.  Presumably, this happens when the gateway
 * is configured to be a single outgoing NIC.  This was on a Linux box.  We
 * need a more reliable way to select the outgoing NIC.  XXX Actually, the
 * problem may be that we aren't listening on the incorrect port -- the UDP
 * receipt code may use the IP address the message came from rather than the
 * machine name in the message.
 *
 * @param local_host A buffer of size 64 that will contain the name of the local
 * interface.
 * @param max_length The maximum length of the local_host buffer.
 * @param remote_host The name of the remote host.
 *
 * @return Returns -1 on getsockname() error, or the output of sprintf
 * building the local_host string.
 */
int get_local_socket_name(char* local_host, size_t max_length,
	const char* remote_host);

/**
 * This section deals with implementing a method of connection termed a
 * UDP request.  This works by having the client open a TCP socket that
 * it listens on. It then lobs datagrams to the server asking to be
 * called back at the socket. This allows it to timeout on waiting for
 * a connection request, resend datagrams in case some got lost, or give
 * up at any time. The whole algorithm is implemented in the
 * udp_request_call() function; the functions before that are helper
 * functions that have been broken out to allow a subset of the algorithm
 * to be run by a connection whose server has dropped and they want to
 * re-establish it.
 *
 * This routine will lob a datagram to the given port on the given
 * machine asking it to call back at the port on this machine that
 * is also specified. It returns 0 on success and -1 on failure.
 */

int udp_request_lob_packet(
	SOCKET udp_sock,      // Socket to use to send
	const char*,         // Name of the machine to call
	const int,            // UDP port on remote machine
	const int local_port, // TCP port on this machine
	const char* NIC_IP = NULL);

/**
 * This routine will get a TCP socket that is ready to accept connections.
 * That is, listen() has already been called on it.
 * It will get whatever socket is available from the system. It returns
 * 0 on success and -1 on failure. On success, it fills in the pointers to
 * the socket and the port number of the socket that it obtained.
 * To select between multiple network interfaces, we can specify an IPaddress;
 * the default value is NULL, which uses the default NIC.
 */

int get_a_TCP_socket(SOCKET* listen_sock, int* listen_portnum,
	const char* NIC_IP = NULL);

/**
 *   This function returns the host IP address in string form.  For example,
 * the machine "ioglab.cs.unc.edu" becomes "152.2.130.90."  This is done
 * so that the remote host can connect back even if it can't resolve the
 * DNS name of this host.  This is especially useful at conferences, where
 * DNS may not even be running.
 *   If the NIC_IP name is passed in as NULL but the SOCKET passed in is
 * valid, then look up the address associated with that socket; this is so
 * that when a machine has multiple NICs, it will send the outgoing request
 * for UDP connections to the same place that its TCP connection is on.
 */

int getmyIP(char* myIPchar, unsigned maxlen,
	const char* NIC_IP = NULL,
	SOCKET incoming_socket = INVALID_SOCKET);

/// @param [in] NICaddress Name of the network card to use, can be obtained
///             by calling getmyIP() or set to NULL to listen on all IP addresses.
bool connect_tcp_to(const char* addr, int port, const char *NICaddress, SOCKET *s);

int close_socket(SOCKET sock);

/// @brief Convert types to and from network-standard byte order.
double hton(double d);
double ntoh(double d);

int64_t hton(int64_t d);
int64_t ntoh(int64_t d);

}  }	// End of namespace definitions.
