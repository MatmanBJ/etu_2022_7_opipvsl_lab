/*
 * ./writer number_of_strings interval_time
 *
 * number_of_strings
 *     Number of loops (cycles), i.e. how many times this (WRITER) program (process) will write strings in the file. Integer number in the range [0; +inf].
 * interval_time
 *     Interval time (as argument for "sleep()" function) for every loop (cycle), i.e. how many time we will wait after each iteration. Integer number in the range [-1; +inf].
 *
 */

// https://www.unix.com/aix/74632-how-clean-unused-semaphore.html
// WARNING
// if you have undeleted old semaphore w/ old data,
// and you start a new program, but there is infinite waiting
// input in terminal: "ipcrm -s <semaphore id>"
// usually id starts from 1, so you can try it
// ID IS NOT KEY (it's int ptr in program, not the key)

#include <iostream>
#include <fstream>
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/shm.h>

using namespace std;

/*
struct sembuf
{
	short sem_num; // Semaphore number
	short sem_op; // Operation on semaphore
	short sem_flg; // Operation flags
};
*/

/*
 * https://www.opennet.ru/docs/RUS/ipcbook/node34.html
 * http://ccfit.nsu.ru/~deviv/courses/unix/unix/ngc9deb.html
 * sem_num: semaphore number (index, id), for whom we setting the operations/flags
 * sem_op > 0: add sem_op
 * sem_op < 0: decrease by |sem_op|, if there will be number < 0 after that, it will return an error
 * sem_op == 0: compare to 0, if != 0, there will be an error
 * sem_flg == IPC_NOWAIT == 2048: if some operation couldn't be finished, there will be an error, nothing will be changed
 * sem_flg == SEM_UNDO == 4096: test operation regime, even if it's successful or not, it will be reseted (undo) to values before
 */

/*
 * http://ccfit.nsu.ru/~deviv/courses/unix/unix/ngc9deb.html
 * int semop(int semid, struct smbuf *sops, unsigned nsops);
 * nsops -- the number of structures in the array pointed to by sops, nsops must always be greater than or equal to 1;
 * semid -- semaphore set identifier received from semget;
 */

typedef struct sembuf SemaphoreBuffer;

