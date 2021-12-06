Программа
============
Команда `./psearch [keys] pattern [keys] directory [keys]` ищет вхождения шаблона `pattern` в файлы директории `directory` и глубже в несколько потоков.  
`-n` - искать вхождения только в текущей директории.  
`-t#` - искать вхождения в # потоков.  
По умолчанию вхождения ищутся в текущей директории и глубже в один поток.  

Компиляция
============
`g++ psearch.cpp -lpthread -o psearch` - стандартная компиляция  
`g++ -fsanitize=address psearch.cpp -lp_thread -o psearch_fsanitize` - компиляция с -fsanitize=address  
`valgrind ./psearch [keys] pattern [keys] directory [keys]` - запуск под valgrind  

Исполнение
===========
На сравнительно небольшом наборе данных:  
Input: `time ./psearch int /usr/include -t1 -n`  
Output: `real	0m0,361s user	0m0,298s sys	0m0,035s`  

Input: `time ./psearch int /usr/include -t4 -n`  
Output: `real	0m0,266s user	0m0,300s sys	0m0,091s`  

На сравнительно большом наборе данных:
Input: `time ./psearch int /usr/include -t1`  
Output: ``
