#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <functional>

#include "ib/containers.h"
#include "ib/csv_table.h"
#include "ib/logger.h"
#include "ib/tokenizer.h"

#include "seasick/map_reduce.h"

using namespace std;
using namespace std::placeholders;
using namespace ib;
using namespace seasick;

int main(int argc, char** argv) {
	if (argc < 4) {
		Logger::error("usage: % keycols valcols operation", argv[0]);
		return -1;
	}
	bool need_sort = true;
	if (string(argv[0]).find("csv_sorted_mr") != string::npos) {
		need_sort = false;
	}
	/* TODO
	if (argc > 4) {
		operations["morethan"] = bind(&morethan, argv[4], _1);
	}*/

	set<size_t> keycols;
	set<size_t> valcols;
	Tokenizer::numset(argv[1], &keycols, -1);
	Tokenizer::numset(argv[2], &valcols, -1);
	size_t max = 0;
	for (auto &x : keycols) {
		if (x > max) max = x;
	}
	for (auto &x : valcols) {
		if (x > max) max = x;
	}
	++max;

	string operation = argv[3];

	string cur = "";
	bool unset = true;
	CSVTable<false> ts;
	ts.stream();
	vector<string> vals;

	if (!need_sort) {
	while (true) {
		vector<string> row;
		ts.get_next_row(&row);
		if (!row.size()) break;
		if (row.size() == 1 && row[0].empty()) break;
		if (row.size() < max) continue;
		string key = Tokenizer::join(row, ",", keycols);
		if (unset || cur != key) {
			if (vals.size()) {
				try {
					if (keycols.size()) cout << cur << ",";
					cout << MapReduce::run(operation, vals) << endl;
				} catch (char const *) {}
			}
			unset = false;
			cur = key;
			vals.clear();
		}
		vals.push_back(Tokenizer::join(row, ",", valcols));
	}
	if (!unset) {
		try {
			cout << cur << ","
			     << MapReduce::run(operation, vals)
			     << endl;
		} catch (char const*) {}
	}
	} else {
	map<string, vector<string>> mapping;
	while(true) {
		vector<string> row;
                ts.get_next_row(&row);
                if (!row.size()) break;
                if (row.size() == 1 && row[0].empty()) break;
                string key = Tokenizer::join(row, ",", keycols);
		mapping[key].push_back(Tokenizer::join(row, ",", valcols));
	}
	for (auto &x : mapping) {
		try {
			if (keycols.size()) cout << x.first << ",";
			cout << MapReduce::run(operation, x.second) << endl;
		} catch (char const*) {}
	}
	}
}
