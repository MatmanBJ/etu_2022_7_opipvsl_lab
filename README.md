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

# Лицензия

Этот проект находится под лицензией MIT. Дополнительная информация в файле `license.txt`.