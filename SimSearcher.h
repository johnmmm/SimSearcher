#pragma once
#include <vector>
#include <utility>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cstring>
#include <map>
#include <stdlib.h>
#include <math.h>
#include<iostream>
#include<sstream>
using namespace std;

const int SUCCESS = 0;
const int FAILURE = 1;
const int MAXN = 66666666;
const double eps = 1e-8;

class SimSearcher
{
public:
	SimSearcher();
	~SimSearcher();

	vector<string> strs;
    vector<vector<string> > str_tokens;
    map<string, vector<unsigned> > inverted_list_jac;
	map<string, vector<unsigned> > inverted_list_ed;
	unsigned q_num;
    unsigned s_min;

	int createIndex(const char *filename, unsigned q);
	int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double> > &result);
	int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned> > &result);

	void print_inverted_list();
    void print_jaccard_result(std::vector<std::pair<unsigned, double> > &result);
	void print_ed_result(std::vector<std::pair<unsigned, unsigned> > &result);
    double jaccard_distance(vector<string> a, vector<string> b, unsigned same_gram);
	unsigned lenenshtein_distance(string a, string b);

    void tokenize(string str1, vector<string> &res);
};

