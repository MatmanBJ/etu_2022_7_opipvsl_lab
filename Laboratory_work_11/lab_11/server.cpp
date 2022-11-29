#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

typedef struct sockaddr_in SocketAddressIn; // struct with server address w/ IP
typedef struct sockaddr SocketAddress; // struct with server address
typedef struct timeval TimeValue; // struct w/ max waiting time to sending a message

int main(int argc, char* argv[])
{
	// ---------- INITIALIZING & PREPARING ----------
	
	int socket_fd; // socket-accepter, who gets the socket, who wants to establish connection w/ server
	int socket_listener; // socket-listener, who listens all requests to server
	int socket_detect; // socket change check
	char socket_client_name[255];
	fd_set fds; // set of the handles
	pid_t process_id; // new process id
	SocketAddressIn socket_address_in;
	TimeValue time_value;
	
	// ---------- SOCKET CREATION ----------
	// creating socket-listener, who gets all the requests to server
	// setting the connetion port, protocol IPv4 and address
	// then binding the settings (port, protocol IPv4 and address) for it,
	// so the others can identify it now (before that we haven't exact address for it,
	// and the others couldn't know, how to connect)
	
	/*
	https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=socket&category=2
	https://man7.org/linux/man-pages/man2/socket.2.html
	#include <sys/socket.h>
	int socket(int domain, int type, int protocol)
	domain -- argument specifies a communication domain
	type -- specifies the communication semantics
	  since linux 2.6.27, the type argument serves a second purpose:
	  in addition to specifying a socket type,
	  it may include the bitwise OR of any of the following values,
	  to modify the behavior of socket()
	protocol -- specifies a particular protocol to be used with the socket
	  normally only a single protocol exists to support a particular socket type within a given protocol family,
	  in which case protocol can be specified as 0
	purpose: create an endpoint for communication
	return: on success, a file descriptor for the new socket is returned, on error, -1 is returned, and errno is set to indicate the error
	*/
	socket_listener = socket(AF_INET, SOCK_STREAM, 0); // creating recieving socket
	if(socket_listener < 0)
	{
		perror("Socket creation error!");
		exit(1);
	}
	
	socket_address_in.sin_family = AF_INET; // network interaction, IPv4 protocol
	socket_address_in.sin_port = htons(3434); // port number in network byte order
	socket_address_in.sin_addr.s_addr = inet_addr("127.0.0.1"); // ip-address; "127.0.0.1" is "localhost"
	
	/*
	https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=bind&category=2
	https://man7.org/linux/man-pages/man2/bind.2.html
	#include <sys/socket.h>
	int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	sockfd -- the socket referred to by the file descriptor sockfd
	addr -- socket exists in a name space (address family) but has no address assigned to it, so it assigns the address specified by addr
	addrlen -- specifies the size, in bytes, of the address structure pointed to by addr
	purpose: bind a name to a socket
	return: on success, zero is returned on error, -1 is returned, and errno is set to indicate the error
	*/
	if(bind(socket_listener, (SocketAddress*)&socket_address_in, sizeof(socket_address_in)) < 0) // binding to a network address
	{
		perror("Binding to the network address error!");
		exit(2);
	}
	
	// ---------- RECIEVING MESSAGE FROM CLIENTS & SENDING MESSAGE TO CLIENTS ----------
	// setting timeout time for waiting any requests from clients,
	// every time that we establishing a new connection, we reset it to 0,
	// so count 15 secongs again,
	// then we establishing max connections -- 10 -- for server
	// and finally we waiting any requests from socket-listener, accept it w/ socket-accepter
	// and start a new process, who treat a message and send it backwards
	
	// setting waiting time for recieving a message from client
	time_value.tv_sec = 15;
	time_value.tv_usec = 0;
	
	/*
	https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=listen&category=2
	https://man7.org/linux/man-pages/man2/listen.2.html
	#include <sys/socket.h>
	int listen(int sockfd, int backlog)
	sockfd -- the socket referred to
	backlog -- argument defines the maximum length to which the queue of pending connections for sockfd may grow
	purpose: listen for connections on a socket
	return: on success, zero is returned, on error, -1 is returned, and errno is set to indicate the error
	struct sockaddr
	{
		sa_family_t sa_family;
		char        sa_data[14];
	}
	*/
	listen(socket_listener, 10); // creating a queue for the attached sockets

	while(true)
	{
		// waiting a socket
		FD_ZERO(&fds);
		FD_SET(socket_listener, &fds);
		time_value.tv_sec = 15;
		time_value.tv_usec = 0;
		
		/*
		https://www.opennet.ru/man.shtml?topic=select&category=2&russian=0
		https://man7.org/linux/man-pages/man2/select.2.html
		#include <sys/select.h>
		int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout)
		void FD_SET(int fd, fd_set *set)
		void FD_ZERO(fd_set *set)
		nfds -- the highest-numbered file descriptor in any of the three sets, plus 1
		readfds -- the file descriptors in this set are watched to see if they are ready for reading
		writefds -- the file descriptors in this set are watched to see if they are ready for writing
		exceptfds -- the file descriptors in this set are watched for "exceptional conditions"
		timeout -- the timeout argument is a timeval structure that specifies
		  the interval that select() should block waiting for a file descriptor to become ready
		purpose: synchronous I/O multiplexing
		return: on success, return the number of file descriptors contained in the three returned descriptor sets,
		  the return value may be zero if the timeout expired before any file descriptors became ready;
		  on error, -1 is returned, and errno is set to indicate the error
		*/
		socket_detect = select(FD_SETSIZE, &fds, NULL, NULL, &time_value); // socket state check, number of asked handles, ready to read checking, time
		if(socket_detect == 0)
		{
			cout << "---------- MESSAGE REQUEST FROM CLIENT TIMEOUT ----------\n";
			/*
			https://www.opennet.ru/man.shtml?topic=close&category=3&russian=5
			https://man7.org/linux/man-pages/man2/close.2.html
			#include <unistd.h>
			int close(int fd)
			fd -- file descriptor
			purpose: close a file descriptor
			return: returns zero on success, on error, -1 is returned, and errno is set to indicate the error
			*/
			close(socket_fd);
			exit(1);
		}
		else
		{
			process_id = -1;
			if (process_id != 0)
			{
				/*
				https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=accept&category=2
				https://man7.org/linux/man-pages/man2/accept.2.html
				#include <sys/socket.h>
				int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen)
				sockfd -- listening socket
				addr -- pointer to a sockaddr structure, filled w/ peer socket, as known to the communications layer
				addrlen -- length of addr
				purpose: accept a connection on a socket
				return: on success, these system calls return a file descriptor for the accepted socket (a nonnegative integer),
				  on error, -1 is returned, errno is set to indicate the error, and addrlen is left unchanged
				*/
				socket_fd = accept(socket_listener, NULL, NULL);
				if(socket_fd < 0)
				{
					perror("Connection establish error!");
					exit(1);
				}
				sprintf(socket_client_name, "%d", socket_fd);
				process_id = fork();
				if(process_id == 0)
				{
					execl("subserver", " ", socket_client_name, NULL);
				}
			}
		}
	}
	
	// ---------- CLEANING & TERMINATING ----------
	
	if (process_id != 0) // if it's NOT a new process, close ITS socket
	{
		/*
		https://www.opennet.ru/man.shtml?topic=close&category=3&russian=5
		https://man7.org/linux/man-pages/man2/close.2.html
		#include <unistd.h>
		int close(int fd)
		fd -- file descriptor
		purpose: close a file descriptor
		return: returns zero on success, on error, -1 is returned, and errno is set to indicate the error
		*/
		close(socket_listener);
	}
	
	return 0;
}
