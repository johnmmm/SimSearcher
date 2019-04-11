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
    s_min = (unsigned)MAXN;
	char buf[1024];
	unsigned tmp_num = 0;
	FILE *fp = fopen(filename, "r");
	while (fgets(buf ,sizeof(buf), fp))
	{
		unsigned n = strlen(buf) - 1;
		buf[n] = '\0';
		string tmp_str = buf;
		strs.push_back(tmp_str);

        //jac:
        vector<string> tmp_tokens;
        tokenize(tmp_str, tmp_tokens);
        str_tokens.push_back(tmp_tokens);
        if (s_min > tmp_tokens.size())
            s_min = tmp_tokens.size();
        for (unsigned i = 0; i < tmp_tokens.size(); i++)
        {
            inverted_list_jac[tmp_tokens[i]].push_back(tmp_num);
        }

        //ed:
		for (unsigned i = 0; i < n - q + 1; i++)
		{
			string q_gram = tmp_str.substr(i, q);
			inverted_list_ed[q_gram].push_back(tmp_num);
		}
		tmp_num++;
	}

    map<string, vector<unsigned> >::iterator it1;
	for (it1 = inverted_list_jac.begin(); it1 != inverted_list_jac.end(); it1++)
	{
		vector<unsigned>::iterator iter = unique(it1->second.begin(), it1->second.end());
		it1->second.erase(iter, it1->second.end());
	}

	map<string, vector<unsigned> >::iterator it2;
	for (it2 = inverted_list_ed.begin(); it2 != inverted_list_ed.end(); it2++)
	{
		vector<unsigned>::iterator iter = unique(it2->second.begin(), it2->second.end());
		it2->second.erase(iter, it2->second.end());
	}

	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
    string query_str = query;
    vector<string> query_tokens;
    tokenize(query_str, query_tokens);
    unsigned str_size = query_tokens.size();
    int least_common = (int)ceil( max(threshold*str_size, (str_size+s_min)*threshold/(1.0+threshold) ) );

	map<string, vector<unsigned> > common_gram;
    for (unsigned i = 0; i < query_tokens.size(); i++)
	{
		string q_gram = query_tokens[i];
		common_gram[q_gram] = inverted_list_jac[q_gram];
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
    // printf("least: %d\n", least_common);
    // for (int i = 0; i < ids_total; i++)
    // {
    //     printf("%d: %d\n", i, nums[i]);
    // }
    for (unsigned i = 0; i < ids_total; i++)
	{
		if (nums[i] >= least_common)
		{
            double distance = jaccard_distance(str_tokens[i], query_tokens, nums[i]);
            //printf("%f\n", distance);
			if (distance >= threshold)
			{
				result.push_back(make_pair(i, distance));
			}
		}
	}
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
		common_gram[q_gram] = inverted_list_ed[q_gram];
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
            //printf("%d\n", distance);
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
	// for (int i = 0; i < 6; i++)
    // {
    //     printf("%s\n", strs[i].c_str());
    // }
    printf("inverted list jac:\n");
    map<string, vector<unsigned> >::iterator it1;
	for (it1 = inverted_list_jac.begin(); it1 != inverted_list_jac.end(); it1++)
	{
		printf("%s: ", (it1->first).c_str());
		unsigned size = it1->second.size();
		for (unsigned i = 0; i < size; i++)
		{
			printf("%d ", (it1->second)[i]);
		}
		printf("\n");
	}

    printf("inverted list ed:\n");
    map<string, vector<unsigned> >::iterator it;
	for (it = inverted_list_ed.begin(); it != inverted_list_ed.end(); it++)
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

void SimSearcher::print_jaccard_result(std::vector<std::pair<unsigned, double> > &result)
{
    for(unsigned i = 0; i < result.size(); i++)
	{
		printf("(%d, %f), ", result[i].first, result[i].second);
	}
	printf("\n");
}

void SimSearcher::print_ed_result(std::vector<std::pair<unsigned, unsigned> > &result)
{
	for(unsigned i = 0; i < result.size(); i++)
	{
		printf("(%d, %d), ", result[i].first, result[i].second);
	}
	printf("\n");
}

double SimSearcher::jaccard_distance(vector<string> a, vector<string> b, unsigned same_gram)
{
    unsigned size_a = a.size();
    unsigned size_b = b.size();
    double jd = (double)same_gram / (double)(size_a + size_b - same_gram);
    return jd;
}

unsigned SimSearcher::lenenshtein_distance(string a, string b)
{
	unsigned size_a = a.size();
    unsigned size_b = b.size();
    unsigned dp[2][size_b+1];
    for (int i = 0; i <= size_b; i++)
        dp[0][i] = i;
    for (int i = 0; i <= size_a; i++)
    {
        dp[i&1][0] = i;
        for (int j = 1; j <= size_b; j++)
        {
            if (a[i-1] == b[j-1])
                dp[i&1][j] = dp[(i-1)&1][j-1];
            else
                dp[i&1][j] = min( min(dp[i-1&1][j], dp[i&1][j-1]), dp[i-1&1][j-1] ) + 1;        
        }
    }
	return dp[size_a&1][size_b];
}

void SimSearcher::tokenize(string str1, vector<string> &res)
{
    string results;
    stringstream input(str1);
    while(input>>results)
    {
        res.push_back(results);
    }
}