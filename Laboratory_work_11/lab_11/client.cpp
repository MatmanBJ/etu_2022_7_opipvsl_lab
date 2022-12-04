/*
 * ./client
 *
 * could be many clients
 *
 */

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
	https://man7.org/linux/man-pages/man2/socket.2.html
	int socket(int domain, int type, int protocol)
	creates socket (w/ domain in domain, protocol in protocol), return file descriptor for the new socket if successful, -1 if error
	AF_INET -- IPv4
	SOCK_STREAM -- two endpoints bytes stream
	*/
	socket_fd = socket(AF_INET, SOCK_STREAM, 0); // socket creation, TCP/IP interaction, steam socket
	
	if(socket_fd < 0)
	{
		perror("Socket creation error");
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
	https://man7.org/linux/man-pages/man2/connect.2.html
	int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	initiates connection w/ socket, w/ socket address & address length (size), return 0 if success, -1 if error
	*/
	while ((time(NULL) - begin_time) < 15
	&& (is_connected = connect(socket_fd, (SocketAddress*)&socket_address_in, sizeof(socket_address_in))) < 0) // waiting time to connect server -- 15 seconds
	{}
	
	if (is_connected == -1)
	{
		cout << "---------- COULDN'T CONNECT TO THE SERVER ----------\n";
		/*
		https://man7.org/linux/man-pages/man2/close.2.html
		int close(int fd)
		closing a file descriptor of the socket, return 0 if success, -1 if error
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
	https://man7.org/linux/man-pages/man3/setsockopt.3p.html
	int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
	sets the socket options (written in option_name) w/ specified protocol (writter in level), return 0 if success, -1 if error
	SOL_SOCKET -- options at the socket level
	SO_SNDTIMEO -- setting timeout for socket (in option_value & option_len)
	*/
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&time_value, sizeof(time_value)); // setting a parameters for socket before sending
	
	/*
	https://man7.org/linux/man-pages/man2/send.2.html
	ssize_t send(int sockfd, const void *buf, size_t len, int flags)
	send message (located in buffer) on a socket w/ concrete legnth (size), return the number of bytes sent if success, -1 if error
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
	https://man7.org/linux/man-pages/man2/select.2.html
	int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout)
	void FD_SET(int fd, fd_set *set)
	void FD_ZERO(fd_set *set)
	monitoring file descriptors to find ready for I/O operations, return number of file descriptors contained in the three returned descriptor sets/0 if timeout if success, -1 if error
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
		https://man7.org/linux/man-pages/man2/recv.2.html
		ssize_t recv(int sockfd, void *buf, size_t len, int flags)
		recieves a message (writes in buffer) from socket w/ concrete length (size), return the number of bytes recieved if success, -1 if error
		*/
		recv(socket_fd, message_receive, sizeof(message_receive), 0); // recieving a message
		cout << "---------- MESSAGE HAS BEEN RECIEVED ----------\n"
		<< "---------- BEGIN MESSAGE ----------\n"
		<< message_receive << "\n" // treated message from server to client (this) output
		<< "---------- END MESSAGE ----------\n";
	}
	
	// ---------- CLEANING & TERMINATING ----------
	
	/*
	https://man7.org/linux/man-pages/man2/close.2.html
	int close(int fd)
	closing a file descriptor of the socket, return 0 if success, -1 if error
	*/
	close(socket_fd);
	return 0;
}
