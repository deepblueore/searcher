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
