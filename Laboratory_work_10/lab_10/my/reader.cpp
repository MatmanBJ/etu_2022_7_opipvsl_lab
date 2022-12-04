/*
 * ./reader
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/shm.h>

using namespace std;

typedef struct sembuf SemaphoreBuffer;

int main (int argc, char* argv[])
{
	// ---------- INITIALIZING & PREPARING ----------
	
	/*
	 * SEMAPHORE CONVENTIONS (for all 3 semaphores):
	 * 0 -- FILE semaphore
	 * 1 -- ACTIVE WRITERS semaphore
	 * 2 -- ACTIVE READERS semaphore
	 * 3 -- ACTIVE PROCESSES semaphore
	 */
	
	int semaphore_ptr; // pointer to the semaphore
	int key_semaphore = 190; // semaphore key
	char local_buffer[80]; // buffer to read the file
	string filename = "shared_file.txt"; // name of the file to read strings
	ifstream local_file;
	SemaphoreBuffer semaphore_file_increase = {0, 1, 0}; // INcreasing ACCESS FOR ONLY 1 WRITER to FILE semaphore (flags = 0)
	SemaphoreBuffer semaphore_writers_compare = {1, 0, 0}; // comparing ACTIVE WRITERS semaphore with 0 (flags = 0)
	SemaphoreBuffer semaphore_readers_decrease = {2, -1, 0}; // DEcreasing ACTIVE READERS semaphore (flags = 0)
	SemaphoreBuffer semaphore_readers_increase = {2, 1, 0}; // INcreasing ACTIVE READERS semaphore (flags = 0)
	SemaphoreBuffer semaphore_processes_decrease = {3, -1, 0}; // DEcreasing ACTIVE PROCESSES semaphore (flags = 0)
	SemaphoreBuffer semaphore_processes_increase = {3, 1, 0}; // INcreasing ACTIVE PROCESSES semaphore (flags = 0)
	
	cout << "---------- READER PROCESS NUMBER " << getpid() << " ----------\n";
	cout << "---------- FILENAME TO READ IS " << filename << " ----------\n";
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
	
	// ---------- READING FILE ----------
	
	// here we are waiting only writers, but we aren't tracking other readers
	// (as it was for writers w/ semaphore, where it increasing +1 in the creation part),
	// because reading operation (instead of writing) doesn't require that
	// so we have situation, where if NO writers -- ALL readers read,
	// but if there IS writer -- ONLY 1 reader reads
	// it's because in this situation there is a queue of processes, where (most probably) writer will be next, not reader
	
	cout << "---------- R/ PROCESS №" << getpid() << " IS WAITING FOR THE READY W/ PROCS ----------\n";
	
	semop(semaphore_ptr, &semaphore_writers_compare, 1); // reader waits until writer will finish the reading
	semop(semaphore_ptr, &semaphore_readers_increase, 1); // +1 writer process, who wants to read from the file
	
	cout << "---------- OPEN FILE \"" << filename << "\" TO R/ BY PROCESS №" << getpid() << " BEGIN ----------\n";
	local_file.open(filename);
	cout << "---------- OPEN FILE \"" << filename << "\" TO R/ BY PROCESS №" << getpid() << " END ----------\n";
	
	cout << "---------- READ STRINGS BY PROCESS №" << getpid() << " BEGIN ----------\n";
	while (local_file.getline(local_buffer, 80))
	{
		cout << local_buffer << "\n";
		sleep(1);
	}
	cout << "---------- READ STRINGS BY PROCESS №" << getpid() << " END ----------\n";
	
	cout << "---------- CLOSE FILE \"" << filename << "\" TO R/ BY PROCESS №" << getpid() << " BEGIN ----------\n";
	local_file.close();
	cout << "---------- CLOSE FILE \"" << filename << "\" TO R/ BY PROCESS №" << getpid() << " END ----------\n\n";
	
	semop(semaphore_ptr, &semaphore_readers_decrease, 1); // decreasing number of active readers of file
	
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
