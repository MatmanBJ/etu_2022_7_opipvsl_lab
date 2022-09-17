#include <iostream>
#include <fstream>
#include <unistd.h> // delay, process attributes
#include <sys/types.h> // process attributes
#include <sys/wait.h> // pid_t types

using namespace std;

int main(int argc, char *argv[])
{
	ofstream file_stream; // file stream
    file_stream.open(argv[0], file_stream.out | file_stream.app); // file open & write
	sleep(atoi(argv[1])); // "child_2" delay
	
	file_stream << "\n***CHILD_2***\n\n"
	<< "Process ID (PID): " << getpid() << "\n"
	<< "Parent process ID (PPID): " << getppid() << "\n"
	<< "Session ID (SID): " << getsid(getpid()) << "\n"
	<< "Process group ID (PGID): " << getpgid(getpid()) << "\n"
	<< "[real] User ID (UID): " << getuid() << "\n"
	<< "Effective user ID (EUID): " << geteuid() << "\n"
	<< "[real] Group ID (GID): " << getgid() << "\n"
	<< "Effective group ID (EGID): " << getegid() << "\n";

    file_stream.close();			  
	return 0;
}
