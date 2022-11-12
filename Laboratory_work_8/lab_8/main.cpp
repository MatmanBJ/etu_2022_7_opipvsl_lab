// start program
// ./main -num (1 | 2 | 3) -file <filename.txt>
// flag "-num" -- number of program id, it can be only 1, 2 or 3
// flag "-file" -- filename with .txt extension, e.g. filename.txt
// .txt extension is not required by program, only some name,
// but the porgram will read txt files, not the other ones

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <cstring>

using namespace std;

void sendingRequest (struct msg_request *local_buffer, int local_other_program_id, int local_program_id, int local_local_queue, int local_common_queue);

struct msg_request
{
	long mtype; //номер программы-получателя
	int sender;	//номер программы-отправителя
	int local_qid; //идентификатор локальной очереди
	time_t request_time; //время посылки запроса
};

struct msg_response
{
	int sender; //номер программы
	time_t response_time; //время ответа программы, получившей запрос
};

int main(int argc, char *argv[])
{
	bool creator;
	int local_queue; 
	int common_queue;
	int perm_get = 0;
	int perm_send = 0;
	int readers_end = 0;
	int other_first_program_id = 0;
	int other_second_program_id = 0;
	struct msg_response buffer;
	int index;
	struct msg_request msg_req_buf[2];
	struct msg_request req;
	struct msg_request buf[4];
	int prog_id = 0;
	
	// ---------- FLAG CHECK ----------
	
	if (strcmp(argv[1], string("-num").c_str()) != 0) // clutch!!!
	{
		cout << "Couldn't find \"-num\" flag. Program terminating!\n";
		exit(-1);
	}
	else
	{
		if (atoi(argv[2]) != 1 && atoi(argv[2]) != 2 && atoi(argv[2]) != 3)
		{
			cout << "\"-num\" parameter doesn't equal to 1, 2 or 3. Program terminating!\n";
			exit(-1);
		}
	}
	
	if (strcmp(argv[3], string("-file").c_str()) != 0) // clutch!!!
	{
		cout << "Couldn't find \"-file\" flag. Program terminating!\n";
		exit(-1);
	}
	else
	{
		if (argv[4] == nullptr)
		{
			cout << "\"-file\" parameter doesn't contain filename. Program terminating!\n";
			exit(-1);
		}
	}
	
	prog_id = atoi(argv[2]);
	cout << "---------- PROGRAM NUMBER " << prog_id << " ----------\n";
	
	// ---------- CREATING/OPENING COMMON QUEUE ----------
	
	// IPC_CREAT -- if there wasn't queue, it will be created
	// O_EXCL + IPC_CREAT -- if there was queue, msgget will return error
	common_queue = msgget(200, 0606 | IPC_CREAT | IPC_EXCL); // trying to create common queue
	
	// checking if common queue has been created
	if (common_queue != -1) // if we created common queue, write message
	{
		creator = true;
		cout << "---------- COMMON QUEUE HAS BEEN CREATED ----------\n";
	}
	else // if we hasn't been created common queue, try to open, write message
	{
		common_queue = msgget(200, IPC_CREAT); // trying to open common queue
		if (common_queue == -1) // if we couldn't open, write message & terminate program
		{
			cout << "---------- COMMON QUEUE HAS NOT BEEN OPENED ----------\n";
			exit(-1);
		}
		else // if we can open, write message
		{
			cout << "---------- COMMON QUEUE HAS BEEN OPENED ----------\n";
		}
	}
	
	// ---------- CREATING LOCAL QUEUE ----------
	
	local_queue = msgget(IPC_PRIVATE, 0606 | IPC_CREAT); // creating local queue
	
	// checking if local queue has been created or not
	if (local_queue == -1) // if not created -- delete remaining object if it has been created & print message
	{
		cout << "---------- LOCAL QUEUE HAS NOT BEEN CREATED ----------\n\n";
		if (creator) // deleting local queue if there is remaining object
		{
			msgctl(common_queue, IPC_RMID, NULL);
		}
		exit(-1); // terminate program
	}
	else // if created -- pring message
	{
		cout << "---------- LOCAL QUEUE HAS BEEN CREATED ----------\n\n";
	}
	
	// ---------- SENDING REQUESTS FOR ABILITY TO READ TO OTHER PROGRAMS ----------
	
	other_first_program_id = (prog_id)%3 + 1;
	other_second_program_id = (prog_id+1)%3 + 1;
	
	sendingRequest (buf, other_first_program_id, prog_id, local_queue, common_queue);
	sendingRequest (buf, other_second_program_id, prog_id, local_queue, common_queue);
	
	// ---------- GETTING REQUESTS FOR ABILITY AND ABILITIES TO READ FROM OTHER PROGRAMS ----------
	
	while(perm_get < 2) // when we have not got abilities to read from 2 other programs
	{
		if(msgrcv(common_queue, &msg_req_buf[index], sizeof(msg_req_buf[index]), prog_id, IPC_NOWAIT) != -1) // common queue message check
		{
			cout << "Request to read has been got from: " << msg_req_buf[index].sender << "\n";
			cout << "Request to read has been send at: " << ctime(&msg_req_buf[index].request_time) << "\n";
			
			// if the request TIME for ability to read from OTHER program <= request TIME for ability to read from THIS program,
			// THIS program sends the ability to read to OTHER program
			if((msg_req_buf[index].request_time < buf[msg_req_buf[index].sender].request_time)
			  || (msg_req_buf[index].request_time == buf[msg_req_buf[index].sender].request_time 
			  && msg_req_buf[index].sender < prog_id))
			{
				buffer.sender = prog_id;
				buffer.response_time = time(NULL);
				msgsnd(msg_req_buf[index].local_qid, &buffer, sizeof(buffer), 0);
				perm_send = perm_send + 1;
				cout << "Отправка разрешения на чтение для " << msg_req_buf[index].sender << "\n\n";
			}
			else // else untreated request will be placed to "msg_req_buf" array
			{
				index = index + 1;
			}
		}
		// check messages in local queue for abilities to read from other programs
		if(msgrcv(local_queue, &buffer, sizeof(buffer), 0, IPC_NOWAIT) != -1)
		{
			perm_get = perm_get + 1;
			cout << "Ability to read has been got from: " << buffer.sender << "\n";
			cout << "Ability to read has been send at: " << ctime(&buffer.response_time) << "\n";
		}
	}
	
	// ---------- OPENING AND READING THE FILE ----------
	
	cout << "\n---------- OPEN FILE BEGIN ----------\n\n";
	fstream in("input.txt");
	string line;
	cout << "\n---------- OPEN FILE END ----------\n\n";
	
	cout << "\n---------- READ FILE BEGIN ----------\n\n";
	while(!in.eof() && getline(in, line))
	{
		cout << line << "\n";
	}
	cout << "\n---------- READ FILE END ----------\n\n\n";
	in.close();
	
	// ---------- REQUESTS TREATMENT ----------
	
	while(index > 0) // all requests treatment, if they wasn't treated before
	{
		buffer.sender = prog_id;
		buffer.response_time = time(NULL);
		msgsnd(msg_req_buf[index - 1].local_qid, &buffer, sizeof(buffer), 0);
		perm_send++;
		cout << "Sending ability to read for " << msg_req_buf[index - 1].sender << "\n";
		index = index - 1;
	}
	
	while(perm_send < 2) // if other program sended request before checking common queue
	{
		if(msgrcv(common_queue, &msg_req_buf[0], sizeof(msg_req_buf[0]), prog_id, IPC_NOWAIT) != -1) // checking messages from common queue
		{
			buffer.sender = prog_id;
			buffer.response_time = time(NULL);
			msgsnd(msg_req_buf[0].local_qid, &buffer, sizeof(buffer), 0);
			perm_send++;
			cout << "Sending ability to read for " << msg_req_buf[0].sender << "\n";
		}
	}
	
	req.mtype = 4; // message type -- 4
	req.request_time = time(NULL);
	req.local_qid = local_queue;
	req.sender = prog_id;
	msgsnd(common_queue, &req, sizeof(req), 0); // sending ready signal to delete common queue
	
	// ---------- CLEANING & TERMINATING ----------
	
	if(creator) // waiting till other processes will finish
	{
		while(readers_end < 3)
		{
	 		if(msgrcv(common_queue, &buf[0], sizeof(buf[0]), 4, 0) != -1)
			{
				readers_end = readers_end + 1;
			}
	 	}
		msgctl(common_queue, IPC_RMID, 0); // deleting common queue
	}
	
	msgctl(local_queue, IPC_RMID, 0); // deleting local queue
	return 0;
}

void sendingRequest (struct msg_request *local_buffer, int local_other_program_id, int local_program_id, int local_local_queue, int local_common_queue)
{
	local_buffer[local_other_program_id].mtype = local_other_program_id;
	local_buffer[local_other_program_id].request_time = time(NULL);
	local_buffer[local_other_program_id].local_qid = local_local_queue;
	local_buffer[local_other_program_id].sender = local_program_id;
	msgsnd(local_common_queue, &local_buffer[local_other_program_id], sizeof(local_buffer[local_other_program_id]), 0);
	cout << "Request to read has been send to: " << local_buffer[local_other_program_id].mtype << "\n";
	cout << "Request to read has been send at: " << ctime(&local_buffer[local_other_program_id].request_time) << "\n\n";
}
