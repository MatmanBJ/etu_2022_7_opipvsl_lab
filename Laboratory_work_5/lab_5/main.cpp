// start program
// ./main (signal | sigaction | <none=default>) (1 | 2 | <none=default>)
// ----------
// 1st
// signal -- signal function usage
// sigaction -- sigaction function usage
// <none=default> -- default (signal) function usage
// ----------
// 2nd
// 1 -- dividing by zero operation usage
// 2 -- adressing to nullptr operation usage
// <none=default> -- default (dividing by zero) operation usage

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

void ErrorHandler (int local);

int main (int argc, char *argv[])
{
	string type_func = argv[1];
	int type_err = atoi(argv[2]);
	
	// https://man7.org/linux/man-pages/man2/sigaction.2.html
	struct sigaction sig; // struct for handling sigaction
	
	sig.sa_handler = &ErrorHandler; // error (sigaction) handle function set
	
	// https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-sigaction-examine-change-signal-action
	// http://fkn.ktu10.com/?q=node/666
	// no flags added, because "the... ...mask with signal stays in effect
	// until the signal handler returns, or..."
	
	// https://stackoverflow.com/questions/45477254/how-sigaction-differs-from-signal
	// https://stackoverflow.com/questions/231912/what-is-the-difference-between-sigaction-and-signal
	if (type_func == "signal") // signal
	{
		signal(SIGFPE, ErrorHandler); // SIGFPE = 8, invalid operation (overflow, dividing by 0)
		signal(SIGSEGV, ErrorHandler); // SIGSEGV = 11, memory protection breach
	}
	else if (type_func == "sigaction") // sigaction
	{
		sigaction(SIGFPE, &sig, NULL); // SIGFPE = 8, invalid operation (overflow, dividing by 0)
		sigaction(SIGSEGV, &sig, NULL); // SIGSEGV = 11, memory protection breach
	}
	else // signal by default
	{
		signal(SIGFPE, ErrorHandler); // SIGFPE = 8, invalid operation (overflow, dividing by 0)
		signal(SIGSEGV, ErrorHandler); // SIGSEGV = 11, memory protection breach
	}
	
	switch(type_err)
	{
		case 1: // invalid operation (overflow, dividing by 0): dividing by zero
		{
			int number = 1;
			int zero = 0;
			int res = 0;
			res = number/zero;
			break;
		}
		case 2: // memory protection breach: trying to adress to nullptr pointer
		{
			char *c = nullptr;
			*c = 'z';
			break;
		}
		default: // dividing by zero by default
		{
			int number = 1;
			int zero = 0;
			int res = 0;
			res = number/zero;
			break;
		}
	}
	return 0;
}

void ErrorHandler (int local)
{
	switch(local)
	{
		case SIGFPE: // dividing by zero, SIGFPE function
		{
			puts("Invalid operation (overflow, dividing by 0): dividing by zero!");
			exit(1);
		}
		case SIGSEGV: // memory protection breach, SIGSEGV function
		{
			puts("Memory protection breach: trying to adress to nullptr pointer!");
			exit(2);
		}
		default: // unable to recognize signal
		{
			puts("Unable to recognize signal!");
			exit(3);
		}
	}
}
