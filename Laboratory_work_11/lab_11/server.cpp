/*
 * ./server
 *
 * could be only 1 server
 *
 */

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
	https://man7.org/linux/man-pages/man2/socket.2.html
	int socket(int domain, int type, int protocol)
	creates socket (w/ domain in domain, protocol in protocol), return file descriptor for the new socket if successful, -1 if error
	AF_INET -- IPv4
	SOCK_STREAM -- two endpoints bytes stream
	*/
	socket_listener = socket(AF_INET, SOCK_STREAM, 0); // creating recieving socket
	if(socket_listener < 0)
	{
		perror("Socket creation error");
		exit(1);
	}
	
	socket_address_in.sin_family = AF_INET; // network interaction, IPv4 protocol
	socket_address_in.sin_port = htons(3434); // port number in network byte order
	socket_address_in.sin_addr.s_addr = inet_addr("127.0.0.1"); // ip-address; "127.0.0.1" is "localhost"
	
	/*
	https://man7.org/linux/man-pages/man2/bind.2.html
	int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	bind (associate) a name (address), written in sockaddr, to a socket, return 0 if success, -1 if error
	*/
	if(bind(socket_listener, (SocketAddress*)&socket_address_in, sizeof(socket_address_in)) < 0) // binding to a network address
	{
		perror("Binding to the network address error");
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
	https://man7.org/linux/man-pages/man2/listen.2.html
	int listen(int sockfd, int backlog)
	listen for any connections to the socket w/ maximal queue for connections requests, return 0 if success, -1 if error
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
		https://man7.org/linux/man-pages/man2/select.2.html
		int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout)
		void FD_SET(int fd, fd_set *set)
		void FD_ZERO(fd_set *set)
		monitoring file descriptors to find ready for I/O operations, return number of file descriptors contained in the three returned descriptor sets/0 if timeout if success, -1 if error
		*/
		socket_detect = select(FD_SETSIZE, &fds, NULL, NULL, &time_value); // socket state check, number of asked handles, ready to read checking, time
		if(socket_detect == 0)
		{
			cout << "---------- MESSAGE REQUEST FROM CLIENT TIMEOUT ----------\n";
			/*
			https://man7.org/linux/man-pages/man2/close.2.html
			int close(int fd)
			closing a file descriptor of the socket, return 0 if success, -1 if error
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
				https://man7.org/linux/man-pages/man2/accept.2.html
				int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen)
				accepts a connection to the socket (extracts the 1st connection on a queue)
				return a file descriptor for the accepted socket (int >0) if success, -1 if error
				*/
				socket_fd = accept(socket_listener, NULL, NULL);
				if(socket_fd < 0)
				{
					perror("Connection establish error");
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
		https://man7.org/linux/man-pages/man2/close.2.html
		int close(int fd)
		closing a file descriptor of the socket, return 0 if success, -1 if error
		*/
		close(socket_listener);
	}
	
	return 0;
}
