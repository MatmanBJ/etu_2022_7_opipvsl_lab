#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

void *ChildThread(void *arg)
{
	int local_handle_check; // handle check local
    int schedPolicy; // schedualing policy field
    int local_close = 0; // 0 -- child thread DOES NOT CLOSES the handle, 1 -- chiled thread CLOSES the handle, other -- DOES NOT CLOSE by default
	int local_buffer_counter = -1; // number of bytes read (-1 is for begin, 0 is for ending loop)
    struct sched_param schedParam; // struct for schedual priority
    
    cout << "---------- CHILD THREAD BEGIN ----------\n";
    
    cout << "Close the handle by child process (0 -- don't close, 1 -- close, other -- don't close)?\n"; // close handle by child thread or not
    cin >> local_close; // user's choice

    pthread_getschedparam(pthread_self(), &schedPolicy, &schedParam); // getting parameters about CHILD thread
    
    int handle = *((int*)arg);
    
    // ---------- SCHEDUALING POLICY, CURRENT, MIN & MAX PRIORITY, FILE OUTPUT ----------
    
    cout << "---------- CHILD THREAD INFO ----------\n"
    << "Schedualing policy: " // schedualing policy
    << (schedPolicy == SCHED_FIFO ? to_string(schedPolicy) + " -- SCHED_FIFO" : "")
    << (schedPolicy == SCHED_RR ? to_string(schedPolicy) + " -- SCHED_RR" : "")
    << (schedPolicy == SCHED_OTHER ? to_string(schedPolicy) + " -- SCHED_OTHER" : "") << "\n"
    << "Current priority: " << schedParam.sched_priority << "\n" // current priority
    << "Minimal priority: " << sched_get_priority_min(schedPolicy) << "\n" // minimal priority
    << "Maximal priority: " << sched_get_priority_max(schedPolicy) << "\n" // maximal priority
    << "File:\n" // file output
    << "---------- BEGIN OF FILE ----------\n";
    //<< local_buffer << "\n"
	while(local_buffer_counter != 0) // 0 means no bytes to read
	{
		char local_buffer[80];
		size_t n = sizeof(local_buffer);
		local_buffer_counter = read(handle, &local_buffer, n);
		local_buffer[local_buffer_counter] = '\0';
		cout << local_buffer;
	}
	cout << "\n";
    cout << "---------- END OF FILE ----------\n";
    
    if (local_close == 0) // handle close chosen actions
    {
		cout << "File's handle SHOULD NOT BE closed by CHILD thread!\n";
	}
	else if (local_close == 1)
	{
		cout << "File's handle SHOULD BE closed by CHILD thread!\n";
		close(handle);
	}
	else
	{
		cout << "File's handle SHOULD NOT BE closed by CHILD thread by default!\n";
	}
    
    local_handle_check = fcntl(handle, F_GETFD);
    
    if (local_handle_check == -1) // handle close try check
    {
		cout << "File's handle HAS BEEN closed by CHILD thread!\n";
    }
    else
    {
		cout << "File's handle HAS NOT BEEN closed by CHILD thread!\n";
    }

    cout << "---------- CHILD THREAD END ----------\n";
    pthread_exit(NULL); // terminating calling thread, i.e. this CHILD process will be terminated
}

int main()
{
	int handle_check; // handle check
    int handle; // handle
    int schedPolicy; // schedualing policy field MAIN THREAD
    struct sched_param schedParam; // struct for schedual priority MAIN THREAD
    pthread_t thread;
    pthread_attr_t attr;
    
    cout << "---------- MAIN THREAD BEGIN ----------\n";
    
    // ---------- OPENING TEXT FILE ----------
    
    handle = open("/home/matmanbj/lorem.txt", O_RDONLY);

    if (handle == -1)
    {
        cout << "File's handle HAS NOT BEEN opened by PARENT thread!\n";
        return -1; // end program with error
    }
    else
    {
		cout << "File's handle HAS BEEN opened by PARENT thread! Handle: " << handle << "\n";
    }
    
    // ---------- CREATING A THREAD (SENDING HANDLE AS A PARAMETER) ----------

    pthread_attr_init(&attr);

    pthread_create(&thread, &attr, ChildThread, &handle);
    pthread_join(thread, nullptr);
    
    // ---------- CURRENT PRIORITY OUTPUT ----------
    
    cout << "---------- MAIN THREAD INFO ----------\n";
    pthread_getschedparam(pthread_self(), &schedPolicy, &schedParam); // getting parameters about MAIN thread
    cout << "Current priority: " << schedParam.sched_priority << "\n"; // current priority
    
    // ---------- CHECKING ON OPEN/CLOSED FILE (IF OPENED -- FORCIBLY CLOSE) ----------
    
    handle_check = fcntl(handle, F_GETFD); // checking handle
    
    if (handle_check != -1)
    {
		cout << "File's handle HAS NOT BEEN closed by CHILD thread!\n";
		close (handle); // forced closing the file (handle)
		handle_check = fcntl(handle, F_GETFD); // checking handle again
		
		if (handle_check != -1)
		{
			cout << "File's handle HAS NOT BEEN closed by PARENT thread!\n";
			return -2; // end program with error
		}
		else
		{
			cout << "File's handle HAS BEEN closed by PARENT thread!\n";
		}
    }
	else
	{
		cout << "File's handle HAS BEEN closed by CHILD thread!\n";
    }
    
    cout << "---------- MAIN THREAD END ----------\n";

    pthread_attr_destroy(&attr);
    return 0;
}
