#include <stdio.h>
#include <string>
#include <vector>
#include <iostream> 
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

//начало и конец алфавита
#define ALPHA_START 32
#define ALPHA_END 125

class KMP
{
	private:
		std::vector<std::string> prefixes = {""}; //состояния КМП-автомата
		std::vector<char> transitions; //переходы между состояниями
		std::vector<int> links = {0}; //ссылки-исключения
		int length;
	public:
		std::vector<std::string> paths; //директории, в которых ищется вхождение шаблона
	        pthread_mutex_t mutex;
		
		int recursive_link_detector(int number, char symbol) //вычисление суффиксных ссылок (ссылок-исключений)
		{
			if (links[number] == 0 && (transitions[0] != symbol || number == 0)) return 0;
			if (number > 0 && transitions[links[number]] == symbol) return links[number] + 1;
			else return recursive_link_detector(links[number], symbol);
		}
		KMP()
		{
			length = 0;
		}
                ~KMP()
                {
                        prefixes.clear();
                        transitions.clear();
                        links.clear();
                }
		void get_KMP(std::string pattern) //построение автомата по шаблону
		{
			length = pattern.size();
			std::string tmp;
			for (int iter = 0; iter < length; ++iter)
			{
				char symbol = pattern.at(iter);
				tmp.push_back(symbol);
				prefixes.push_back(tmp);
				transitions.push_back(symbol);
				links.push_back(recursive_link_detector(iter, symbol));
			}
		}
		bool is_in_string(std::string element, int pos_in_element, int pos_in_kmp) //поиск вхождения в строку
		{
			while (pos_in_element < element.size())
			{
				if (pos_in_kmp == length) return true;
				if (element.at(pos_in_element) < ALPHA_START || element.at(pos_in_element) > ALPHA_END) break;
				if (element.at(pos_in_element) == transitions.at(pos_in_kmp)) //если определен переход по символу
				{
					++pos_in_element;
					++pos_in_kmp;
					if (pos_in_kmp == length) return true;
					continue;
				}
				else //если переход не определен
				{
					if (pos_in_kmp == 0)
					{
						++pos_in_element;
						continue;
					}
					pos_in_kmp = links.at(pos_in_kmp);
					continue;
				}
			}
			return false;
		}

		void is_in_text(std::vector<std::string> text, std::string directory) //поиск вхождений шаблона в текст
		{
			bool is_almost_one = false;
			int iter = 0;
			for (iter; iter < text.size(); ++iter)
			{
				if (is_in_string(text.at(iter), 0, 0)) //поиск первого вхождения, чтобы вывести имя директории
				{
					is_almost_one = true;
					pthread_mutex_lock(&mutex);
					printf("%s:\n", directory.c_str());
					printf("%d: %s\n", iter+1, text.at(iter).c_str());
					++iter;
					pthread_mutex_unlock(&mutex);
					break;
				}
			}
			for (iter; iter < text.size(); ++iter) //поиск оставшихся вхождений
			{	
				pthread_mutex_lock(&mutex);
				if (is_in_string(text.at(iter), 0, 0)) printf("%d: %s\n", iter+1, text.at(iter).c_str());
				pthread_mutex_unlock(&mutex);
			}
			pthread_mutex_lock(&mutex);
			if (is_almost_one)
			{
				printf("\n");
			}
			pthread_mutex_unlock(&mutex);
		}

		void check_file(std::string directory) //заполнение текста строками из директории
		{
	        	FILE* file = fopen(directory.c_str(), "r");
        		if (!file) //обработка ошибок
        		{
				pthread_mutex_lock(&mutex);
                		fprintf(stderr, "can't open file: %s\n", directory.c_str());
				pthread_mutex_unlock(&mutex);
                		return;
        		}

			char* buffer = NULL;
        		size_t buf_size = 0;
        		int string_count = 1;
        		ssize_t string_size;
			string_size = getline(&buffer, &buf_size, file);
			int flag1 = 0;
			int flag2 = 0;
			while(string_size >= 0)
			{
				std::string text;
				for (int iter = 0; iter < buf_size; ++iter) text.push_back(buffer[iter]);
            			flag1 = is_in_string(text, 0, 0);
            			if (flag1 && !flag2)
				{
                			pthread_mutex_lock(&mutex);
					printf("%s\n", directory.c_str());
                			printf("%d: %s", string_count, buffer);
					flag2 = 1;
                			pthread_mutex_unlock(&mutex);
            			}
				else if (flag1 && flag2)
				{
					pthread_mutex_lock(&mutex);
					printf("%d: %s", string_count, buffer);
					pthread_mutex_unlock(&mutex);
				}
            			++string_count;
            			string_size = getline(&buffer, &buf_size, file);
        		}
			if (flag2) printf("\n");
        		free(buffer);
        		buffer = NULL;
        		fclose(file);
		}

