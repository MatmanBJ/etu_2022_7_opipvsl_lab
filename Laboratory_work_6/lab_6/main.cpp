#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

void processFunction (int local_int); // function for repeat
float finalTime (float local_start_time); // counting final time

int main(int argc, char* argv[])
{
	int period = 0; // launch period, 0 by default
	int numOfL = 0; // number of launches, 0 by default
	float start_time; // child start time
	struct itimerval cur;
	struct itimerval old;
	struct sigaction sig;

	start_time = ((float)clock()) / CLOCKS_PER_SEC; // measuring the current time
	
	period = atoi(argv[1]); // convert input data to numeric
	numOfL = atoi(argv[2]); // convertiong input data to numeric

	sig.sa_handler = processFunction; // setting handling function
	sigemptyset(&sig.sa_mask); // cleaning "set" from all signals
	
	sigaddset(&sig.sa_mask, SIGTSTP); // SIGTSTP signal adding to mask
	
	sigprocmask(SIG_BLOCK, &sig.sa_mask, NULL); // SIGTSTP signal blocking
	sig.sa_flags = 0; // no flags added
	
	// !!! need to change 0 to NULL for "my code style"
	sigaction(SIGALRM, &sig, 0); // setting responce to SIGALRM signal

	cur.it_interval.tv_sec = period; // период перезапуска
	cur.it_interval.tv_usec = 0;
	cur.it_value.tv_usec = 1;
	cur.it_value.tv_sec = 0; // первый запуск через period сек
	setitimer(ITIMER_REAL, &cur, &old);
	
	for (int i = 0; i < numOfL; ++i)
	{
		pause(); // SIGALRM signal waiting
		cout << "Parent process' work time (seconds): " << finalTime(start_time) << "\n\n";
	}

	return 0;
}

void processFunction (int local_int) // function for repeat
{
	int local_status; // status for "waitpid" func
	pid_t local_pid = fork(); // creating child process

	if (local_pid == 0) // if child is created
	{
		float local_start_time = 0;
		sigset_t set;
		time_t seconds = time(NULL);
		struct tm* timeinfo = localtime(&seconds);
		sigemptyset(&set);
		
		local_start_time = ((float)clock()) / CLOCKS_PER_SEC;
		
		cout << "Child process' PID: " << getpid() << "\n";
		cout << "Parent process' work start time: " << asctime(timeinfo); //<< "\n";

		sigaddset(&set, SIGTSTP);
		sigprocmask(SIG_BLOCK, &set, NULL);
		
		cout << "Child process' work time (seconds): " << finalTime(local_start_time) << "\n";
		
		exit(EXIT_SUCCESS);
	}
	else // else if it is not child or child is not created
	{
		waitpid(local_pid, &local_status, 0); // wait until child is terminate its work
	}
}

float finalTime (float local_start_time)
{
	//float local_end_time = 0;
	//local_end_time = ((float)clock()) / CLOCKS_PER_SEC);
	return ((((float)clock()) / CLOCKS_PER_SEC) - local_start_time);
}
