// ./writer <циклы> <время>
// на всякий случай: ipcrm -s sem_id

#include <iostream>
#include <fstream>
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include "sem_func.h" // файл с функциями с семафорами

using namespace std;

int main (int argc, char* argv[]) // 0 -- сама программа, 1 -- количество строк, 2 -- время задержки
{
	int sem_id; // ID семафора, возвращаемый при создании/открытии
	ofstream file; // файл для записи
	
	cout << "Процесс id=" << getpid() << "\n" << "Имя файла " << file_name << "\n" << "Ключ семафора " << sem_key << "\n";
	
	// semget(ключ семафора, количество семафоров в наборе (4 штуки), флаги для создания семафора и с правом доступа у всех)
	sem_id = semget(sem_key, 4, IPC_CREAT | IPC_EXCL | 0666); // создаём множественный семафор
	
	if (sem_id != -1)
	{
		cout << "Семафор id=" << sem_id << " создан процессом id=" << getpid() << "\n";
		// важный шаг: при создании семафора (и в reader, и во writer) увеличиваем семафор файла на 1 (то есть максимум -- это 1)
		// этот семафор нужен только для писателей, он нужен для того,
		// чтобы отслеживать возможность записи в файл для писателей
		// при входе в цикл делаем -1, и он =0, поэтому остальные не могут сделать -1
		// так как не стоит флага IPC_NOWAIT, который сразу возвращает ошибку,
		// и остальные программы вынуждены ждать, пока семафор будет =1,
		// чтобы снова сделать -1 (пока =0 они ждут, так как <0 быть не может)
		semop(sem_id, &sem_file_in, 1);
	}	
	else
	{
		sem_id = semget(sem_key, 4, IPC_CREAT); // открываем семафор
		if (sem_id != -1)
		{
			cout << "Семафор id=" << sem_id << " открыт процессом id=" << getpid() << "\n";
		}
		else
		{
			cout << "Семафор не был открыт процессом id=" << getpid() << "\n";
			exit(-1);
		}
	}
	
	// увеличиваем количество активных процессов, чтобы в конце удаляла последняя программа
	semop(sem_id, &sem_proc_in, 1);
	
	cout << "Текущее количество работающих программ " << semctl(sem_id, 3, GETVAL, 0) << "\n\n";
	
	for (int i = 0; i < atoi(argv[1]); i++) // начинаем запись в файл
	{
		cout << "Итерация на запись writer'ом строки " << i + 1 << " началась\n";
		
		cout << "Увеличиваем число активных writer'ов в семафоре процессом id=" << getpid() << "\n";
		semop(sem_id, &sem_writer_in, 1);
		
		cout << "Ждём окончания всех активных reader'ов в семафоре процессом id=" << getpid() << "\n";
		semop(sem_id, &sem_reader_com, 1);
		
		cout << "Уменьшаем возможность доступа к файлу в семафоре процессом id=" << getpid() << " и ЖДЁМ остальных\n";
		// [смотреть пункт, когда создаётся семафор, я там расписал]
		semop(sem_id, &sem_file_dec, 1); // уменьшаем семафор файла, чтобы другие не могли получить доступ к файлу
		
		cout << "Открываем файл на запись \"" << file_name << "\" процессом id=" << getpid() << "\n";
		file.open(file_name, ios::app);
		
		cout << "Записываем строку " << i + 1 << " процессом id=" << getpid() << "\n" << "Процесс id=" << getpid() << ", строка " << i + 1 << "\n";
		file << "Процесс id=" << getpid() << ", строка " << i + 1 << "\n";
		
		cout << "Закрываем файл на запись \"" << file_name << "\" процессом id=" << getpid() << "\n";
		file.close();
		
		cout << "Увеличиваем возможность доступа к файлу в семафоре процессом id=" << getpid() << " (возвращаем как было)\n";
		// [смотреть пункт, когда создаётся семафор, я там расписал]
		semop(sem_id, &sem_file_in, 1); // увеличиваем семафор файла, чтобы другие снова могли получить доступ к файлу (возвращаем всё, как было)
		
		cout << "Уменьшаем число активных writer'ов в семафоре процессом id=" << getpid() << "\n";
		semop(sem_id, &sem_writer_dec, 1);
		
		cout << "Итерация на запись writer'ом строки " << i + 1 << " закончилась\n\n";
		
		sleep(atoi(argv[2])); // ждём указанное в качестве аргумента время
	}
	
	// уменьшаю количество активных процессов в семафоре на 1
	// в итоге у самой последней программы после этого шага будет 0, и она всё удалит
	semop(sem_id, &sem_proc_dec, 1);
	
	// сравниваю количество процессов (если=0, то значит захожу, если нет, то пропускаю)
	if (semctl(sem_id, 3, GETVAL, 0) == 0)
	{
		semctl(sem_id, IPC_RMID, 0); // удаляю семафор последней программой (последний процесс удаляет все 4 семафора)
		cout << "Семафор id=" << sem_id << " удалён\n\n";
	}
	
	return 0;
}