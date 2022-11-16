# Лабораторные работы по ОПиПвСL

## Лабораторные работы по Организации процессов и программирования в среде Linux (ОПиПвСL), 7 семестр СПбГЭТУ "ЛЭТИ", 2022

### Лабораторная работа 4

Программу необходимо компилировать с флагом `-pthread`, который отвечает за мультипроцессорное программирование в используемой библиотеке `<pthread.h>`!

Например, `g++ -Wall -pthread -o "%e" "%f"`.

### Лабораторная работа 5

Программу необходимо запускать со следующими флагами:\
`./main (signal | sigaction | <other=default>) (1 | 2 | <other=default>)`, где:
- в части **./main**:
  - `./main` -- название программы (исполняемого файла);
- в части **(signal | sigaction | <other=default>)**:
  - `signal` -- использование функции `signal`;
  - `sigaction` -- использование функции `sigaction`;
  - `<other=default>` -- использование дефолтной функции (`signal`);
- в части **(1 | 2 | <other=default>)**:
  - `1` -- деление на 0 в качестве ошибки;
  - `2` -- обращение по **nullptr** адресу в качестве ошибки;
  - `<other=default>` -- дефолтная операция (деление на 0) в качестве ошибки.

Например, `./main signal 1` будет означать, что будет использоваться функция `signal` и что будет производиться деление на 0.

### Лабораторная работа 6

Компиляция программы стандартная.\
Запуск программы со следующими параметрами:

`./main <launch period> <number of launches>`, где:

- `<launch period>` — время периода между запусками программы в секундах (время между каждым запуском программы);
- `<number of launches>` — количество периодических запусков (сколько раз программа запустится).

### Лабораторная работа 7

Компиляция программы стандартная.\
Запуск программы со следующими параметрами:

`./main <filename.txt>`, где:

- `<filename.txt>` — имя читаемого программой файла в формате **txt**, например, `./main lorem_ipsum.txt`.

Внимание!
При создании файла важно учитывать формат окончания строки, на Unix/Linux это **LF — line feed** (`\n`),
а на Windows это **CRLF — carriage return line feed** (`\r\n`).
Из-за того, что файлы с разными форматами окончания строки будут читаться по-разному,
могут возникнуть ошибки, и программа может работать некорректно.
*Создавайте* файлы *на Linux* или *меняйте формат* окончания строки на *LF*!
Примеры в отчёте приведены для файлов с форматом окончания строки **LF**.

### Лабораторная работа 8

#### Теория

Зачем нужны функции `msgrcv` и `msgsnd`:

- `msgrcv` -- получить сообщение;
- `msgsnd` -- отправить сообщение.

Зачем нужны структуры `MessageRequest` и `MessageResponse`, а также их объекты в программе:

- `MessageRequest` — для посылки/принятия в общей очереди и для хранения всяких данных;
  - `MessageRequest message_request_receive[2]` — для хранения полученных запросов на чтение от других программ (то есть необработанных запросов), сделано 2 ячейки для двух других программ, от которых я принимаю запрос;
  - `MessageRequest message_request_send[4]` — для хранения отправленных запросов на чтение для других программ и для их отправки, сделано 4 ячейки для соответствия 1, 2 и 3 программ (одна из них не понадобится), а 0-я ячейка нужна для проверки завершения очереди;
  - `MessageRequest message_request` — для посылки запроса об окончании работы общей очереди;
- `MessageResponse` — для посылки/принятия в локальной очереди;
  - `MessageResponse message_response` — для отправки разрешения другим программам и для получения разрешения от других программ.

Пример 1:
```
msgsnd(message_request_receive[message_number].local_queue_id, &message_response, sizeof(message_response), 0)
```
В этой строке, оперирующей локальной очередью, есть `message_request_receive` типа `MessageRequest`,
но он служит для извлечения нужной нам локальной очереди
(чтобы разрешение отправить не в общую очередь, где все её могут прочитать, а в локальную, чтобы только эта программа могла её увидеть),
а `&message_response` типа `MessageResponse` служит уже для непосредственной отправки.

Пример 2:
```
msgrcv(common_queue, &message_request_receive[message_number], sizeof(message_request_receive[message_number]), program_id, IPC_NOWAIT) != -1
```
В этой строке, оперирующей общей очередью, идёт принятие из общей очереди,
и вторым параметром стоит уже `&message_request_receive[message_number]` типа `MessageRequest`.

Пример 3:
```
(msgrcv(local_queue, &message_response, sizeof(message_response), 0, IPC_NOWAIT) != -1)
```
В этой строке, оперирующей локальной очередью, `&message_response` типа `MessageResponse` служит для принятия, а не для отправки.

#### Director's cut (режиссёрская версия)

