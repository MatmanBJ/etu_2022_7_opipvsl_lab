// WARNING: create file in Linux to have "\n" ending (LF) instead of "\r\n" (CRLF) in Windows file
// LF -- line feed ("\n")
// CRLF -- carriage return line feed ("\r\n")

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>

using namespace std;

void LocalHandler (int local_int);

int main(int argc, char *argv[])
{
	FILE* output_file_2 = fopen(argv[3], "w"); // file opening w/ write flag
	int sig; // for sigwait
	int fildes[2]; // pipe channels handles, fildes[0] -- read from pipe, fildes[1] -- write to pipe
	char ch; // char buffer
	sigset_t b_set;
	sigset_t set;
	struct sigaction sigact;
	
	fildes[0] = *argv[1];
	fildes[1] = *argv[2];
	
	sigaddset(&set, SIGUSR2); // SIGUSR2 signal add to child process 2 set
	sigact.sa_handler = &LocalHandler; // setting new handler
	sigaction(SIGQUIT, &sigact, NULL); // changing function reaction to SIGQUIT
	sigaddset(&b_set, SIGQUIT); // SIGQUIT signal add to set
	sigprocmask(SIG_UNBLOCK, &b_set, NULL); // unblock SIGQUIT w/ set signals reaction
	
	cout << "---------- CHILD PROCESS 2 BEGINS WRITING DATA F/ THE PIPE TO FILE ----------\n";
	
	sigwait(&set, &sig); // child process 2 waits child process 1 (1st waiting before reading)
	
	while(read(fildes[0], &ch, 1) > 0) // reading from pipe
	{
		fputc(ch, output_file_2); // writing to the file
		kill(0, SIGUSR1); // child process 2 give signal to child process 1
		sigwait(&set, &sig); // child process 2 waits child process 1
	}
	
	kill(0, SIGUSR1); // child process 2 give signal to child process 1 FINAL BEFORE TERMINATION
	fclose(output_file_2); // close output file
	close(fildes[0]); // close pipe fo read
	exit(0);
}

void LocalHandler (int local_int)
{}
