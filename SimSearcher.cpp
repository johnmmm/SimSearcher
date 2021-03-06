#include "SimSearcher.h"
#include <cstdlib>

using namespace std;

vector<unsigned>* inverted_list_ed_hash[ed_hash];
vector<unsigned>* inverted_list_jac_hash[jac_hash];
unsigned jac_is_dup[jac_hash];

bool cmp2(pair<vector<unsigned>*, unsigned>a, pair<vector<unsigned>*, unsigned>b) //降序排序
{
    return a.second > b.second;
}

bool cmp3(pair<set<unsigned>*, unsigned>a, pair<set<unsigned>*, unsigned>b) //降序排序
{
    return a.second > b.second;
}

template<typename UINT>
int levenshtein_bit_vector(string a, int m, string b, int n)
{    
    int r = m;
    UINT vp = (UINT)-1;
    UINT vn = 0;
    UINT hp = 0;
    UINT hn = 0;
    int bl = min<int>(n, sizeof(UINT) * 8);
    for (int j = 0; j < m; j++) {
        UINT pm = 0;
        for (int i = bl-1; i >= 0; i--) {
            pm <<= 1;
            pm |= b[i] == a[j];
        }
        UINT d = (((pm & vp) + vp)) ^ vp | pm | vn;
        hp = (vn | ~(d | vp));
        UINT hpw = (hp << 1) | 1;
        hn = d & vp;
        vp = (hn << 1) | ~(d | hpw);
        vn = d & hpw;
    }
    for (int i = 0; i < bl; i++) {
        r += (vp & 1) - (vn & 1);
        vp >>= 1;
        vn >>= 1;
    }
    return r;
}

SimSearcher::SimSearcher()
{
}

SimSearcher::~SimSearcher()
{
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{    
    memset(jac_is_dup, 0, sizeof(int)*jac_hash);
    
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
        set<unsigned long long> tmp_hash;

        if (s_min > tmp_tokens.size())
            s_min = tmp_tokens.size();
        for (int i = 0; i < tmp_tokens.size(); i++)
        {
            unsigned new_hash_num = new_jaccard_hash(tmp_tokens[i]);

            if (inverted_list_jac_hash[new_hash_num] == NULL)
            {
                inverted_list_jac_hash[new_hash_num] = new vector<unsigned>;
                inverted_list_jac_hash[new_hash_num]->push_back(tmp_num);
            }
            else
            {
                if (inverted_list_jac_hash[new_hash_num]->back() != tmp_num)
                    inverted_list_jac_hash[new_hash_num]->push_back(tmp_num);
                //替代set完成去重
            }
            tmp_hash.insert(new_hash_num);
        }
        str_tokens.push_back(tmp_hash);

        //ed:
        vector<unsigned> q_gram_vec;
        for (unsigned i = 0; i < n - q + 1; i++)
        {
            string q_gram = tmp_str.substr(i, q);
            q_gram_vec.push_back(q_gram_hash(q_gram));
        }
        for (int i = 0; i < q_gram_vec.size(); i++)
        {
            if (inverted_list_ed_hash[q_gram_vec[i]] == NULL)
            {
                inverted_list_ed_hash[q_gram_vec[i]] = new vector<unsigned>;
                inverted_list_ed_hash[q_gram_vec[i]]->push_back(tmp_num);
            }
            else
            {
                if (inverted_list_ed_hash[q_gram_vec[i]]->back() != tmp_num)
                    inverted_list_ed_hash[q_gram_vec[i]]->push_back(tmp_num);
            }   
        }
		tmp_num++;
	}

	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
    string query_str = query;
    vector<string> query_tokens;
    tokenize(query_str, query_tokens);

    //search_jac_scancount(query_tokens, threshold, result);
    search_jac_divideskip(query_tokens, threshold, result);
    
    return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
	result.clear();
	string query_str = query;
	
    //search_ed_scancount(query_str, threshold, result);
    //search_ed_mergeopt(query_str, threshold, result);
    search_ed_divideskip(query_str, threshold, result);
	
	return SUCCESS;
}

