#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "ib/csv_table.h"
#include "ib/fileutil.h"
#include "ib/formatting.h"
#include "ib/logger.h"
#include "ib/set.h"
#include "ib/tokenizer.h"

using namespace std;
using namespace ib;

int main(int argc, char** argv) {
	if (argc < 4) {
		Logger::error("usage: % file1 cols file2 cols", argv[0]);
		return -1;
	}
	vector<unique_ptr<CSVTable<false>>> ts;
	vector<set<size_t>> cols;
	for (int i = 1; i < argc; i += 2) {
		ts.push_back(unique_ptr<CSVTable<false>>());
		ts.back().reset(new CSVTable<false>());
		ts.back()->stream(argv[i]);
		cols.push_back(set<size_t>());
		Tokenizer::numset(argv[i + 1], &cols.back());
	}

	while (true) {
		vector<vector<string>> outpart;
		vector<string> inpart;
		for (size_t i = 0; i < ts.size(); ++i) {
			string in, out;
			while (true) {
				vector<string> row;
				ts[i]->get_next_row(&row);
				if (!row.size()) break;
				Tokenizer::join(row, ",", cols[i], &in, &out);
				if (inpart.size() && in < inpart[0]) continue;
				if (inpart.size() && in > inpart[0]) {
					ts[i]->set_next_row(row);
					break;
				}

				if (inpart.size() == i) {
					inpart.push_back(in);
					outpart.push_back(vector<string>());
				}
				outpart.back().push_back(out);
			}
		}
		if (!inpart.size()) return 0;
		vector<vector<string>::iterator> iters;
                for (size_t i = 0; i < outpart.size(); ++i) {
			if (outpart[i].empty()) {
				continue;
			}
			iters.push_back(outpart[i].begin());
		}
		if (iters.size() != ts.size()) continue;

		while (true) {
			cout << inpart[0] << ",";
			for (size_t i = 0; i < outpart.size(); ++i) {
				assert(inpart[i] == inpart[0]);
				if (iters[i]->length())
					cout << *iters[i] << ",";
			}
			cout << endl;

			bool finished = true;
			for (size_t i = 0; i < outpart.size(); ++i) {
				iters[i]++;
				if (iters[i] == outpart[i].end()) {
					iters[i] = outpart[i].begin();
				} else {
					finished = false;
					break;
				}
			}
			if (finished) break;
		}
	}


/*
	vector<size_t> found_col;
	for (const auto &x : cols[0]) {
		set<string> master_col;
		ts[0]->get_column(x, &master_col);
		bool found = true;
		found_col.push_back(x);

		for (int i = 1; i < cols.size(); ++i) {
			bool match = false;
			for (const auto z : cols[i]) {
				set<string> candidate;
				ts[i]->get_column(z, &candidate);
				if (Set::close(candidate, master_col, percent)) {
					match = true;
					found_col.push_back(z);
					break;
				}
			}
			if (!match) {
				found = false;
				break;
			}
		}
		if (found) {
			Logger::info("Found a match %", found_col);
			for (size_t i = 0; i < found_col.size(); ++i) {
				Logger::info("\t%",
					     ts[i]->get_header(found_col[i]));
			}
			Logger::info("If you are happy type yes");
			string word;
			cin >> word;
			if (word == "yes") {
				break;
			}
		}
		found_col.clear();
	}
	if (found_col.size()) {
		Logger::info("Okay, saving to joined.csv");
		for (int i = 1; i < ts.size(); ++i) {
			ts[0]->join(found_col[0], *(ts[i].get()), found_col[i]);
		}
		ts[0]->save("joined.csv");
		return true;
	} else {
		return false;
	}*/
}
