#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

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

int main()
{
}

