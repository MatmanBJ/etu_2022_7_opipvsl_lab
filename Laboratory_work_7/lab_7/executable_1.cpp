// WARNING: create file in Linux to have "\n" ending (LF) instead of "\r\n" (CRLF) in Windows file
// LF -- line feed ("\n")
// CRLF -- carriage return line feed ("\r\n")

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>

using namespace std;

// false -- writing in pipe is NOT finished
// true -- writing in pipe is finished
bool pipe_write_is_finished = false; // end of writing indication
void LocalHandler (int local_int);

int main(int argc, char *argv[]) // getting output filename as parameter
{
	FILE* output_file_1 = fopen(argv[3], "w"); // file opening w/ write flag
	int sig; // for sigwait
	int pipe_read_is_done = 1; // if > 0, pipe read is NOT done, if < 0, is done
	int fildes[2]; // pipe channels handles, fildes[0] -- read from pipe, fildes[1] -- write to pipe
	char ch; // char buffer
	sigset_t b_set;
	sigset_t set;
	struct sigaction sigact;
	
	fildes[0] = *argv[1];
	fildes[1] = *argv[2];
	
	sigaddset(&set, SIGUSR1); // SIGUSR1 signal add to child process 1 set
	sigact.sa_handler = &LocalHandler; // setting new handler
	sigaction(SIGQUIT, &sigact, NULL); // changing function reaction to SIGQUIT
	sigaddset(&b_set, SIGQUIT); // SIGQUIT signal add to set
	sigprocmask(SIG_UNBLOCK, &b_set, NULL); // unblock SIGQUIT w/ set signals reaction
	
	cout << "---------- CHILD PROCESS 1 BEGINS WRITING DATA F/ THE PIPE TO FILE ----------\n";
	
	//pipe_read_is_done = read(fildes[0], &ch, 1); // reading from pipe
	while ((pipe_read_is_done = read(fildes[0], &ch, 1)) > 0 || pipe_write_is_finished == false) // while pipe writing by parent process is not finished & pipe not done
	{
		if (pipe_read_is_done > 0) // additional check pipe if is done (if previous while there was ||, not &&)
		{
			fputc(ch, output_file_1); // writing to the file
			kill(0, SIGUSR2); // child process 1 give signal to child process 2
			sigwait(&set, &sig); // child process 1 waits child process 2
		}
		//pipe_read_is_done = read(fildes[0], &ch, 1); // reading from pipe
	}
	
	kill(0, SIGUSR2); // child process 1 give signal to child process 2 FINAL BEFORE TERMINATION
	fclose(output_file_1); // close output file
	close(fildes[0]); // close pipe fo read
	exit(0);
}

void LocalHandler (int local_int)
{
	pipe_write_is_finished = true; // pipe writing is finished
}