void SimSearcher::print_inverted_list()
{
	// for (int i = 0; i < 6; i++)
    // {
    //     printf("%s\n", strs[i].c_str());
    // }
    printf("inverted list jac:\n");

    printf("inverted list ed:\n");
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

double SimSearcher::jaccard_distance(set<unsigned long long> a, set<unsigned long long> b, unsigned same_gram)
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
    set<unsigned> insection_result;
    set_intersection(a.begin(), a.end(), 
                    b.begin(), b.end(), 
                    inserter(insection_result, insection_result.begin()));
    cnt = insection_result.size();
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

void SimSearcher::tokenize(string str1, vector<string> &res)
{
    //memset(jac_is_dup, 0, sizeof(int)*jac_hash);
    string results;
    stringstream input(str1);
    vector<unsigned> changed_num;
    while(input>>results)
    {
        unsigned new_hash = new_jaccard_hash(results);
        if (jac_is_dup[new_hash] == 0)
        {
            jac_is_dup[new_hash]++;
            changed_num.push_back(new_hash);
            res.push_back(results);
        }
    }
    for (int i = 0; i < changed_num.size(); i++)
    {
        jac_is_dup[changed_num[i]] = 0;
    }
    //这个地方有一个去重的问题，所以之前用了set，可以更改jachash来解决这个问题


    // int place = 0;
    // for (int i = 0; i < str1.size(); i++)
    // {
    //     if (str1[i] == ' ')
    //     {
    //         res.push_back(str1.substr(place, i-place));
    //     }
    // }
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

unsigned SimSearcher::new_jaccard_hash(string strs)
{
    unsigned long long hash_num = 0;
    int seed = 173;
    int len = strs.size();
    for (int i = 0; i < len; i++) {
        hash_num = hash_num * seed + (int)strs[i];
    }
    return hash_num & 0x3FFFFFF;
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

void SimSearcher::search_jac_scancount(set<string> query_tokens, double threshold, vector<pair<unsigned, double> > &result)
{
    unsigned str_size = query_tokens.size();
    int least_common = (int)ceil( max(threshold*str_size, (str_size+s_min)*threshold/(1.0+threshold) ) );
    unsigned ids_total = strs.size();
	int nums[ids_total];
	for(unsigned i = 0; i < ids_total; i++)
		nums[i] = 0;
    set<unsigned long long> query_hash;

    set<string>::iterator it1;
    for (it1 = query_tokens.begin(); it1 != query_tokens.end(); it1++)
    {
        unsigned long long tmp_long_long = new_jaccard_hash(*it1);
        query_hash.insert(tmp_long_long);
        vector<unsigned>* common_gram = inverted_list_jac_hash[tmp_long_long];
        for (unsigned j = 0; j < (*common_gram).size(); j++)
            nums[(*common_gram)[j]]++;
    }

    for (unsigned i = 0; i < ids_total; i++)
	{
		if (nums[i] >= least_common)
		{
            double distance = jaccard_distance(str_tokens[i], query_hash, nums[i]);
            //double distance = new_jaccard_distance(str_tokens[i], query_hash);
            //printf("%f\n", distance);
			if (distance >= threshold)
			{
				result.push_back(make_pair(i, distance));
			}
		}
	}
}

void SimSearcher::search_jac_divideskip(vector<string> &query_tokens, double threshold, vector<pair<unsigned, double> > &result)
{
    unsigned str_size = query_tokens.size();
    int least_common = (int)ceil( max(threshold*str_size, (str_size+s_min)*threshold/(1.0+threshold) ) );
    unsigned ids_total = strs.size();
	int nums[ids_total];
	memset(nums, 0, sizeof(int)*ids_total);
    set<unsigned long long> query_hash;
    vector<unsigned> selected_str;
    vector<pair<vector<unsigned>*, unsigned> > skip_list;

    set<string>::iterator it1;
    for (int i = 0; i < query_tokens.size(); i++)
    {
        unsigned long long tmp_long_long = new_jaccard_hash(query_tokens[i]);
        query_hash.insert(tmp_long_long);
        vector<unsigned>* selected_list = inverted_list_jac_hash[tmp_long_long];
        if (selected_list != NULL)
        {
            skip_list.push_back(make_pair(selected_list, (*selected_list).size()));
        }
    }

    sort(skip_list.begin(), skip_list.end(), cmp2);
    divideskip_jac(skip_list, selected_str, least_common, nums);

    for (int i = 0; i < selected_str.size(); i++)
    {
        double distance = jaccard_distance(str_tokens[selected_str[i]], query_hash, nums[selected_str[i]]);
        //double distance = new_jaccard_distance(str_tokens[selected_str[i]], query_hash);
        //printf("%f\n", distance);
        if (distance >= threshold)
        {
            result.push_back(make_pair(selected_str[i], distance));
        }
    }
}

void SimSearcher::search_ed_scancount(string query_str, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
    int least_common = query_str.size() - q_num + 1 - threshold * q_num;
    unsigned ids_total = strs.size();

    if (least_common > 0)
    {
        int nums[ids_total];
        for(unsigned i = 0; i < ids_total; i++)
            nums[i] = 0;
        
        for (unsigned i = 0; i < query_str.size() - q_num + 1; i++)
        {
            string q_gram = query_str.substr(i, q_num);
            unsigned hash_num = q_gram_hash(q_gram);
            vector<unsigned>* selected_list = inverted_list_ed_hash[hash_num];
            if (selected_list != NULL)
            {
                for (unsigned j = 0; j < (*selected_list).size(); j++)
                    nums[(*selected_list)[j]]++;
            }
            
        }

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
    }

    else
    {
        for (unsigned i = 0; i < ids_total; i++)
        {
            unsigned distance = new_lenenshtein_distance(strs[i], query_str, threshold);
            //printf("%d\n", distance);
            if (distance <= threshold)
            {
                result.push_back(make_pair(i, distance));
            }
        }
    }
    
	
}

void SimSearcher::search_ed_mergeopt(string query_str, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
    int least_common = query_str.size() - q_num + 1 - threshold * q_num;
    unsigned ids_total = strs.size();
    vector<unsigned> selected_str;
    vector<vector<unsigned>* > waiting_list;
    //printf("least_common: %d\n", least_common);

    if (least_common > 0)
    {
        for (unsigned i = 0; i < query_str.size() - q_num + 1; i++)
        {
            string q_gram = query_str.substr(i, q_num);
            unsigned hash_num = q_gram_hash(q_gram);
            vector<unsigned>* selected_list = inverted_list_ed_hash[hash_num];
            if (selected_list != NULL)
                waiting_list.push_back(selected_list);
        }

        //printf("begin!\n");
        mergeopt(waiting_list, selected_str, least_common);

        // printf("size: %lu\n", selected_str.size());
        // for (int i = 0; i < selected_str.size(); i++)
        // {
        //     printf("%d, ", selected_str[i]);
        // }
        // printf("\n");

        for (int i = 0; i < selected_str.size(); i++)
        {
            unsigned distance = new_lenenshtein_distance(strs[selected_str[i]], query_str, threshold);
            //printf("%d\n", distance);
            if (distance <= threshold)
            {
                result.push_back(make_pair(selected_str[i], distance));
            }
        }
    }

    else
    {
        for (unsigned i = 0; i < ids_total; i++)
        {
            unsigned distance = new_lenenshtein_distance(strs[i], query_str, threshold);
            //printf("%d\n", distance);
            if (distance <= threshold)
            {
                result.push_back(make_pair(i, distance));
            }
        }
    }
    
}

void SimSearcher::search_ed_divideskip(string query_str, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
    int least_common = query_str.size() - q_num + 1 - threshold * q_num;
    unsigned ids_total = strs.size();
    vector<unsigned> selected_str;
    vector<pair<vector<unsigned>*, unsigned> > skip_list;

    if (least_common > 0)
    {
        for (unsigned i = 0; i < query_str.size() - q_num + 1; i++)
        {
            string q_gram = query_str.substr(i, q_num);
            unsigned hash_num = q_gram_hash(q_gram);
            vector<unsigned>* selected_list = inverted_list_ed_hash[hash_num];
            if (selected_list != NULL)
            {
                skip_list.push_back(make_pair(selected_list, (*selected_list).size()));
            }
        }

        //printf("begin:\n");
        sort(skip_list.begin(), skip_list.end(), cmp2);
        //printf("divide:\n");
        divideskip(skip_list, selected_str, least_common);

        for (int i = 0; i < selected_str.size(); i++)
        {
            unsigned distance = new_lenenshtein_distance(strs[selected_str[i]], query_str, threshold);
            // // const char *a = strs[selected_str[i]].c_str();
            // // const char *b = query_str.c_str();
            // unsigned distance;
            // int m = strs[selected_str[i]].size(), n = query_str.size();
            // // if (m < n)
            // //     swap(a, b), swap(m, n);
            // if (n <= 64)
            //     distance = levenshtein_bit_vector<uint64_t>(strs[selected_str[i]], m, query_str, n);
            // else if (n <= 128)
            //     distance = levenshtein_bit_vector<unsigned __int128>(strs[selected_str[i]], m, query_str, n);
            // else
            //     distance = new_lenenshtein_distance(strs[selected_str[i]], query_str, threshold);
            if (distance <= threshold)
            {
                result.push_back(make_pair(selected_str[i], distance));
            }
        }
    }

    else
    {
        for (unsigned i = 0; i < ids_total; i++)
        {
            unsigned distance = new_lenenshtein_distance(strs[i], query_str, threshold);
            // // const char *a = strs[i].c_str();
            // // const char *b = query_str.c_str();
            // unsigned distance;
            // //int m = (int)strlen(a), n = (int)strlen(b);
            // int m = strs[i].size(), n = query_str.size();
            // // if (m < n)
            // //     swap(a, b), swap(m, n);
            // if (n <= 64)
            //     distance = levenshtein_bit_vector<uint64_t>(strs[i], m, query_str, n);
            // else if (n <= 128)
            //     distance = levenshtein_bit_vector<unsigned __int128>(strs[i], m, query_str, n);
            // else
            //     distance = new_lenenshtein_distance(strs[i], query_str, threshold);
            //printf("%d\n", distance);
            if (distance <= threshold)
            {
                result.push_back(make_pair(i, distance));
            }
        }
    }
    
}

void SimSearcher::mergeopt(vector<vector<unsigned>* > &waiting_list, vector<unsigned> &selected_str, int count_thres)
{
    // for (int i = 0; i < waiting_list.size(); i++)
    // {
    //     printf("%d: ", i);
    //     for (int j = 0; j < waiting_list[i].size(); j++)
    //     {
    //         printf("%d, ", waiting_list[i][j]);
    //     }
    //     printf("\n");
    // }
    
    //在这里对已经排好序的列表们进行筛选，得到一个可行的列表
    unsigned waiting_size = waiting_list.size();
    unsigned ids_total = strs.size();
    //unsigned ids_total = 16;
    unsigned count_place = 0;//已经取得的最小值，当其为ids那么多的时候就结束了
    //重复至没有要搜的

    //维护这些变量，重复使用
    vector<unsigned> need_to_search;//表示这些waiting_list需要搜一遍
    for (unsigned i = 0; i < waiting_size; i++)
        need_to_search.push_back(i);
    int occur_num[ids_total]; //选出来的那些数值的数量
    unsigned waiting_place[waiting_size];//当前计算到哪一个了，到顶就炸
    vector<unsigned> num_to_list[ids_total];//记录那些被选中的数值来自哪些链
    memset(occur_num, 0, sizeof(unsigned)*ids_total);
    memset(waiting_place, 0, sizeof(unsigned)*waiting_size);
    set<unsigned> occur_str; //选出来的那些数值，按大小排好了已经

    while (need_to_search.size() != 0)
    {
        //debug:
        // printf("count_place: %d\n", count_place);
        

        // printf("occur_num: ");
        // for (int i = 0; i < ids_total; i++)
        // {
        //     printf("%d, ", occur_num[i]);
        // }
        // printf("\n");

        // printf("waiting_place: ");
        // for (int i = 0; i < waiting_size; i++)
        // {
        //     printf("%d, ", waiting_place[i]);
        // }
        // printf("\n");
        
        //更新搜索
        for (unsigned i = 0; i < need_to_search.size(); i++)
        {
            unsigned place = need_to_search[i];
            if (waiting_place[place] < (*waiting_list[place]).size())
            {
                unsigned tmp_str_num = (*waiting_list[place])[waiting_place[place]];
                occur_str.insert(tmp_str_num);
                occur_num[tmp_str_num]++;
                num_to_list[tmp_str_num].push_back(place);
            }
        }
        need_to_search.clear();

        //开始筛选，前进
        int already_delete = 0; //occur_num[choice_one] >= count_thres - already_delete
        set<unsigned>::iterator it1;
        it1 = occur_str.begin();
        while (it1 != occur_str.end())
        {
            if (occur_num[*it1] < count_thres - already_delete)//这个不行
            {
                already_delete += occur_num[*it1];
                count_place = *it1 + 1;
                //回溯这边的位置
                for (unsigned j = 0; j < num_to_list[*it1].size(); j++)
                {
                    waiting_place[num_to_list[*it1][j]]++;
                    need_to_search.push_back(num_to_list[*it1][j]);
                }   
                it1++;
                occur_str.erase(occur_str.begin()); 
            }
            else if (occur_num[*it1] >= count_thres)//直接爆掉
            {
                selected_str.push_back(*it1);
                for (unsigned j = 0; j < num_to_list[*it1].size(); j++)
                {
                    waiting_place[num_to_list[*it1][j]]++;
                    need_to_search.push_back(num_to_list[*it1][j]);
                }  
                count_place = *it1 + 1;
                occur_str.erase(occur_str.begin()); 
                break;
            }
            else    //  左边的便都没有意义了
            {
                count_place = *it1;
                break;
            }
                
        }

        // printf("num_to_list: \n");
        // for (unsigned i = 0; i < ids_total; i++)
        // {
        //     printf("%d:  ", i);
        //     for (unsigned j = 0; j < num_to_list[i].size(); j++)
        //     {
        //         printf("%d, ", num_to_list[i][j]);
        //     }
        //     printf("\n");
        // }

        // printf("need to search: ");
        // for (unsigned i = 0; i < need_to_search.size(); i++)
        // {
        //     printf("%d, ", need_to_search[i]);
        // }
        // printf("\n");



        //寻找对应点在每个表中的位置
        for (unsigned i = 0; i < need_to_search.size(); i++)
        {
            unsigned place = need_to_search[i];
            waiting_place[place] = bi_search(*waiting_list[place], count_place, waiting_place[place]);
        }
        //这轮就算完了应该
    }
    
}

void SimSearcher::divideskip_jac(vector<pair<vector<unsigned>*, unsigned> > &skip_list, vector<unsigned> &selected_str, int count_thres, int* nums)
{
    unsigned count_thres_plus;
    if (count_thres <= 2)
        count_thres_plus = 1;
    else if (count_thres <= 4)
        count_thres_plus = 2;
    else 
        count_thres_plus = 4;
    unsigned ids_total = strs.size();
    memset(nums, 0, sizeof(int)*ids_total);
    int search_place[count_thres-1];
    memset(search_place, 0, sizeof(int)*(count_thres-1));
    vector<unsigned> avail_str;

    //printf("scancount:\n");
    //scancount
    for (int i = count_thres - count_thres_plus; i < skip_list.size(); i++)
    {
        vector<unsigned>* tmp_pointer = skip_list[i].first;
        for (unsigned j = 0; j < (*tmp_pointer).size(); j++)
        {
            nums[(*tmp_pointer)[j]]++;
            if (nums[(*tmp_pointer)[j]] == count_thres_plus)
            {
                avail_str.push_back((*tmp_pointer)[j]);
            }
                
        }      
    }

    sort(avail_str.begin(), avail_str.end());
    int is_push[ids_total];
    memset(is_push, 0, sizeof(int)*ids_total);

    for (int i = 0; i < avail_str.size(); i++)
    {
        if (nums[avail_str[i]] >= count_thres)
        {
            if (is_push[avail_str[i]] == 0)
            {
                selected_str.push_back(avail_str[i]);
                is_push[avail_str[i]] = 1;
            }
            // selected_str.push_back(avail_str[i]);
            // continue;
        }
                    
        for (int j = count_thres - count_thres_plus - 1; j >= 0; j--)
        {
            unsigned new_place = bi_search(*(skip_list[j].first), avail_str[i], search_place[j]);
            if ((*skip_list[j].first)[new_place] == avail_str[i])
            {
                nums[avail_str[i]]++;
                search_place[j] = new_place + 1;
                if (nums[avail_str[i]] >= count_thres)
                {
                    if (is_push[avail_str[i]] == 0)
                    {
                        selected_str.push_back(avail_str[i]);
                        is_push[avail_str[i]] = 1;
                    }
                        
                }
            }
            else
            {
                search_place[j] = new_place;
            }
        }
    }
}

void SimSearcher::divideskip(vector<pair<vector<unsigned>*, unsigned> > &skip_list, vector<unsigned> &selected_str, int count_thres)
{
    unsigned count_thres_plus;
    if (count_thres <= 2)
        count_thres_plus = 1;
    else if (count_thres <= 4)
        count_thres_plus = 2;
    else 
        count_thres_plus = 4;
    unsigned ids_total = strs.size();
    int nums[ids_total];
    memset(nums, 0, sizeof(int)*ids_total);
    int search_place[count_thres-1];
    memset(search_place, 0, sizeof(int)*(count_thres-1));
    vector<unsigned> avail_str;

    //printf("scancount:\n");
    //scancount
    for (int i = count_thres - count_thres_plus; i < skip_list.size(); i++)
    {
        vector<unsigned>* tmp_pointer = skip_list[i].first;
        for (unsigned j = 0; j < (*tmp_pointer).size(); j++)
        {
            nums[(*tmp_pointer)[j]]++;
            if (nums[(*tmp_pointer)[j]] == count_thres_plus)
            {
                selected_str.push_back((*tmp_pointer)[j]);
            }
                
        }      
    }

    //选择直接无视长链
    sort(selected_str.begin(), selected_str.end());
    return;
}

//寻找合适的出来的点，如果没有就返回前面一个
unsigned SimSearcher::bi_search(vector<unsigned> &one_list, unsigned one_place, unsigned start_place)
{
    unsigned left = start_place;
    unsigned right = one_list.size();

    while (left + 1 < right)
    {
        unsigned next = (left + right) >> 1;
        if (one_list[next] <= one_place)
            left = next;
        else
            right = next;
    }
    if (one_list[left] < one_place)
        return right;
    else
        return left;
}

void SimSearcher::mergeopt_cell_test()
{
    vector<vector<unsigned> > waiting_list;
    vector<unsigned> vector1;
    vector<unsigned> vector2;
    vector<unsigned> vector3;
    vector<unsigned> vector4;
    vector<unsigned> vector5;

    vector1.push_back(1);
    vector1.push_back(3);
    vector1.push_back(5);
    vector1.push_back(10);
    vector1.push_back(13);

    vector2.push_back(10);
    vector2.push_back(13);
    vector2.push_back(15);

    vector3.push_back(5);
    vector3.push_back(7);
    vector3.push_back(13);

    vector4.push_back(13);

    vector5.push_back(15);

    waiting_list.push_back(vector1);
    waiting_list.push_back(vector2);
    waiting_list.push_back(vector3);
    waiting_list.push_back(vector4);
    waiting_list.push_back(vector5);

    vector<unsigned> selected_str;

    //mergeopt(waiting_list, selected_str, 4);

    for (int i = 0; i < selected_str.size(); i++)
    {
        printf("%d, ", selected_str[i]);
    }
    printf("\n");
}
