// start program
// ./main <filename.txt>
// <filename.txt> -- name of the file with .txt extension, which will be read by program
// e.g. "./main lorem_ipsum.txt"

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

using namespace std;

int main(int argc, char *argv[])
{
	int fildes[2]; // pipe channels handles, fildes[0] -- read from pipe, fildes[1] -- write to pipe
	pid_t pid_1; // child process 1 pid
	pid_t pid_2; // child process 2 pid
	char ch[80]; // char buffer
	char *c = NULL; // file end indicator, if just "char *c;" instead of "char *c = NULL;" program won't end, idk why
	sigset_t set; // process' signals set
	FILE* fp = NULL; // file for parent process' read
	
	fp = fopen(argv[1], "r"); // file opening w/ read flag
	
	if (fp == NULL)
	{
		cout << "---------- FILE HAS NOT BEEN OPENED SUCCESSFULLY ----------\n";
		exit(2);
	}
	else
	{
		cout << "---------- FILE HAS BEEN OPENED SUCCESSFULLY ----------\n";
	}
	
	// adding sync signals to process set
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGUSR2);
	sigprocmask(SIG_BLOCK, &set, NULL); // blocking signals in mask
	
	// creating pipe
	if(pipe2(fildes, O_NONBLOCK) == -1) // if program can't create pipe, then exit w/ error
	{
		cout << "---------- PIPE HAS NOT BEEN CREATED SUCCESSFULLY ----------\n"; // message about creating pipe unsuccessfully/not creating pipe
		exit(1);
	}
	else // if program can create pipe, then continue execution
	{
		cout << "---------- PIPE HAS BEEN CREATED SUCCESSFULLY ----------\n"; // message about creating pipe successfully/not creating pipe
		pid_1 = fork(); // child process 1 creation
		if(pid_1 == 0)
		{
			cout << "---------- CHILD PROCESS 1 BEGINS ----------\n";
			close(fildes[1]); // closing pipe to write by child process 1
			execl("executable_1", "executable_1", &fildes[0], &fildes[1], "output_file_1.txt", NULL);
		}
		else
		{
			pid_2 = fork(); // child process 2 creation
			if(pid_2 == 0)
			{
				cout << "---------- CHILD PROCESS 2 BEGINS ----------\n";
				close(fildes[1]); // closing pipe to write by child process 2
				execl("executable_2", "executable_2", &fildes[0], &fildes[1], "output_file_2.txt", NULL);
			}
		}
		
		close(fildes[0]); // closing pipe to read by parent process
		cout << "---------- PARENT PROCESS BEGINS WRITING DATA TO THE PIPE ----------\n";
		c = fgets(ch, 80, fp); // get data from file & check if file is ended
		while(c) // writing data from <filename>
		{
			write(fildes[1], ch, strlen(ch) - 1); // writing data to the pipe
			c = fgets(ch, 80, fp); // get data from file & check if file is ended
		}
		cout << "---------- PARENT PROCESS ENDS WRITING DATA TO THE PIPE ----------\n";
		
		kill(pid_1, SIGQUIT);
		kill(pid_2, SIGQUIT);
		
		waitpid(pid_1, NULL, 0); // waiting child process 1 termination
		cout << "---------- CHILD PROCESS 1 ENDS ----------\n";
		waitpid(pid_2, NULL, 0); // waiting child process 2 termination
		cout << "---------- CHILD PROCESS 2 ENDS ----------\n";
		
		close(fildes[1]); // closing pipe to write by parent process
	}
	
	return 0;
}