int main (int argc, char* argv[])
{
	// ---------- INITIALIZING & PREPARING ----------
	
	// checking if there is wrong/incopatible arguments
	if (argv[1] == nullptr)
	{
		cout << "Syntax error. No number of strings argument has found!\n";
		exit(-1);
	}
	if (atoi(argv[1]) < 0)
	{
		cout << "Syntax error. Number of strings to write file argument must be in range [0; +inf)!\n";
		exit(-1);
	}
	
	// checking if there is wrong/incopatible arguments
	if (argv[2] == nullptr)
	{
		cout << "Syntax error. No interval time (for \"sleep()\" function) argument has found!\n";
		exit(-1);
	}
	if (atoi(argv[2]) < -1)
	{
		cout << "Syntax error. Interval time (for \"sleep()\" function) argument must be in range [-1; +inf)!\n";
		exit(-1);
	}
	
	/*
	 * SEMAPHORE CONVENTIONS (for all 3 semaphores):
	 * 0 -- FILE semaphore
	 * 1 -- ACTIVE WRITERS semaphore
	 * 2 -- ACTIVE READERS semaphore
	 * 3 -- ACTIVE PROCESSES semaphore
	 */
	
	int number_of_strings = atoi(argv[1]); // number of loops (cycles)
	int interval_time = atoi(argv[2]); // interval time (as argument for "sleep()" function) for every loop (cycle)
	int i = 0;
	int semaphore_ptr; // pointer to the semaphore
	int key_semaphore = 190; // semaphore key
	string filename = "shared_file.txt"; // name of the file to write strings
	ofstream local_file;
	SemaphoreBuffer semaphore_file_decrease = {0, -1, 0}; // DEcreasing ACCESS FOR ONLY 1 WRITER to FILE semaphore (flags = 0)
	SemaphoreBuffer semaphore_file_increase = {0, 1, 0}; // INcreasing ACCESS FOR ONLY 1 WRITER to FILE semaphore (flags = 0)
	SemaphoreBuffer semaphore_writers_decrease = {1, -1, 0}; // DEcreasing ACTIVE WRITERS semaphore (flags = 0)
	SemaphoreBuffer semaphore_writers_increase = {1, 1, 0}; // INcreasing ACTIVE WRITERS semaphore (flags = 0)
	SemaphoreBuffer semaphore_readers_compare = {2, 0, 0}; // comparing ACTIVE READERS semaphore with 0 (flags = 0)
	SemaphoreBuffer semaphore_processes_decrease = {3, -1, 0}; // DEcreasing ACTIVE PROCESSES semaphore (flags = 0)
	SemaphoreBuffer semaphore_processes_increase = {3, 1, 0}; // INcreasing ACTIVE PROCESSES semaphore (flags = 0)
	
	cout << "---------- WRITER PROCESS NUMBER " << getpid() << " ----------\n";
	cout << "---------- FILENAME TO WRITE IS " << filename << " ----------\n";
	cout << "---------- SEMAPHORE KEY IS " << (key_semaphore == IPC_PRIVATE ? "IPC_PRIVATE = " + to_string(key_semaphore) : to_string(key_semaphore)) << " ----------\n";
	
	// ---------- CREATING/OPENING SEMAPHORES ----------
	
	// https://ru.manpages.org/semget/2
	// int semget(key_t key, int nsems, int semflg);
	semaphore_ptr = semget(key_semaphore, 4, IPC_CREAT | IPC_EXCL | 0666); // creating set of 4 semaphores (file, acrive writers, active readers, active processes)
	
	if (semaphore_ptr != -1)
	{
		cout << "---------- SEMAPHORE ID = " << semaphore_ptr << " HAS BEEN CREATED BY PROCESS " << getpid() << " ----------\n";
		// when we create the semaphore, increasing it +1
		// this semaphore is only for writers: for tracking the possibility of writig in the file at the moment
		// when we enter the cycle (the loop), we dectreasing it -1, so it will be =0 (because we had +1 ONLY when created),
		// so the other ones (the other writers) can't make -1, when they reach the same point in the cycle,
		// because there is no IPC_NOWAIT flag, which returns error immediately, so other programs-writers MUST wait untill it will be +1,
		// so they could make -1 (when =0, they couldn't do that, because there couldn't be <0 in semaphore)
		semop(semaphore_ptr, &semaphore_file_increase, 1);
	}	
	else
	{
		semaphore_ptr = semget(key_semaphore, 4, IPC_CREAT); // opening semaphore
		if (semaphore_ptr != -1)
		{
			cout << "---------- SEMAPHORE ID = " << semaphore_ptr << " HAS BEEN OPENED BY PROCESS " << getpid() << " ----------\n";
		}
		else
		{
			cout << "---------- SEMAPHORE HAS NOT BEEN OPENED BY PROCESS " << getpid() << " ----------\n";
			exit(-1);
		}
	}
	
	// ---------- INCREASING PROCESSES SEMAPHORE NUMBER ----------
	
	// increase number of working processes,
	// because we started working w/ file, so other programs will know,
	// how many programs are active (i.e. not finished working w/ file part)
	semop(semaphore_ptr, &semaphore_processes_increase, 1);
	
	cout << "---------- NUMBER OF PROCESSES, CONNECTED TO R/W FILE IS " << semctl(semaphore_ptr, 3, GETVAL, 0) << " ----------\n\n";
	
	// ---------- WRITING FILE ----------
	
	for (i = 0; i < number_of_strings; i++)
	{
		semop(semaphore_ptr, &semaphore_writers_increase, 1); // +1 writer process, who wants to write into the file
		semop(semaphore_ptr, &semaphore_readers_compare, 1); // writer waits until reader will finish the reading
		
		cout << "---------- W/ PROCESS №" << getpid() << " IS WAITING FOR THE FILE SEMAPHORE ----------\n";
		// [see the semaphore creation part, i've made more detailed explanations]
		semop(semaphore_ptr, &semaphore_file_decrease, 1); // decrease, so other processes-WRITERs waiting to get the access to the file
		
		cout << "---------- OPEN FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " BEGIN ----------\n";
		local_file.open(filename, ios::app);
		cout << "---------- OPEN FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " END ----------\n";
		
		cout << "---------- WRITE STRING №" << i << " BY PROCESS №" << getpid() << " BEGIN ----------\n";
		local_file << "Written by process №" << getpid() << ". String №" << i << "\n";
		cout << "Written by process №" << getpid() << ". String №" << i << "\n";
		cout << "---------- WRITE STRING №" << i << " BY PROCESS №" << getpid() << " END ----------\n";
		
		cout << "---------- CLOSE FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " BEGIN ----------\n";
		local_file.close();
		cout << "---------- CLOSE FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " END ----------\n";
		
		cout << "---------- FILE & ACTIVE W/'S SEMAPHORE RELEASING BY W/ №" << getpid() << " BEGIN ----------\n";
		// [see the semaphore creation part, i've made more detailed explanations]
		semop(semaphore_ptr, &semaphore_file_increase, 1); // freeing the file holding by this process-WRITER
		semop(semaphore_ptr, &semaphore_writers_decrease, 1); // decreasing number of active writers of file
		cout << "---------- FILE & ACTIVE W/'S SEMAPHORE RELEASING BY W/ №" << getpid() << " END ----------\n\n";
		
		sleep(interval_time); // sleeping before next iteration by the time passed in the argument
	}
	
	// ---------- DECREASING PROCESSES SEMAPHORE NUMBER ----------
	
	// decrease number of working processes, because we finished shared file part
	// decreasing number in the semaphore number 4, so other programs will know,
	// how many programs are active (i.e. not finished working w/ file part)
	semop(semaphore_ptr, &semaphore_processes_decrease, 1);
	
	// ---------- CLEANING & TERMINATING ----------
	
	// so the last process, who see 0 in semaphore,
	// will delete semaphore and shared memory segment
	if (semctl(semaphore_ptr, 3, GETVAL, 0) == 0) // last process will delete semaphore and will free the shared memory segment
	{
		semctl(semaphore_ptr, IPC_RMID, 0); // deleting semaphore
		cout << "---------- SEMAPHORE HAS BEED DELETED ----------\n";
	}
	
	return 0;
}
