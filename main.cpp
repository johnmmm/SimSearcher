#include "SimSearcher.h"

using namespace std;

int main(int argc, char **argv)
{
	SimSearcher searcher;

	vector<pair<unsigned, unsigned> > resultED;
	vector<pair<unsigned, double> > resultJaccard;

	unsigned q = 3, edThreshold = 6;
	double jaccardThreshold = 0.2;

    //searcher.mergeopt_cell_test();

	searcher.createIndex(argv[1], q);
	searcher.searchJaccard("query", jaccardThreshold, resultJaccard);
	searcher.searchED("query", edThreshold, resultED);
	//searcher.print_inverted_list();
    searcher.print_jaccard_result(resultJaccard);
	searcher.print_ed_result(resultED);

	return 0;
}

