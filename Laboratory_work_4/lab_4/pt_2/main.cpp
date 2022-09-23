// !!! compile w/ flag "-pthread" !!!
// i.e. "g++ -Wall -pthread -o "%e" "%f""

#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace std;

typedef struct // struct to give arguments (string & write file adress) to function
{
	string local_string;
	ofstream *write_file;
} funcArg;

void isFileOpenMyFunc (ifstream *local_file, const string local_file_name);
void isFileOpenMyFunc (ofstream *local_file, const string local_file_name);
void* threadFunction (void* main_argument);

// ---------- MAIN ----------

int main(int argc, char *argv[])
{
	pthread_t thread_1; // 1st thread to write odd strings (1, 3, 5...)
	pthread_t thread_2; // 2nd thread to write even strings (2, 4, 6...)
	bool is_file_end = false; // file read end indicator
	funcArg main_arguments_1;
	funcArg main_arguments_2;
	
	cout << "---------- MAIN THREAD BEGIN ----------\n";
	
	cout << "---------- FILE OPEN & CHECK ----------\n";
	
	ifstream input_file("/home/matmanbj/lorem_ipsum_2.txt"); // open file to read
	ofstream output_file_1("output_file_1.txt"); // open file to write 1
	ofstream output_file_2("output_file_2.txt"); // open file to write 2
	
	main_arguments_1.write_file = &output_file_1; // for struct
	main_arguments_2.write_file = &output_file_2; // for struct
	
	isFileOpenMyFunc (&input_file, "/home/matmanbj/out0.txt"); // check opening file to read
	isFileOpenMyFunc (&output_file_1, "out1.txt"); // check opening file to write 1
	isFileOpenMyFunc (&output_file_2, "out2.txt"); // check opening file to write 2
	
	cout << "---------- READ & WRITE BEGINS ----------\n";
	
	while (is_file_end == false) // while file's end didn't reached, do R/W
	{
		if(getline(input_file, main_arguments_1.local_string)) // if we can read 2n+1_th string, then put characters into string
		{
			pthread_create(&thread_1, NULL, threadFunction, (void*)&main_arguments_1); // create thread to write this string to file
		}
		else
		{
			is_file_end = true; // else indicate the loop to stop
		}
		
		if(getline(input_file, main_arguments_2.local_string)) // if we can read 2n_th string, then put characters into string
		{
			pthread_create(&thread_2, NULL, threadFunction, (void*)&main_arguments_2); // create thread to write this string to file
		}
		else
		{
			is_file_end = true; // else indicate the loop to stop
		}
		
		pthread_join(thread_1,NULL); // waiting for parallel threads termination
		pthread_join(thread_2,NULL); // waiting for parallel threads termination
	}
	
	cout << "---------- READ & WRITE ENDS ----------\n";
	
	// close all opened files
	output_file_1.close();
	output_file_2.close();
	input_file.close();
	
	cout << "---------- MAIN THREAD END ----------\n";
	
	return 0;
}

// ---------- isFileOpen function ----------

void isFileOpenMyFunc (ifstream *local_file, const string local_file_name)  // check if the file open
{
	if ((*local_file).is_open()) //if (*local_file)
	{
		cout << "File to READ \"" << local_file_name << "\" HAS BEEN opened by PARENT thread!\n";
	}
	else // if the file isn't opened, throw an error
	{
		cout << "File to READ \"" << local_file_name << "\" HAS NOT BEEN opened by PARENT thread!\n"
		<< "The program terminates w/ error!\n";
		puts(("File to READ \"" + local_file_name + "\" HAS NOT BEEN opened by PARENT thread. Continuation is impossible!").c_str());
	}
}

void isFileOpenMyFunc (ofstream *local_file, const string local_file_name) // check if the file open
{
	if ((*local_file).is_open()) //if (*local_file)
	{
		cout << "File to WRITE \"" << local_file_name << "\" HAS BEEN opened by PARENT thread!\n";
	}
	else // if the file isn't opened, throw an error
	{
		cout << "File to WRITE \"" << local_file_name << "\" HAS NOT BEEN opened by PARENT thread!\n"
		<< "The program terminates w/ error!\n";
		puts(("File to WRITE \"" + local_file_name + "\" HAS NOT BEEN opened by PARENT thread. Continuation is impossible!").c_str());
	}
}

// ---------- threadFunction function ----------

void* threadFunction(void* main_argument) // write to file
{
	funcArg* local_argument = (funcArg*) main_argument;
	*(local_argument->write_file) << local_argument->local_string << "\n";
	pthread_exit(NULL);
}
