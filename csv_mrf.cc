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
	if (string(argv[0]).find("csv_sorted_mrf") != string::npos) {
		need_sort = false;
	}

	set<size_t> keycols;
	set<size_t> valcols;
	Tokenizer::numset(argv[1], &keycols, -1);
	Tokenizer::numset(argv[2], &valcols, -1);

	string operation = argv[3];
	assert(operation.length());

	// TODO: allow and and or here

	string cur = "";
	bool unset = true;
	CSVTable<false> ts;
	ts.stream();
	vector<string> vals;

	if (!need_sort) {
	stringstream ss;
	while (true) {
		stringstream next_ss;
		vector<string> row;
		ts.get_next_row(&row, &next_ss);
		next_ss << endl;
		if (!row.size()) break;
		if (row.size() == 1 && row[0].empty()) break;
		string key = Tokenizer::join(row, ",", keycols);
		if (unset || cur != key) {
			if (vals.size()) {
				try {
					if (MapReduce::test(operation, vals)) {
						cout << ss.str();
					}
				} catch (string s) {
					Logger::error("%", s);
					return -1;
				}
			}
			unset = false;
			cur = key;
			vals.clear();
			ss.str("");
		}
		ss << next_ss.str();
		vals.push_back(Tokenizer::join(row, ",", valcols));
	}
	if (!unset) {
		try {
			if (MapReduce::test(operation, vals)) {
				cout << ss.str();
			}
		} catch (string s) {
			Logger::error("%", s);
			return -1;

		}
	}
	} else {
	map<string, vector<string>> mapping;
	map<string, stringstream> sss;
	while(true) {
		stringstream ss;
		vector<string> row;
                ts.get_next_row(&row, &ss);
                if (!row.size()) break;
                if (row.size() == 1 && row[0].empty()) break;
                string key = Tokenizer::join(row, ",", keycols);
		mapping[key].push_back(Tokenizer::join(row, ",", valcols));
		sss[key] << ss.str() << endl;
	}
	for (auto &x : mapping) {
		try {
			if (MapReduce::test(operation, x.second)) {
				cout << sss[x.first].str();
			}
		} catch (string s) {
			Logger::error("%", s);
			return -1;
		}
	}
	}
}
