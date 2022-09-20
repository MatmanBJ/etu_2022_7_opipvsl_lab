#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

//pthread_attr_t *attr;

void *threadFunction(void *arg)
{
	int local_handle_check; // handle check local
    int schedPolicy;
    struct sched_param planParam;
    pthread_getschedparam(pthread_self(), &schedPolicy, &planParam);
	
    cout << "Thread starts working!\n\n";
    
    
    
    cout << "Thread attributes: " << endl;

   // pthread_attr_getschedpolicy(&attr, &schedPolicy);



    switch(schedPolicy)
    {
        case SCHED_FIFO:
            cout << "Sched policy: SCHED_FIFO" << endl;
            break;
        case SCHED_RR:
            cout << "Sched policy: SCHED_RR" << endl;
            break;
        case SCHED_OTHER:
            cout << "Sched policy: SCHED_OTHER" << endl;
            break;
    }
    
    int handle = *((int*)arg);

    char buffer[1024];
    size_t count = sizeof(buffer);

    int bytesRead = read(handle, &buffer, count);
    buffer[bytesRead] = '\0';
    
    // ---------- SCHEDUALING POLICY, CURRENT, MIN & MAX PRIORITY, FILE OUTPUT ----------
    
    cout << "Schedualing policy: " << schedPolicy << "\n" // schedualing policy
    << "Current priority: " << planParam.sched_priority << "\n" // current priority
    << "Minimal priority: " << sched_get_priority_min(schedPolicy) << "\n" // minimal priority
    << "Maximal priority: " << sched_get_priority_max(schedPolicy) << "\n" // maximal priority
    << "File:\n"
    << "---------- BEGIN OF FILE ----------\n"
    << buffer << "\n"
    << "---------- END OF FILE ----------";

    //close(handle);
    
    local_handle_check = fcntl(handle, F_GETFD);
    
    if (local_handle_check == -1)
    {
        cout << "\nThread successfully closed the file!" << endl;
    }
    else
    {
        cout << "\nThe thread didn't close the file." << endl;
    }

    cout << "\nThread ended its work!\n";
    pthread_exit(0);
}

int main()
{
	int handle_check; // handle check
    int handle; // handle
    pthread_t thread;
    pthread_attr_t attr;
    
    // ---------- OPENING TEXT FILE ----------
    
    handle = open("/home/matmanbj/Рабочий стол/lab_4_examples/4_prokopenko/out0.txt", O_RDONLY);

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

    pthread_create(&thread, &attr, threadFunction, &handle);
    pthread_join(thread, nullptr);
    
    // ---------- CURRENT PRIORITY OUTPUT ----------
    
    // need to write check here
    
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

    pthread_attr_destroy(&attr);
    return 0;
}
