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

// shared struct (i.e. variable) for counting number of active processes,
// who wants to have read/write operation with the same file
typedef struct
{
	int process_number = 0; // number of working process, which connect to r/w
} SharedMemorySegmentBuffer;

int main (int argc, char* argv[])
{
	// ---------- INITIALIZING & PREPARING ----------
	
	/*
	 * SEMAPHORE CONVENTIONS (for all 3 semaphores):
	 * 0 -- FILE semaphore
	 * 1 -- ACTIVE WRITERS semaphore
	 * 2 -- ACTIVE READERS semaphore
	 */
	
	int active_processes_before_cleaning = 0; // if 0, the last active process delete semaphore and shared memory segment
	int shared_mem_seg_ptr; // pointer to the shared memory segment
	int semaphore_ptr; // pointer to the semaphore
	int key_shared_mem_seg = 160; // shared memory segment key
	int key_semaphore = 695; // semaphore key
	char local_buffer[80]; // buffer to read the file
	string filename = "shared_file.txt"; // name of the file to read strings
	ifstream local_file;
	SemaphoreBuffer ZeroNumOfWriters = {1, 0, 0}; // comparing ACTIVE WRITERS semaphore with 0 (flags = 0)
	SemaphoreBuffer DecNumOfReaders={2, -1, 0}; // DEcreasing ACTIVE READERS semaphore (flags = 0)
	SemaphoreBuffer IncNumOfReaders={2, 1, 0}; // INcreasing ACTIVE READERS semaphore (flags = 0)
	SharedMemorySegmentBuffer* shared_mem_seg_this_process;
	
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
		shared_mem_seg_this_process->process_number = 0; // THIS SEMAPHORE NEEDS TO BE REPLASED THIS IS THE ONE WHO KNOCKS
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
	
	// ---------- READING FILE ----------
	
	// if there is NO writers, who are ready to write, readers could access file together
	// if there IS writers, who are ready to write, other readers will wait access to the file
	
	cout << "---------- R/ PROCESS №" << getpid() << " IS WAITING FOR THE READY W/ PROCS ----------\n";
	semop(semaphore_ptr, &ZeroNumOfWriters, 1); // reader waits until writer will finish the reading
	
	semop(semaphore_ptr, &IncNumOfReaders, 1);// +1 writer process, who wants to read from the file
	
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
	
	semop(semaphore_ptr, &DecNumOfReaders, 1); // decreasing number of active readers of file
	
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
