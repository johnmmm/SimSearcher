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
	q_num = q;
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
	int least_common = query_str.size() - q_num + 1 - threshold * q_num;
	map<string, vector<unsigned> > common_gram;
	for (unsigned i = 0; i < query_str.size() - q_num + 1; i++)
	{
		string q_gram = query_str.substr(i, q_num);
		common_gram[q_gram] = inverted_list[q_gram];
	}
	unsigned ids_total = strs.size();
	int nums[ids_total];
	for(unsigned i = 0; i < ids_total; i++)
		nums[i] = 0;
	map<string, vector<unsigned> >::iterator it;
	for (it = common_gram.begin(); it != common_gram.end(); it++)
	{
		for (unsigned i = 0; i < it->second.size(); i++)
		{
			nums[it->second[i]]++;
		}
	}
	vector<unsigned> avail_dis;
	for (unsigned i = 0; i < ids_total; i++)
	{
		if (nums[i] >= least_common)
		{
			unsigned distance = lenenshtein_distance(strs[i], query_str);
			if (distance <= threshold)
			{
				result.push_back(make_pair(i, distance));
			}
		}
	}

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
	for(unsigned i = 0; i < result.size(); i++)
	{
		printf("(%d, %d), ", result[i].first, result[i].second);
	}
	printf("\n");
}

unsigned SimSearcher::lenenshtein_distance(string a, string b)
{
	
	
	return 1;
}