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

class KMP
{
	private:
		std::vector<std::string> prefixes = {""};
		std::vector<char> transitions;
		std::vector<int> links = {0};
		int length;
	public:
		std::vector<std::string> paths;
	        off_t files_size;
	        pthread_mutex_t mutex;
		
		int recursive_link_detector(int number, char symbol)
		{
			if (links[number] == 0 && (transitions[0] != symbol || number == 0)) return 0;
			if (number > 0 && transitions[links[number]] == symbol) return links[number] + 1;
			else return recursive_link_detector(links[number], symbol);
		}
		KMP()
		{
			length = 0;
		}
		void get_KMP(std::string pattern)
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
		~KMP()
		{
			prefixes.clear();
			transitions.clear();
			links.clear();
		}
		void print_KMP()
                {
                        for (int i = 0; i < prefixes.size(); ++i) printf("%s ", prefixes.at(i).c_str());
                        printf("\n");
                        for (int i  = 0; i < transitions.size(); ++i) printf("%c ", transitions.at(i));
                        printf("\n");
                        for (int i = 0; i < links.size(); ++i) printf("%d ", links.at(i));
                        printf("\n");
                }
		bool is_in_string(std::string element, int pos_in_element, int pos_in_kmp)
		{
			while (pos_in_element < element.size())
			{
				if (pos_in_kmp == length) return true;
				if (element.at(pos_in_element) == transitions.at(pos_in_kmp))
				{
					++pos_in_element;
					++pos_in_kmp;
					if (pos_in_kmp == length) return true;
					continue;
				}
				else 
				{
					++pos_in_element;
					return is_in_string(element, pos_in_element, pos_in_kmp);
				}
			}
			return false;
		}

		void is_in_text(std::vector<std::string> text, std::string directory)
		{
			bool is_almost_one = false;
			int iter = 0;
			for (iter; iter < text.size(); ++iter)
			{
				if (is_in_string(text.at(iter), 0, 0))
				{
					is_almost_one = true;
					printf("%s:\n", directory.c_str());
					printf("%d: %s\n", iter+1, text.at(iter).c_str());
					++iter;
					break;
				}
			}
			for (iter; iter < text.size(); ++iter)
			{
				if (is_in_string(text.at(iter), 0, 0)) printf("%d: %s\n", iter+1, text.at(iter).c_str());
			}
			if (is_almost_one) printf("\n");
			//if (!is_almost_one) printf("%s: No entries!\n", directory.c_str());
		}

		void check_file(std::string directory)
		{
	        	FILE* file = fopen(directory.c_str(), "r");
        		if (!file)
        		{
                		fprintf(stderr, "can't open file: %s\n", directory.c_str());
                		return;
        		}
        		std::vector<std::string> text;
			std::string tmp;
	        	//std::string buffer;
        		fseek(file, 0, SEEK_END);
        		long long int lSize = ftell(file);
        		rewind(file);
        		char* buffer = (char*)malloc(sizeof(char)*lSize);
        		size_t result = fread(buffer, 1, lSize, file);
			for (int i = 0; i < lSize; ++i)
			{
				if (buffer[i] != '\n') tmp.push_back(buffer[i]);
				else
				{
					text.push_back(tmp);
					tmp.clear();
					continue;
				}
			}
        		//puts(buffer);
			//for (int i = 0; i < text.size(); ++i) printf("%s\n", text.at(i).c_str());
        		is_in_text(text, directory);
        		free(buffer);
			text.clear();
        		fclose(file);
		}

		void* check_thread(std::vector<std::string> thread)
		{
			for (int iter = 0; iter < thread.size(); ++iter) check_file(thread.at(iter));
			return NULL;
		}
};

std::string get_directory()
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
void walk_recursive(std::string const &dirname, std::vector<std::string> &ret)
{
	DIR *dir = opendir(dirname.c_str());
	if (dir == nullptr) 
	{
		//ret.push_back(dirname);
        	perror(dirname.c_str());
        	return;
    	}
    	for (dirent *de = readdir(dir); de != NULL; de = readdir(dir)) {
        	if (strcmp(".",de->d_name) == 0 || strcmp("..", de->d_name) == 0) continue;
        	//ret.push_back(dirname + "/" + de->d_name);
        	if (de->d_type != DT_DIR)
		{
			ret.push_back(dirname + "/" + de->d_name);
		}
		else if (de->d_type == DT_DIR)
			walk_recursive(dirname + "/" + de->d_name, ret);
   	 }
   	 closedir(dir);
}
void walk_non_recursive(std::string const& dirname, std::vector<std::string>& ret)
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
	std::string pattern, directory;
	std::vector<std::string> ret;
	for (int iter = 1; iter < argc; ++iter)
	{
		if (std::string(argv[iter]) == "-n") only_current_dir = true;
		else if (argv[iter][0] == '-' && argv[iter][1] == 't') threads_num = (int)(argv[iter][2] - 48);
		else if (pattern.empty()) pattern = std::string(argv[iter]);
		else directory = std::string(argv[iter]);
	}
	if (directory.empty()) directory = get_directory();
	//printf("%s\n%s\n%d\n%d\n", pattern.c_str(), directory.c_str(), only_current_dir, threads_num);

	//KMP aut(pattern);
	const char* dirname = directory.c_str();
	only_current_dir ? walk_non_recursive(dirname, ret) : walk_recursive(dirname, ret);
	printf("\n");
	std::sort(ret.begin(), ret.end(), [](std::string &s1, std::string &s2){return s1.length() > s2.length();});
	std::vector<KMP> args;
	args.resize(threads_num);
	for (int iter = 0; iter < threads_num; ++iter)
	{
		if (iter >= ret.size()) break;
		args[iter].get_KMP(pattern);
		args[iter].paths.push_back(ret.at(iter));
	}
	for (int iter = threads_num; iter < ret.size();)
	{
		for (int jter = 0; jter < threads_num; ++jter)
		{
			if (iter >= ret.size()) break;
			args[jter].paths.push_back(ret.at(iter));
		}
	}
	pthread_t *threads = (pthread_t *) malloc(threads_num * sizeof(pthread_t));
    	pthread_mutex_t mutex_main;
	pthread_mutex_init(&mutex_main, NULL);
	for(int iter = 0; iter < threads_num; ++iter)
	{
        	args[iter].mutex = mutex_main;
        	pthread_create(threads+iter, NULL, KMP::check_thread, args[iter]);
    	}
	for(int iter = 0; iter < threads_num; ++iter)
	{
        	pthread_join(threads[iter], NULL);
    	}
	
	//for (int i = 0; i < ret.size(); ++i) printf("%s\n", ret[i].c_str());
	//for (int i = 0; i < ret.size(); ++i) aut.check_file(ret.at(i).c_str());
	//std::vector<std::thread> search;
	return 0;
}


