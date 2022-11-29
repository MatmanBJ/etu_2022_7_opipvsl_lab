#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

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
	if (send(local_socket_fd, local_consequence_buffer, local_buffer_length, 0) > 0)
	{
		cout << "---------- MESSAGE HAS BEEN SENDED ----------\n"
		<< "---------- BEGIN MESSAGE ----------\n"
		<< local_consequence_buffer << "\n" // output treated message from client to server to subserver (this) to client
		<< "---------- END MESSAGE ----------\n";
	}
	else
	{
		cout << "---------- MESSAGE SENDING TO CLIENT ERROR ----------\n";
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
	close(local_socket_fd);
	exit(0);
}