		void check_thread() //нужно для file_check()
		{
			for (long long int iter = 0; iter < paths.size(); ++iter) check_file(paths.at(iter));
		}
};

void* file_check(void* arg)
{
	KMP* args = (KMP*)arg;
	args->check_thread();
	return NULL;
}
std::string get_directory() //вычисление текущей директории
{
	size_t buf_size = 20;
	std::string directory;
	int iter = 0;
	do
	{
		++iter;
		directory.clear();
		errno = 0;
		char buffer[buf_size*iter];
		getcwd(buffer, buf_size*iter);
		for (int i = 0; i < buf_size*iter; ++i) directory.push_back(buffer[i]);
	}while(errno == ERANGE);
	return directory;
}
void walk_recursive(std::string const &dirname, std::vector<std::string> &ret) //рекурсивный обход директории (если во вводе есть -n)
{
	DIR *dir = opendir(dirname.c_str());
	if (dir == nullptr) //обработка ошибок 
	{
        	perror(dirname.c_str());
        	return;
    	}
    	for (dirent *de = readdir(dir); de != NULL; de = readdir(dir)) {
        	if (strcmp(".",de->d_name) == 0 || strcmp("..", de->d_name) == 0) continue;
        	if (de->d_type != DT_DIR) ret.push_back(dirname + "/" + de->d_name); //если указатель на файл, добавляем директорию в вектор
		else if (de->d_type == DT_DIR)
			walk_recursive(dirname + "/" + de->d_name, ret);
   	 }
   	 closedir(dir);
}
void walk_non_recursive(std::string const& dirname, std::vector<std::string>& ret) //нерекурсивный обход директории (если во вводе нет -n)
{
        DIR *dir = opendir(dirname.c_str());
        for (dirent *de = readdir(dir); de != NULL; de = readdir(dir)) {
		if (de->d_type == DT_DIR) continue;
                if (strcmp(".",de->d_name) == 0 || strcmp("..", de->d_name) == 0) continue;
		const char* tmp = (dirname + "/" + de->d_name).c_str();
                ret.push_back(dirname + "/" + de->d_name);
         }
         closedir(dir);
}

int main(int argc, char* argv[])
{
	bool only_current_dir = false;
	int threads_num = 1;
	std::string pattern, directory; //шаблон, текущая директория
	std::vector<std::string> ret; //вектор директорий, в которых осуществляется поиск вхождений
	for (int iter = 1; iter < argc; ++iter) //парсинг ввода
	{
		if (std::string(argv[iter]) == "-n") only_current_dir = true;
		else if (argv[iter][0] == '-' && argv[iter][1] == 't') threads_num = (int)(argv[iter][2] - 48);
		else if (pattern.empty()) pattern = std::string(argv[iter]);
		else directory = std::string(argv[iter]);
	}
	if (directory.empty()) directory = get_directory();
	const char* dirname = directory.c_str();
	only_current_dir ? walk_non_recursive(dirname, ret) : walk_recursive(dirname, ret);
	printf("\n");
	std::sort(ret.begin(), ret.end(), [](std::string &s1, std::string &s2){return s1.length() > s2.length();});

	/*
	 * директории распределяются по объектам класса KMP,
	 * для каждого объекта создается поток
	 */

	std::vector<KMP> args;
	args.resize(threads_num);
	for (long long int iter = 0; iter < threads_num; ++iter)
	{
		if (iter >= ret.size()) break;
		args[iter].get_KMP(pattern);
		args[iter].paths.push_back(ret.at(iter));
	}
	for (long long int iter = threads_num; iter < ret.size();)
	{
		for (int jter = 0; jter < threads_num; ++jter)
		{
			if (iter >= ret.size()) break;
			args[jter].paths.push_back(ret.at(iter));
			++iter;
		}
	}
	pthread_t *threads = (pthread_t *) malloc(threads_num * sizeof(pthread_t));
    	pthread_mutex_t mutex_main;
	pthread_mutex_init(&mutex_main, NULL);
	for(long long int iter = 0; iter < threads_num; ++iter)
	{
        	args[iter].mutex = mutex_main;
        	pthread_create(threads+iter, NULL, file_check, &args[iter]);
    	}
	for(long long int iter = 0; iter < threads_num; ++iter)
	{
        	pthread_join(threads[iter], NULL);
    	}
    	pthread_mutex_destroy(&mutex_main);
    	free(threads);
	return 0;
}

