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

int do_join(int argc, char** argv, float percent) {
	if (argc < 3) {
		Logger::error("usage: % file1 file2 ...", argv[0]);
		return -1;
	}
	vector<unique_ptr<CSVTable>> ts;
	for (int i = 1; i < argc; ++i) {
		ts.resize(i);
		ts.back().reset(new CSVTable());
		ts.back()->load(argv[i]);
	}

	vector<set<size_t>> cols;
	for (const auto &x : ts) {
		cols.push_back(set<size_t>());
		x->get_primary_keys(&cols.back());
		if (cols.back().empty()) {
			Logger::info("No candidate primary keys found.");
			return 0;
		}
	}

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
	}
}

int main(int argc, char** argv) {
	if (argc < 3) {
		Logger::error("usage: % file1 file2 ...", argv[0]);
		return -1;
	}
	if (do_join(argc, argv, .95)) {
		return 0;
	}

	Logger::info("No candidate column set found. Enter new matching threshold?");
	string answer;
	float percent = 0;
	cin >> answer;
	if (answer.empty()) return 0;
	percent = atof(answer.c_str());
	if (percent >= 1) percent /= 100;
	if (percent == 0 || percent > 1) return 0;
	if (!do_join(argc, argv, percent)) {
		Logger::info("Sorry. Still no luck.");
	}
	return 0;
}
