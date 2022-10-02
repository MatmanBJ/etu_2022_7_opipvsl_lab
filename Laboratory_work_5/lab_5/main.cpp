#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

void InvalidOperationError (int);
void MemoryProtectionError (int);

int main (int argc, char *argv[])
{
	string type_func = argv[1];
	int type_err = atoi(argv[2]);
	//структура для фунции перехвата sigaction
	struct sigaction for_sigfpe;
	struct sigaction for_sigsegv;
	
	//устанавливаем флаг игнорирования дополнительной сигнальной маски
	//и флаг сброса реакции после первого перехвата

	for_sigfpe.sa_flags = SA_NOMASK || SA_ONESHOT;
	for_sigsegv.sa_flags = SA_NOMASK || SA_ONESHOT;
	
	//задаём функцию обработки сигнала для функции перехвата sigaction

	for_sigfpe.sa_handler = &InvalidOperationError;
	for_sigsegv.sa_handler = &MemoryProtectionError;
	
	if (type_func == "signal") // signal
	{
		signal(SIGFPE, InvalidOperationError); // SIGFPE = 8, invalid operation (overflow, dividing by 0)
		signal(SIGSEGV, MemoryProtectionError); // SIGSEGV = 11, memory protection breach
	}
	else if (type_func == "sigaction") // sigaction
	{
		sigaction(SIGFPE, &for_sigfpe, NULL); // SIGFPE = 8, invalid operation (overflow, dividing by 0)
		sigaction(SIGSEGV, &for_sigsegv, NULL); // SIGSEGV = 11, memory protection breach
	}
	else // signal by default
	{
		signal(SIGFPE, InvalidOperationError); // SIGFPE = 8, invalid operation (overflow, dividing by 0)
		signal(SIGSEGV, MemoryProtectionError); // SIGSEGV = 11, memory protection breach
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
	}
	return 0;
}

void InvalidOperationError (int) // dividing by zero, SIGFPE function
{
	puts("Invalid operation (overflow, dividing by 0): dividing by zero!");
	exit(1);
}

void MemoryProtectionError (int) // memory protection breach, SIGSEGV function
{
	puts("Memory protection breach: trying to adress to nullptr pointer!");
	exit(2);
}
