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
	bool exact = false;
	if (argc < 2) {
		Logger::error("usage: % col mapfile [exact]", argv[0]);
		return -1;
	}
	if (argc == 4) exact = true;
	map<string, string> fr;
	CSVTable<false>::load_map(argv[2], &fr);
	CSVTable<false> table;

	table.stream();
	int col = atoi(argv[1]) - 1;

	while (true) {
		vector<string> row;
		table.get_next_row(&row);
		if (!row.size()) break;
		if (row.size() == 1 && row[0] == "") break;
		if (exact) {
			if (fr.count(row[col])) {
				row[col] = fr[row[col]];
			}
		} else {
			for (auto &x: fr) {
				if (row[col].find(x.first) != string::npos) {
					row[col] = Tokenizer::replace(
						row[col], x.first,
						x.second);
				}
			}
		}
		stringstream ss;
		for (auto &x : row) {
			ss << Formatting::csv_escape(x)
			   << ",";
		}
		cout << ss.str().substr(0, ss.str().length() - 1) << endl;
	}
}

