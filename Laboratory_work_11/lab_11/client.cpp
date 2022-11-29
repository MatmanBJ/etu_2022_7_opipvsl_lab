#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>

using namespace std;

typedef struct sockaddr_in SocketAddressIn; // struct with server address w/ IP
typedef struct sockaddr SocketAddress; // struct with server address
typedef struct timeval TimeValue; // struct w/ max waiting time to sending a message

int main(int argc, char* argv[])
{
	// ---------- INITIALIZING & PREPARING ----------
	
	int i = 0; // standart variable for loop
	int socket_fd; // client's socket handle
	int socket_detect; // client's socket change
	int is_connected = 0; // is client connected to the socket
	int begin_time = 0; // begin time
	char message_send[10]; // message to send from client (this) to server
	char message_receive[10]; // treated message to recieve from server to client (this)
	fd_set readfds; // set of the handles
	SocketAddressIn socket_address_in; // struct with server address
	TimeValue time_value; // struct w/ max waiting time to sending a message
	
	// ---------- SOCKET CREATION ----------
	// creating socket, sill be sent to the server
	// setting the connetion port, protocol IPv4 and address to FIND server
	// this settings must be the same in the server part
	
	// https://stackoverflow.com/questions/52801380/srandtimenull-function
	srand(time(NULL));
	
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
	socket_fd = socket(AF_INET, SOCK_STREAM, 0); // socket creation, TCP/IP interaction, steam socket
	
	if(socket_fd < 0)
	{
		perror("Socket creation error!");
		exit(1);
	}
	
	socket_address_in.sin_family = AF_INET; // network interaction, IPv4 protocol
	socket_address_in.sin_port = htons(3434); // port number in network byte order
	socket_address_in.sin_addr.s_addr = inet_addr("127.0.0.1"); // ip-address; "127.0.0.1" is "localhost"
	
	// ---------- WAITING CONNECTION TO THE SERVER ----------
	// establishing a connection w/ server
	
	begin_time = time(NULL);
	
	cout << "---------- WAITING CONNECTION TO THE SERVER FOR 15 SECONDS ----------\n";
	
	/*
	https://www.opennet.ru/man.shtml?topic=connect&category=2&russian=0
	https://man7.org/linux/man-pages/man2/connect.2.html
	#include <sys/socket.h>
	int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	sockfd -- socket file descriptor
	addr -- address structure that contains address of sockfd
	addrlen -- length of addr
	purpose: initiate a connection on a socket
	return: if the connection or binding succeeds, zero is returned, on error, -1 is returned, and errno is set to indicate the error
	*/
	while ((time(NULL) - begin_time) < 15
	&& (is_connected = connect(socket_fd, (SocketAddress*)&socket_address_in, sizeof(socket_address_in))) < 0) // waiting time to connect server -- 15 seconds
	{}
	
	if (is_connected == -1)
	{
		cout << "---------- COULDN'T CONNECT TO THE SERVER ----------\n";
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
		exit(-1);
	}
	
	// ---------- CREATING SEQUENCE & SENDING MESSAGE TO SERVER ----------
	// creating random sequence w/ 10 numbers
	// setting timeout for treatment in server,
	// sending message and output in in terminal
	
	// https://stackoverflow.com/questions/4629050/convert-an-int-to-ascii-character
	for (i = 0; i < 10; i++)
	{
		message_send[i] = '0' + rand()%10;
	}
	
	// https://stackoverflow.com/questions/4629050/convert-an-int-to-ascii-character
	// 90... + 10... is for situation, when number is starts w/ 0... and it will be less, than 10 characters
	//sprintf(message, "%lu", rand()%9000000000 + 1000000000); // creating random number sequence
	
	// setting waiting time to work in server part
	time_value.tv_sec = 15;
	time_value.tv_usec = 0;
	
	/*
	https://www.opennet.ru/man.shtml?topic=setsockopt&category=2&russian=0
	https://man7.org/linux/man-pages/man3/setsockopt.3p.html
	#include <sys/socket.h>
	int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
	socket -- socket file descriptor
	level -- argument specifies the protocol level at which the option resides
	option_name -- argument specifies a single option to set
	option_value -- value
	option_len -- length
	purpose: set the socket options
	return: upon successful completion, setsockopt() shall return 0, otherwise, -1 shall be returned and errno set to indicate the error
	*/
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&time_value, sizeof(time_value)); // setting a parameters for socket before sending
	
	/*
	https://www.opennet.ru/man.shtml?topic=send&category=2&russian=0
	https://man7.org/linux/man-pages/man2/send.2.html
	#include <sys/socket.h>
	ssize_t send(int sockfd, const void *buf, size_t len, int flags)
	sockfd -- the file descriptor of the sending socket
	buf -- the message is found in buf and has length len
	len -- the message is found in buf and has length len
	flags -- flags
	purpose: send a message on a socket
	return: on success, return the number of bytes sent; on error, -1 is returned, and errno is set to indicate the error
	*/
	send(socket_fd, message_send, sizeof(message_send), 0); // sending a message to server
	
	cout << "---------- MESSAGE HAS BEEN SENDED ----------\n"
	<< "---------- BEGIN MESSAGE ----------\n"
	<< message_send << "\n" // sended message from client (this) to server output
	<< "---------- END MESSAGE ----------\n";
	
	// ---------- RECIEVING MESSAGE FROM SERVER ----------
	// waiting a responce from server,
	// setting timeout to receive message and output recieved treated (ordered) message
	
	FD_ZERO(&readfds); // freeing set of the handles (setting it to zero)
	FD_SET(socket_fd, &readfds); // adding the handle into the set of the handles (adding socket, which was argument in this function)
	
	// setting waiting time to receive responce message from server to client (this)
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
	purpose: synchronous I/O multiplexing (waiting for change in the handles)
	return: on success, return the number of file descriptors contained in the three returned descriptor sets,
	  the return value may be zero if the timeout expired before any file descriptors became ready;
	  on error, -1 is returned, and errno is set to indicate the error
	*/
	socket_detect = select(FD_SETSIZE, &readfds, NULL, NULL, &time_value); // socket state detection (change) to get message from server to client (this)
	
	// if nothing (no message) -- output timeout
	// else (have message) -- output treated message
	if(socket_detect == 0)
	{
		cout << "---------- MESSAGE RECIEVE TIMEOUT ----------\n";
	}
	else
	{
		/*
		https://www.opennet.ru/man.shtml?topic=recv&category=2&russian=0
		https://man7.org/linux/man-pages/man2/recv.2.html
		#include <sys/socket.h>
		ssize_t recv(int sockfd, void *buf, size_t len, int flags)
		sockfd -- socket file descriptor (???)
		buf -- placing the received message into the buffer variabl buf
		len -- the size of the buffer in len
		flags -- flags
		purpose: receive a message from a socket
		return: the number of bytes received, or -1 if an error occurred, errno is set to indicate the error
		*/
		recv(socket_fd, message_receive, sizeof(message_receive), 0); // recieving a message
		cout << "---------- MESSAGE HAS BEEN RECIEVED ----------\n"
		<< "---------- BEGIN MESSAGE ----------\n"
		<< message_receive << "\n" // treated message from server to client (this) output
		<< "---------- END MESSAGE ----------\n";
	}
	
	// ---------- CLEANING & TERMINATING ----------
	
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
	return 0;
}
