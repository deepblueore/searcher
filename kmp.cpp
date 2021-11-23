#include <stdio.h>
#include <string>
#include <vector>
#include <iostream> 
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>

class KMP
{
	private:
		std::vector<std::string> prefixes = {""};
		std::vector<char> transitions;
		std::vector<int> links = {0};
		int length;
	public:
		int recursive_link_detector(int number, char symbol)
		{
			if (links[number] == 0 && (transitions[0] != symbol || number == 0)) return 0;
			if (number > 0 && transitions[links[number]] == symbol) return links[number] + 1;
			else return recursive_link_detector(links[number], symbol);
		}

		KMP(std::string pattern)
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

		void is_in_text(std::vector<std::string> text)
		{
			bool is_almost_one = false;
			for (int iter = 0; iter < text.size(); ++iter)
			{
				if (is_in_string(text.at(iter), 0, 0))
				{
					is_almost_one = true;
					printf("%d: %s\n", iter, text.at(iter).c_str());
				}
			}
			if (!is_almost_one) printf("No entries!\n");
		}

};


int main(int argc, char* argv[])
{
	bool only_current_dir = false;
	int threads_num = 1;
	std::string pattern, directory;
	for (int iter = 1; iter < argc; ++iter)
	{
		if (std::string(argv[iter]) == "-n") only_current_dir = true;
		else if (argv[iter][0] == '-' && argv[iter][1] == 't') threads_num = (int)(argv[iter][2] - 48);
		else if (pattern.empty()) pattern = std::string(argv[iter]);
		else directory = std::string(argv[iter]);
	}
	if (directory.empty()) only_current_dir = true;
	//printf("%s\n%s\n%d\n%d\n", pattern.c_str(), directory.c_str(), only_current_dir, threads_num);
	
	return 0;
}

