/*
 * ./writer number_of_strings interval_time
 *
 * number_of_strings
 *     Number of loops (cycles), i.e. how many times this (WRITER) program (process) will write strings in the file. Integer number in the range [0; +inf].
 * interval_time
 *     Interval time (as argument for "sleep()" function) for every loop (cycle), i.e. how many time we will wait after each iteration. Integer number in the range [-1; +inf].
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

// shared struct (i.e. variable) for counting number of active processes,
// who wants to have read/write operation with the same file
typedef struct
{
	int process_number = 0; // number of working process, which connect to r/w
} SharedMemorySegmentBuffer;

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
	 */
	
	int number_of_strings = atoi(argv[1]); // number of loops (cycles)
	int interval_time = atoi(argv[2]); // interval time (as argument for "sleep()" function) for every loop (cycle)
	int i = 0;
	int active_processes_before_cleaning = 0; // if 0, the last active process delete semaphore and shared memory segment
	int shared_mem_seg_ptr; // pointer to the shared memory segment
	int semaphore_ptr; // pointer to the semaphore
	int key_shared_mem_seg = 160; // shared memory segment key
	int key_semaphore = 695; // semaphore key
	string filename = "shared_file.txt"; // name of the file to write strings
	ofstream local_file;
	SemaphoreBuffer DecOfMutForFile = {0, -1, 0}; // DEcreasing FILE semaphore (flags = 0)
	SemaphoreBuffer IncOfMutForFile = {0, 1, 0}; // INcreasing FILE semaphore (flags = 0)
	SemaphoreBuffer DecNumOfWriters = {1, -1, 0}; // DEcreasing ACTIVE WRITERS semaphore (flags = 0)
	SemaphoreBuffer IncNumOfWriters = {1, 1, 0}; // INcreasing ACTIVE WRITERS semaphore (flags = 0)
	SemaphoreBuffer ZeroNumOfReaders = {2, 0, 0}; // comparing ACTIVE READERS semaphore with 0 (flags = 0)
	SharedMemorySegmentBuffer* shared_mem_seg_this_process;
	
	cout << "---------- WRITER PROCESS NUMBER " << getpid() << " ----------\n";
	cout << "---------- FILENAME TO WRITE IS " << filename << " ----------\n";
	cout << "---------- SHARED MEMORY SEGMENT KEY IS " << (key_shared_mem_seg == IPC_PRIVATE ? "IPC_PRIVATE = " + to_string(key_shared_mem_seg) : to_string(key_shared_mem_seg)) << " ----------\n";
	cout << "---------- SEMAPHORE KEY IS " << (key_semaphore == IPC_PRIVATE ? "IPC_PRIVATE = " + to_string(key_semaphore) : to_string(key_semaphore)) << " ----------\n";
	
	// ---------- CREATING/OPENING SHARED MEMORY SEGMENT ----------
	
	shared_mem_seg_ptr = shmget(key_shared_mem_seg, sizeof(SharedMemorySegmentBuffer), 0666 | IPC_CREAT | IPC_EXCL);
	
	// SHARED MEMORY SEGMENT is used for SHARED VARIABLE
	// SHARED VARIABLE is used for counting ACTIVE PROCESSES,
	// connected w/ this process or output file
	if (shared_mem_seg_ptr != -1)
	{
		// we don't use bool variable to indicate owner, because memory will be freed by the last process
		cout << "---------- SHARED MEMORY SEGMENT HAS BEEN CREATED ----------\n";
	}
	else
	{
		shared_mem_seg_ptr = shmget(key_shared_mem_seg, sizeof(SharedMemorySegmentBuffer), 0666 | IPC_CREAT);
		if(shared_mem_seg_ptr == -1)
		{
			cout << "---------- SHARED MEMORY SEGMENT HAS NOT BEEN OPENED ----------\n";
			exit(-1);
		}
		else
		{
			cout << "---------- SHARED MEMORY SEGMENT HAS BEEN OPENED ----------\n";
		}
	}
	
	// attaching means, that allocated shared memory segment
	// now is not only allocated (and we can write something there),
	// but now is the part of current process adress space,
	// so we can use more memory in process
	shared_mem_seg_this_process = (SharedMemorySegmentBuffer*)shmat(shared_mem_seg_ptr, 0, 0); // attach shared memory segment to the current process
	
	// ---------- CREATING/OPENING SEMAPHORES ----------
	
	// https://ru.manpages.org/semget/2
	// int semget(key_t key, int nsems, int semflg);
	semaphore_ptr = semget(key_semaphore, 3, IPC_CREAT | IPC_EXCL | 0666); // creating set of 3 semaphores (file, acrive writers, active readers)
	
	if (semaphore_ptr != -1)
	{
		cout << "---------- SEMAPHORE HAS BEEN CREATED BY PROCESS " << getpid() << " ----------\n";
		shared_mem_seg_this_process->process_number = 0;
		semop(semaphore_ptr, &IncOfMutForFile, 1); // get access to the file
	}	
	else
	{
		semaphore_ptr = semget(key_semaphore, 3, IPC_CREAT); // opening semaphore
		if (semaphore_ptr != -1)
		{
			cout << "---------- SEMAPHORE HAS BEEN OPENED BY PROCESS " << getpid() << " ----------\n";
		}
		else
		{
			cout << "---------- SEMAPHORE HAS NOT BEEN OPENED BY PROCESS " << getpid() << " ----------\n";
			exit(-1);
		}
	}
	
	// ---------- INCREASING SEMAPHORE NUMBER ----------
	
	// increase number of working processes, because we have access
	// increasing number in shared variable, so other programs will know,
	// how many programs have access to this process (memory) at the moment
	shared_mem_seg_this_process->process_number = shared_mem_seg_this_process->process_number + 1;
	
	cout << "---------- NUMBER OF PROCESSES, CONNECTED TO R/W FILE IS " << shared_mem_seg_this_process->process_number << " ----------\n\n";
	
	// ---------- WRITING FILE ----------
	
	for (i = 0; i < number_of_strings; i++)
	{
		semop(semaphore_ptr, &IncNumOfWriters, 1); // +1 writer process, who wants to write into the file
		semop(semaphore_ptr, &ZeroNumOfReaders, 1); // writer waits until reader will finish the reading
		
		cout << "---------- W/ PROCESS №" << getpid() << " IS WAITING FOR THE FILE MUTEX ----------\n";
		semop(semaphore_ptr, &DecOfMutForFile, 1); // writer process waiting to get the access to the file
		
		cout << "---------- OPEN FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " BEGIN ----------\n";
		local_file.open(filename, ios::app);
		cout << "---------- OPEN FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " END ----------\n";
		
		cout << "---------- WRITE STRING №" << i << " BY PROCESS №" << getpid() << " BEGIN ----------\n";
		local_file << "Written by process №" << getpid() << ". String №" << i << "\n";
		cout << "---------- WRITE STRING №" << i << " BY PROCESS №" << getpid() << " END ----------\n";
		
		cout << "---------- CLOSE FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " BEGIN ----------\n";
		local_file.close();
		cout << "---------- CLOSE FILE \"" << filename << "\" TO W/ BY PROCESS №" << getpid() << " END ----------\n";
		
		cout << "---------- FILE & ACTIVE W/'S SEMAPHORE RELEASING BY W/ №" << getpid() << " BEGIN ----------\n";
		semop(semaphore_ptr, &IncOfMutForFile, 1); // freeing the file holding by this process
		semop(semaphore_ptr, &DecNumOfWriters, 1); // decreasing number of active writers of file
		cout << "---------- FILE & ACTIVE W/'S SEMAPHORE RELEASING BY W/ №" << getpid() << " END ----------\n\n";
		
		sleep(interval_time); // sleeping before next iteration by the time passed in the argument
	}
	
	// ---------- DECREASING SEMAPHORE NUMBER ----------
	
	// decrease number of working processes, because we will lose access
	// decreasing number in shared variable, so other programs will know,
	// how many programs have access to this process (memory) at the moment
	shared_mem_seg_this_process->process_number = shared_mem_seg_this_process->process_number - 1;
	
	// fix number of processes from shared variable
	// so the last, who have 0 processes,
	// will delete semaphore
	// and will delete (free) shared memory segment
	active_processes_before_cleaning = shared_mem_seg_this_process->process_number;
	
	// ---------- SHARED MEMORY SEGMENT DETACH FROM THE PROGRAM MEMORY ----------
	
	// detaching means, that allocated shared memory segment
	// now is NOT the part of CURRENT process adress space,
	// but allocated, and we can write sometning there,
	// and if we want, we can attach it to THIS process again
	shmdt((void*)shared_mem_seg_this_process); // detach shared memory segment from current process
	
	// ---------- CLEANING & TERMINATING ----------
	
	// last process will delete semaphore and will free the shared memory segment
	if(active_processes_before_cleaning == 0)
	{
		semctl(semaphore_ptr, IPC_RMID, 0); // deleting semaphore
		shmctl(shared_mem_seg_ptr, IPC_RMID, 0); // delete (free) shared memory segment
		cout << "---------- SEMAPHORE HAS BEED DELETED ----------\n";
	}
	
	return 0;
}
