#include "SimSearcher.h"
#include <cstdlib>

using namespace std;

vector<unsigned> inverted_list_ed_hash[2222222];

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
        set<string> tmp_tokens;
        tokenize(tmp_str, tmp_tokens);
        set<unsigned long long> tmp_hash;

        if (s_min > tmp_tokens.size())
            s_min = tmp_tokens.size();
        set<string>::iterator it1;
        for (it1 = tmp_tokens.begin(); it1 != tmp_tokens.end(); it1++)
        {
            inverted_list_jac[jaccard_hash(*it1)].push_back(tmp_num);
            tmp_hash.insert(jaccard_hash(*it1));
        }
        str_tokens.push_back(tmp_hash);

        //ed:
        set<unsigned> q_gram_set;
        for (unsigned i = 0; i < n - q + 1; i++)
        {
            string q_gram = tmp_str.substr(i, q);
            q_gram_set.insert(q_gram_hash(q_gram));
        }
        set<unsigned>::iterator it2;
        for (it2 = q_gram_set.begin(); it2 != q_gram_set.end(); it2++)
        {
            inverted_list_ed_hash[*it2].push_back(tmp_num);
        }
		tmp_num++;
	}

	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
    string query_str = query;
    set<string> query_tokens;
    tokenize(query_str, query_tokens);

    unsigned str_size = query_tokens.size();
    int least_common = (int)ceil( max(threshold*str_size, (str_size+s_min)*threshold/(1.0+threshold) ) );
    unsigned ids_total = strs.size();
	int nums[ids_total];
	for(unsigned i = 0; i < ids_total; i++)
		nums[i] = 0;

    set<unsigned long long> query_hash;
    search_jac_scancount(nums, query_tokens, query_hash);
    

    // printf("least: %d\n", least_common);
    // for (int i = 0; i < ids_total; i++)
    // {
    //     printf("%d: %d\n", i, nums[i]);
    // }
    for (unsigned i = 0; i < ids_total; i++)
	{
		if (nums[i] >= least_common)
		{
            //double distance = jaccard_distance(str_tokens[i], query_tokens, nums[i]);
            double distance = new_jaccard_distance(str_tokens[i], query_hash);
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
    unsigned ids_total = strs.size();
	int nums[ids_total];
	for(unsigned i = 0; i < ids_total; i++)
		nums[i] = 0;

    search_ed_scancount(nums, query_str);
	
	for (unsigned i = 0; i < ids_total; i++)
	{
		if (nums[i] >= least_common)
		{
            //unsigned distance = lenenshtein_distance(strs[i], query_str);
            unsigned distance = new_lenenshtein_distance(strs[i], query_str, threshold);
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
    // map<string, vector<unsigned> >::iterator it1;
	// for (it1 = inverted_list_jac.begin(); it1 != inverted_list_jac.end(); it1++)
	// {
	// 	printf("%s: ", (it1->first).c_str());
	// 	unsigned size = it1->second.size();
	// 	for (unsigned i = 0; i < size; i++)
	// 	{
	// 		printf("%d ", (it1->second)[i]);
	// 	}
	// 	printf("\n");
	// }

    // printf("inverted list ed:\n");
    // map<string, vector<unsigned> >::iterator it;
	// for (it = inverted_list_ed.begin(); it != inverted_list_ed.end(); it++)
	// {
	// 	printf("%s: ", (it->first).c_str());
	// 	unsigned size = it->second.size();
	// 	for (unsigned i = 0; i < size; i++)
	// 	{
	// 		printf("%d ", (it->second)[i]);
	// 	}
	// 	printf("\n");
	// }
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

double SimSearcher::new_jaccard_distance(set<unsigned long long> a, set<unsigned long long> b)
{
    unsigned size_a = a.size();
    unsigned size_b = b.size();
    int cnt = 0;
    for (auto w : a)
    {
        if (b.find(w) != b.end())
            cnt++;
    }
    return ((double)cnt / (double)(size_a + size_b - cnt));
}

unsigned SimSearcher::lenenshtein_distance(string a, string b)
{
	unsigned size_a = a.size();
    unsigned size_b = b.size();
    unsigned dp[2][size_b+1];
    for (unsigned i = 0; i <= size_b; i++)
        dp[0][i] = i;
    for (unsigned i = 1; i <= size_a; i++)
    {
        dp[i&1][0] = i;
        for (int j = 1; j <= size_b; j++)
        {
            if (a[i-1] == b[j-1])
                dp[i&1][j] = dp[(i-1)&1][j-1];
            else
                dp[i&1][j] = min( min(dp[(i-1)&1][j], dp[i&1][j-1]), dp[(i-1)&1][j-1] ) + 1;        
        }
    }
	return dp[size_a&1][size_b];
}

unsigned SimSearcher::new_lenenshtein_distance(string a, string b, unsigned threshold)
{
    int size_a = a.size();
    int size_b = b.size();
    int thres = (int)threshold;

    if (abs(size_a - size_b) > threshold)
        return MAXN;
    int dp[size_a+1][size_b+1];

    for (int i = 0; i <= min(thres, size_a); i++)
    {
        dp[i][0] = i;
    }
    for (int j = 0; j <= min(thres, size_b); j++)
    {
        dp[0][j] = j;
    }
    for (int i = 1; i <= size_a; i++)
    {
        int begin = max(i - thres, 1);
        int end = min(i + thres, size_b);
        if (begin > end)
            break;
        for (int j = begin; j <= end; j++)
        {
            int t = !(a[i - 1] == b[j - 1]);
            int d1 = abs(i - 1 - j) > threshold ? MAXN : dp[i - 1][j];
            int d2 = abs(i - j + 1) > threshold ? MAXN : dp[i][j - 1];
            dp[i][j] = min( min(d1+1, d2+1), dp[i-1][j-1]+t );            
        }
    }
    return (unsigned)dp[size_a][size_b];
}

void SimSearcher::tokenize(string str1, set<string> &res)
{
    string results;
    stringstream input(str1);
    while(input>>results)
    {
        res.insert(results);
    }
}

unsigned long long SimSearcher::jaccard_hash(string strs)
{
    unsigned long long hash_num = 0;
    int seed = 173;
    int len = strs.size();
    for (int i = 0; i < len; i++) {
        hash_num = hash_num * seed + (int)strs[i];
    }
    return hash_num & 0x7FFFFFFF;
}

unsigned SimSearcher::q_gram_hash(string strs)
{
    unsigned seed = 129;
    unsigned hash_num = 0;
    int len = strs.size();
    for (int i = 0; i < len; i++) {
        hash_num = hash_num * seed + (int)strs[i];
    }
    if (hash_num > 2222221)
        hash_num %= 2222221;
    return hash_num;
}

void SimSearcher::search_jac_scancount(int *nums, set<string> query_tokens, set<unsigned long long> &query_hash)
{
    //set<unsigned long long> query_hash;
    set<string>::iterator it1;
    for (it1 = query_tokens.begin(); it1 != query_tokens.end(); it1++)
    {
        unsigned long long tmp_long_long = jaccard_hash(*it1);
        query_hash.insert(tmp_long_long);
        vector<unsigned> common_gram = inverted_list_jac[tmp_long_long];
        for (unsigned j = 0; j < common_gram.size(); j++)
            nums[common_gram[j]]++;
    }
}

void SimSearcher::search_ed_scancount(int *nums, string query_str)
{
    for (unsigned i = 0; i < query_str.size() - q_num + 1; i++)
	{
		string q_gram = query_str.substr(i, q_num);
        unsigned hash_num = q_gram_hash(q_gram);
        for (unsigned j = 0; j < inverted_list_ed_hash[hash_num].size(); j++)
            nums[inverted_list_ed_hash[hash_num][j]]++;
	}
}