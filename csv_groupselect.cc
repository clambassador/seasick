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
	assert(operation.length());

	// TODO: allow and and or here

	string cur = "";
	bool unset = true;
	CSVTable<false> ts;
	ts.stream();
	vector<string> vals;

	stringstream ss;
	while (true) {
		stringstream next_ss;
		vector<string> row;
		ts.get_next_row(&row, &next_ss);
		next_ss << endl;
		if (!row.size()) break;
		if (row.size() == 1 && row[0].empty()) break;
		if (row.size() < max) continue;
		string key = Tokenizer::join(row, ",", keycols);
		if (unset || cur != key) {
			if (vals.size()) {
				try {
					string maxval =
					    MapReduce::reduce_max(vals);
					while (ss.good()) {
						string s, val;
						getline(ss, s);
						Tokenizer::fast_split(s, ',',
								      1+ *valcols.begin(),
								      &val);
						if (val == maxval) {
							cout << s <<endl;
						}
					}
					ss.clear();
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
					string maxval =
					    MapReduce::reduce_max(vals);
					while (ss.good()) {
						string s, val;
						getline(ss, s);
						Tokenizer::fast_split(s, ',',
								      1 + *valcols.begin(),
								      &val);
						if (val == maxval) {
							cout << s << endl;
						}
					}
		} catch (string s) {
			Logger::error("%", s);
			return -1;

		}
	}
}
