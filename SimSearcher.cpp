#include "SimSearcher.h"
#include <cstdlib>

using namespace std;

SimSearcher::SimSearcher()
{
}

SimSearcher::~SimSearcher()
{
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{
	char buf[1024];
	unsigned tmp_num = 0;
	FILE *fp = fopen(filename, "r");
	while (fgets(buf ,sizeof(buf), fp))
	{
		unsigned n = strlen(buf) - 1;
		buf[n] = '\0';
		string tmp_str = buf;
		strs.push_back(buf);
		for (unsigned i = 0; i < n - q + 1; i++)
		{
			string q_gram = tmp_str.substr(i, q);
			inverted_list[q_gram].push_back(tmp_num);
		}
		tmp_num++;
	}

	map<string, vector<unsigned> >::iterator it;
	for (it = inverted_list.begin(); it != inverted_list.end(); it++)
	{
		vector<unsigned>::iterator iter = unique(it->second.begin(), it->second.end());
		it->second.erase(iter, it->second.end());
	}

	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
	return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
	result.clear();

	string query_str = query;
	

	return SUCCESS;
}

void SimSearcher::print_inverted_list()
{
	map<string, vector<unsigned> >::iterator it;
	for (it = inverted_list.begin(); it != inverted_list.end(); it++)
	{
		printf("%s: ", (it->first).c_str());
		unsigned size = it->second.size();
		for (unsigned i = 0; i < size; i++)
		{
			printf("%d ", (it->second)[i]);
		}
		printf("\n");
	}
}

void SimSearcher::print_ed_result(std::vector<std::pair<unsigned, unsigned> > &result)
{

}