`Режиссёрская версия` — версия программы, созданная для работы,
которая содержит возможность менять значения `номера программы`, читаемого `файла` и `ключа` общей очереди и которая содержит ссылки и дополнительную информацию по коду и теме.

Компиляция программы стандартная.\
Запуск программы происходит в **трёх терминалах параллельно** со следующими параметрами:

`./main -num (1 | 2 | 3) -file <filename.txt> -key (<key_integer_number >= 0>)`, где:

- флаг `-num` — ID запускаемой программы, может быть только одним из следующих значений, при запуске трёх программ у каждой, очевидно, должны быть уникальные значения:
  - `1`;
  - `2`;
  - `3`;
- флаг `-file` — название читаемого программой файла в формате **txt**, расположенного в той же директории, что и исполняемый файл, например, `lorem_ipsum.txt`:
  - `<filename.txt>`;
- флаг `-key` — неотрицательный целочисленный ключ для создания общей очереди (в локальной очереди он всегда равен `IPC_PRIVATE = 0`), необходимый **для подключения к одной общей очереди другими программами** (очевидно, что все 3 запускаемые программы должны иметь **один и тот же ключ**):
  - `<key_integer_number > 0>` — положительный целочисленный ключ, программа будет работать корректно;
  - `<key_integer_number = 0>` — если вы выбираете ключ, равный `0 = IPC_PRIVATE`, то программа не будет работать, так как этот ключ создаёт приватную очередь, которую невозможно опознать другим программам.

Например, для **корректного запуска** трёх программ в **трёх терминалах параллельно** (каждую строку нужно запустить в отдельном терминале) значения будут следующими:

```
./main -num 1 -file lorem_ipsum.txt -key 190
./main -num 2 -file lorem_ipsum.txt -key 190
./main -num 3 -file lorem_ipsum.txt -key 190
```

#### Theatrical cut (театральная версия)

`Театральная версия` — версия программы, соданная для демонстрации,
которая уже содержит конкретные значения `номера программы`, читаемого `файла` и `ключа` общей очереди.

Компиляция программы стандартная.\
Запуск программ происходит в **трёх терминалах параллельно** без параметров:

`./executable_<id_number>`, где `<id_number>` равен `1`, `2` или `3`.

Значение ключа в этой версии для создания общей очереди равно `190`, ID в каждой из программ соответствующее, `1` для `./executable_1`, `2` для `./executable_2` и `3` для `./executable_3`, читаемый файл в каждой из программ `lorem_ipsum.txt`.

Например, для **корректного запуска** трёх программ в **трёх терминалах параллельно** (каждую строку нужно запустить в отдельном терминале) значения будут следующими:

```
./executable_1
./executable_2
./executable_3
```

## Дополнительная информация

### Используемый на Linux компилятор

```
matmanbj@matmanbj-VirtualBox:~$ gcc --version
gcc (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

```
matmanbj@matmanbj-VirtualBox:~$ g++ --version
g++ (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Версия Linux в файле `/etc/os-release`

```
matmanbj@matmanbj-VirtualBox:~$ cat /etc/os-release
NAME="Ubuntu"
VERSION="20.04.5 LTS (Focal Fossa)"
ID=ubuntu
ID_LIKE=debian
PRETTY_NAME="Ubuntu 20.04.5 LTS"
VERSION_ID="20.04"
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
VERSION_CODENAME=focal
UBUNTU_CODENAME=focal
```

Взято [отсюда](https://www.cyberciti.biz/faq/how-to-check-os-version-in-linux-command-line/).

### Использование команды `lsb_release`

```
matmanbj@matmanbj-VirtualBox:~$ lsb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 20.04.5 LTS
Release:	20.04
Codename:	focal
```

Взято [отсюда](https://www.cyberciti.biz/faq/how-to-check-os-version-in-linux-command-line/).

### Использование команды `hostnamectl`

```
matmanbj@matmanbj-VirtualBox:~$ hostnamectl
   Static hostname: matmanbj-VirtualBox
         Icon name: computer-vm
           Chassis: vm
        Machine ID: e5c748f0a6434a2489252bb8c130fcbf
           Boot ID: 189c7cbf53c343488f885a2dc9e7a427
    Virtualization: oracle
  Operating System: Ubuntu 20.04.5 LTS
            Kernel: Linux 5.15.0-50-generic
      Architecture: x86-64
```

Взято [отсюда](https://www.cyberciti.biz/faq/how-to-check-os-version-in-linux-command-line/).

### Использование команды `uname` с флагом `-r`

```
matmanbj@matmanbj-VirtualBox:~$ uname -r
5.15.0-50-generic
```

Взято [отсюда](https://www.cyberciti.biz/faq/how-to-check-os-version-in-linux-command-line/).

# Лицензия

Этот проект находится под лицензией MIT. Дополнительная информация в файле `license.txt`.