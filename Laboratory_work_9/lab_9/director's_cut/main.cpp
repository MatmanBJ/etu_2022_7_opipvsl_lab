/*
 * ./main -id (0 | 1 | 2) -time interval_time -num number_of_strings -file *.txt -key shared_mem_seg_key_value
 *
 * -id
 *     Program (process) ID. It can be only 0, 1 or 2.
 * -time
 *     Interval time for every loop (cycle), i.e. how many times we will wait after start new iteration. Integer number in the range [-1; +inf].
 * -num
 *     Number of loops (cycles), i.e. how many times program will write strings in the file. Integer number in the range [0; +inf].
 * -file
 *     Filename for 3 programs to write string, with .txt extension.
 * -key
 *     Key number for shared memory segment, if 0 = IPC_PRIVATE, it won't work (probably). Integer number in the range [0; +inf].
 *
 * ./main -id 0 -time 5 -num 5 -file shared_file.txt -key 190
 * ./main -id 1 -time 2 -num 3 -file shared_file.txt -key 190
 * ./main -id 2 -time 2 -num 5 -file shared_file.txt -key 190
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/shm.h>

using namespace std;

bool hasEnding (string const &fullString, string const &ending);

class ArgFlagParse
{
public:
	ArgFlagParse();
	ArgFlagParse(int local_flags_number, string* local_flags_array);
	~ArgFlagParse();
	void ArgvCommonCheck (int local_argc, char* local_argv[]);
	void ArgvCommonCheck (int local_argc, char* local_argv[], string* local_flags);
	bool ArgvConditionCheck (int local_flag, char* local_argv);
	bool ArgvConditionCheckID (char* local_argv);
	bool ArgvConditionCheckTime (char* local_argv);
	bool ArgvConditionCheckNum (char* local_argv);
	bool ArgvConditionCheckFile (char* local_argv);
	bool ArgvConditionCheckKey (char* local_argv);
	bool getErrorExistance ();
	string getErrorMessage ();
	int getFlagsNumber ();
	void setFlagsNumber (int local_flags_number);
	string* getFlagsArray ();
	void setFlagsArray (string* local_flags_array);
private:
	bool error_existance;
	string error_message;
	int flags_number;
	string* flags_array;
};

typedef struct // struct for lamport algorithm
{
	bool choosing[3]; // array w/ variables, which indicates process is BUSY w/ choosing
	int number[3]; // array w/ variables w/ token (= priority* = number in queue for access the file) numbers
	// * -- word "priority" in this program also means "j" number in loop
} MrLamportIsBaker;

int main (int argc, char *argv[])
{
	// ---------- PREPARING ----------
	
	string flags [] = {"-id", "-time", "-num", "-file", "-key"}; // automatic size
	ArgFlagParse* arguments = new ArgFlagParse();
	arguments->ArgvCommonCheck(argc, argv, flags);
	
	if (arguments->getErrorExistance() == true)
	{
		cout << arguments->getErrorMessage() << "\n";
		exit(-1);
	}
	
	delete arguments;
	
	// https://stackoverflow.com/questions/1195675/convert-a-char-to-stdstring
	int program_id = atoi(argv[2]); // program id/number
	int interval_time = atoi(argv[4]); // interval time to wait before next start
	int number_of_strings = atoi(argv[6]); // number of strings to write in the file
	int key = atoi(argv[10]); // key number for shared memory segment
	string filename = string(argv[8]); // name of the file to write strings
	
	//int program_id = atoi(argv[1]); // program id/number
	//int interval_time = atoi(argv[2]); // interval time to wait before next start
	//int number_of_strings = atoi(argv[3]); // number of strings to write in the file
	//int key = 190; // key number for shared memory segment
	//string filename = "shared_file.txt"; // name of the file to write strings
	
	bool shared_mem_seg_owner; // is this process is owner of the shared memory segment (to free it at the end)
	int shared_mem_seg_ptr; // pointer to the shared memory segment
	int i = 0; // for loop
	int j = 0; // for loop
	int k = 0; // for loop
	int local_token = -1; // local token number
	string local_string = "Written by program number " + to_string(program_id) + "\n"; // string to write in file
	MrLamportIsBaker* shared_mem_seg_this_process; // lamport algorithm additional variables
	
	cout << "---------- PROGRAM NUMBER " << program_id << " ----------\n";
	cout << "---------- OUTPUT FILENAME IS " << filename << " ----------\n";
	cout << "---------- INTERVAL TIME/NUMBER OF STRINGS IS " << interval_time << "/" << number_of_strings << " ----------\n";
	cout << "---------- KEY IS " << (key == IPC_PRIVATE ? "IPC_PRIVATE = " + to_string(key) : to_string(key)) << " ----------\n";
	
	// ---------- CREATING/OPENING SHARED MEMORY SEGMENT ----------
	
	shared_mem_seg_ptr = shmget(key, sizeof(MrLamportIsBaker), 0666 | IPC_CREAT | IPC_EXCL);
	/*
	 * 0400 -- allowed to read to the user who owns shared memory;
	 * 0200 -- write allowed to the user who owns shared memory;
	 * 0040 -- Reading is allowed for users included in that the same group as the owner of the shared memory;
	 * 0020 -- write allowed to users who are members of the same the same group as the owner of the shared memory;
	 * 0004 -- all other users are allowed to read;
	 * 0002 -- all other users are allowed to write;
	 */
	
	if (shared_mem_seg_ptr != -1)
	{
		shared_mem_seg_owner = true;
		cout << "---------- SHARED MEMORY SEGMENT HAS BEEN CREATED ----------\n\n";
	}
	else
	{
		shared_mem_seg_ptr = shmget(key, sizeof(MrLamportIsBaker), 0666 | IPC_CREAT);
		if(shared_mem_seg_ptr == -1)
		{
			cout << "---------- SHARED MEMORY SEGMENT HAS NOT BEEN OPENED ----------\n\n";
			exit(-1);
		}
		else
		{
			cout << "---------- SHARED MEMORY SEGMENT HAS BEEN OPENED ----------\n\n";
		}
	}
	/*
	 * https://man7.org/linux/man-pages/man2/shmget.2.html
	 * https://www.opennet.ru/man.shtml?topic=shmget&category=2&russian=0
	 * int shmget(key_t key, int size, int shmflg);
	 * on success, a valid shared memory identifier is returned
	 * on error, "-1" is returned, and "errno" is set to indicate the error
	 */
	
	// ---------- SHARED MEMORY SEGMENT ATTACH TO THE PROGRAM MEMORY (UNITE THEM) ----------
	
	shared_mem_seg_this_process = (MrLamportIsBaker*)shmat(shared_mem_seg_ptr, 0, 0);
	/*
	 * https://www.opennet.ru/man.shtml?topic=shmat&category=2&russian=0
	 * https://ru.manpages.org/shmat/2
	 * The "shmat" function attaches the shared memory segment with id = "shmid"
	 * to the address space of the calling process
	 */
	
	// ---------- LAMPORTH'S ALGORITHM ----------
	
	for (i = 0; i < number_of_strings; i++) // loop w/ number of strings to write in file = "-num" flag
	{
		// https://www.javatpoint.com/lamports-bakery-algorithm
		// all "entering" ("choosing") variables are initialized to false,
		// and n integer variables "numbers" ("number") are all initialized to 0
		// the value of integer "number" variables is used to form token numbers
		sleep(interval_time); // sleep w/ file write interval = "-time" flag
		shared_mem_seg_this_process->choosing[program_id] = true; // set choosing[???] to true to make other processes aware that it is choosing a token number
		local_token = -1;
		
		for (k = 0; k < 3; k++)
		{
			// when a process wishes to enter a critical section,
			// it chooses a greater token number than any earlier number
			if (shared_mem_seg_this_process->number[k] > local_token)
			{
				local_token = shared_mem_seg_this_process->number[k]; // choosing maximal token number
			}
		}
		
		shared_mem_seg_this_process->number[program_id] = local_token + 1; // choosing greater token number
		shared_mem_seg_this_process->choosing[program_id] = false; // sets choosing[???] to false after writing token number
		
		// waiting for other processes
		for(j = 0; j < 3; j++) // process enters a loop to evaluate the status of other processes
		{
			// process "i" waits until some other process "j" is choosing its token number
			while(shared_mem_seg_this_process->choosing[j] == true)
			{}
			
			// process "i" then waits until all processes with
			// smaller token numbers or the same token number
			// but with higher priority (here -- id or "j") are served fast
			while((shared_mem_seg_this_process->number[j] != 0)
			&& ((shared_mem_seg_this_process->number[j] < shared_mem_seg_this_process->number[program_id])
			|| ((shared_mem_seg_this_process->number[j] == shared_mem_seg_this_process->number[program_id])
			&& (j < program_id))))
			{}
		}
		
		cout << "---------- OPEN OUTPUT FILE \"" << filename << "\" BY PROCESS ???" << program_id << " BEGIN ----------\n";
		ofstream local_file(filename, ios_base::app); // http://cppstudio.com/post/446/
		cout << "---------- OPEN OUTPUT FILE \"" << filename << "\" BY PROCESS ???" << program_id << " END ----------\n\n";
		
		cout << "---------- WRITE STRING ???" << i << " BY PROCESS ???" << program_id << " BEGIN ----------\n";
		local_file << local_string;
		cout << "---------- WRITE STRING ???" << i << " BY PROCESS ???" << program_id << " END ----------\n\n";
		
		cout << "---------- CLOSE OUTPUT FILE \"" << filename << "\" BY PROCESS ???" << program_id << " BEGIN ----------\n";
		local_file.close();
		cout << "---------- CLOSE OUTPUT FILE \"" << filename << "\" BY PROCESS ???" << program_id << " END ----------\n\n";
		
		// when the process has finished with its critical section execution,
		// it resets its number variable to 0
		shared_mem_seg_this_process->number[program_id] = 0;
	}
	
	// ---------- SHARED MEMORY SEGMENT DETACH FROM THE PROGRAM MEMORY (SEPARATE THEM) ----------
	
	shmdt((void*)shared_mem_seg_this_process);
	/*
	 * https://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/shmdt.html
	 * shmdt(shm_ptr);
	 * 
	 * system call "shmdt" is used to detach a shared memory;
	 * after a shared memory is detached, it cannot be used in process;
	 * but it is still there and can be re-attached back to a adress space of process,
	 * perhaps at a different address;
	 * "shared_mem_seg_this_process" -- argument of the call to "shmdt", the shared memory address returned by "shmat";
	 */
	
	// ---------- CLEANING & TERMINATING ----------
	
	if(shared_mem_seg_owner == true)
	{
		// https://en.cppreference.com/w/cpp/types/NULL
		shmctl(shared_mem_seg_ptr, IPC_RMID, NULL);
		cout << "\n---------- SHARED MEMORY SEGMENT HAS BEEN CLOSED ----------\n";
	}
	/*
	 * https://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/shmdt.html
	 * shmctl(shm_id, IPC_RMID, NULL);
	 * 
	 * to remove a shared memory, use "shmctl" function;
	 * "shared_mem_seg_ptr" is the shared memory ID;
	 * "IPC_RMID" indicates this is a remove operation;
	 * if you want to use it again, you should use "shmget" followed by "shmat";
	 */
	
	return 0;
}

