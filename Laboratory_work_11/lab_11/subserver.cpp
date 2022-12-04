/*
 * not for launch from terminal
 * launches, when forks from main server
 */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <ctime>

using namespace std;

typedef struct timeval TimeValue;

int main(int argc, char* argv[])
{
	// ---------- INITIALIZING & PREPARING ----------
	
	int local_buffer_length; // length (size) of local buffer
	int local_socket_fd = atoi(argv[1]); // client's socket handle
	char local_symbol = '0'; // additional variable to save old number in sorting algorithm ('0' here just for fun)
	char local_consequence_buffer[10]; // number consequence local buffer
	
	// ---------- RECIEVING MESSAGE FROM CLIENTS & SEQUENCE TREATMENT & SENDING MESSAGE TO CLIENTS ----------
	// reading a message from client (which was in server, but we forked process, so we have it there)
	// then we sorting it out (from unordered numbers to ordered numbers 0..9)
	// after that we sending it back (we recieved as ARGUMENT a client's address,
	// where the message was from and where we need to send it after treatment)
	
	TimeValue time_value;
	time_value.tv_sec = 5;
	time_value.tv_usec = 0;
	setsockopt(local_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_value, sizeof(time_value)); // setting a parameters for socket-reciever
	
	/*
	https://man7.org/linux/man-pages/man2/recv.2.html
	ssize_t recv(int sockfd, void *buf, size_t len, int flags)
	recieves a message (writes in buffer) from socket w/ concrete length (size), return the number of bytes recieved if success, -1 if error
	*/
	local_buffer_length = recv(local_socket_fd, local_consequence_buffer, 10, 0); // message recieving
	cout << "---------- MESSAGE HAS BEEN RECIEVED ----------\n"
	<< "---------- BEGIN MESSAGE ----------\n"
	<< local_consequence_buffer << "\n" // output UNtreated message from client to server to subserver (this) to client
	<< "---------- END MESSAGE ----------\n";
	
	// by the 3rd exercice we need to get (server) number consequence
	// sort it, and then return (to client) the ordered number consequence
	// so we have there a classic sorting algorithm to make from
	// unordered consequence the ordered consequence of numbers
	// e.g. "4278600937" --> "0023467789"
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			// https://stackoverflow.com/questions/5029840/convert-char-to-int-in-c-and-c
			if ((local_consequence_buffer[j] - '0') > (local_consequence_buffer[j + 1] - '0'))
			{
				local_symbol = local_consequence_buffer[j];
				local_consequence_buffer[j] = local_consequence_buffer[j + 1];
				local_consequence_buffer[j + 1] = local_symbol;
			}
		}
	}
	
	/*
	https://man7.org/linux/man-pages/man2/send.2.html
	ssize_t send(int sockfd, const void *buf, size_t len, int flags)
	send message (located in buffer) on a socket w/ concrete legnth (size), return the number of bytes sent if success, -1 if error
	*/
	if (send(local_socket_fd, local_consequence_buffer, local_buffer_length, 0) > 0)
	{
		cout << "---------- MESSAGE HAS BEEN SENDED ----------\n"
		<< "---------- BEGIN MESSAGE ----------\n"
		<< local_consequence_buffer << "\n" // output treated message from client to server to subserver (this) to client
		<< "---------- END MESSAGE ----------\n\n";
	}
	else
	{
		cout << "---------- MESSAGE SENDING TO CLIENT ERROR ----------\n\n";
	}
	
	// ---------- CLEANING & TERMINATING ----------
	
	/*
	https://man7.org/linux/man-pages/man2/close.2.html
	int close(int fd)
	closing a file descriptor of the socket, return 0 if success, -1 if error
	*/
	close(local_socket_fd);
	exit(0);
}