// ---------- FUNCTIONS ----------

// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
bool hasEnding (string const &fullString, string const &ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

// ---------- CONSTRUCTOR ----------

ArgFlagParse::ArgFlagParse()
{
	this->error_existance = false;
	this->flags_number = 0;
	this->flags_array = nullptr;
}

// ---------- CONSTRUCTOR CUSTOM ----------

ArgFlagParse::ArgFlagParse(int local_flags_number, string* local_flags_array)
{
	this->error_existance = false;
	this->flags_number = local_flags_number;
	this->flags_array = local_flags_array;
}

// ---------- DESTRUCTOR ----------

ArgFlagParse::~ArgFlagParse()
{
	//delete [] this->flags_array;
}

// ---------- ArgFlagParsevoid ----------

void ArgFlagParse::ArgvCommonCheck (int local_argc, char* local_argv[])
{
	this->ArgvCommonCheck(local_argc, local_argv, this->flags_array);
}

// ---------- ArgFlagParsevoid ----------

void ArgFlagParse::ArgvCommonCheck (int local_argc, char* local_argv[], string* local_flags)
{
	this->error_existance = false;
	if (local_argc == 1)
	{
		this->error_existance = true;
	}
	else
	{
		int i = 1; // argc/argv number -- 2 by 1 loop
		int j = 0; // flags number -- 1 by 1 loop
		while (i < local_argc && this->error_existance == false)
		{
			//if (strcmp(argv[i], string("-num").c_str()) != 0) // clutch!!!
			if (strcmp(local_argv[i], local_flags[j].c_str()) != 0) // clutch!!!
			{
				this->error_message = "Couldn't find \"" + local_flags[j] + "\" flag. Program terminating!";
				this->error_existance = true;
			}
			else
			{
				if (ArgvConditionCheck (i, local_argv[i + 1]) == false) // if not satisfied
				{
					this->error_message = "\"" + local_flags[j] + "\" parameter doesn't satisfy the condition: " + this->error_message + "Program terminating!";
					this->error_existance = true;
				}
			}
			i = i + 2;
			j = j + 1;
		}
	}
}

// ---------- ArgvConitionCheck ----------

bool ArgFlagParse::ArgvConditionCheck (int local_flag, char* local_argv)
{
	bool local_return = true;
	if (local_flag == 1)
	{
		local_return = ArgvConditionCheckID (local_argv);
	}
	else if (local_flag == 3)
	{
		local_return = ArgvConditionCheckTime (local_argv);
	}
	else if (local_flag == 5)
	{
		local_return = ArgvConditionCheckNum (local_argv);
	}
	else if (local_flag == 7)
	{
		local_return = ArgvConditionCheckFile (local_argv);
	}
	else if (local_flag == 9)
	{
		local_return = ArgvConditionCheckKey (local_argv);
	}
	return local_return;
}

bool ArgFlagParse::ArgvConditionCheckID (char* local_argv) // clutch -- only to be here, need to be redefined or updated
{
	bool local_return = true;
	if (atoi(local_argv) != 0 && atoi(local_argv) != 1 && atoi(local_argv) != 2)
	{
		this->error_message = "parameter must be in range [0; 2]. ";
		local_return = false;
	}
	return local_return;
}

bool ArgFlagParse::ArgvConditionCheckTime (char* local_argv) // clutch -- only to be here, need to be redefined or updated
{
	bool local_return = true;
	if (atoi(local_argv) < -1)
	{
		this->error_message = "parameter must be in range [-1; +inf). ";
		local_return = false;
	}
	return local_return;
}

bool ArgFlagParse::ArgvConditionCheckNum (char* local_argv) // clutch -- only to be here, need to be redefined or updated
{
	bool local_return = true;
	if (atoi(local_argv) < 0)
	{
		this->error_message = "parameter must be in range [0; +inf). ";
		local_return = false;
	}
	return local_return;
}

bool ArgFlagParse::ArgvConditionCheckFile (char* local_argv) // clutch -- only to be here, need to be redefined or updated
{
	bool local_return = true;
	if (local_argv == nullptr)
	{
		this->error_message = "filename hasn't been found. ";
		local_return = false;
	}
	// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
	else if (hasEnding(string(local_argv), ".txt") == false)
	{
		this->error_message = "filename must contain \".txt\" extension. ";
		local_return = false;
	}
	return local_return;
}

bool ArgFlagParse::ArgvConditionCheckKey (char* local_argv) // clutch -- only to be here, need to be redefined or updated
{
	bool local_return = true;
	if (atoi(local_argv) < 0)
	{
		this->error_message = "parameter must be in range [0; +inf). ";
		local_return = false;
	}
	return local_return;
}

// ---------- getErrorExistance ----------

bool ArgFlagParse::getErrorExistance ()
{
	return this->error_existance;
}

// ---------- getErrorMessage ----------

string ArgFlagParse::getErrorMessage ()
{
	return this->error_message;
}
