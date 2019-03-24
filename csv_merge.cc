#include <cassert>
#include <iostream>
#include <set>
#include <string>

#include "ib/fileutil.h"
#include "ib/logger.h"
#include "ib/tokenizer.h"
#include "ib/csv_table.h"

using namespace ib;
using namespace std;

int main(int argc, char** argv) {
	if (argc != 3) {
		Logger::error("usage: % outcol incol", argv[0]);
		return -1;
	}
	set<size_t> cols;
	int outcol = atoi(argv[1]);
	int incol = atoi(argv[2]);
	if (!outcol) return -1;
	if (!incol) return -1;
	--outcol;
	--incol;
	unique_ptr<CSVTable<false>> table;
	table.reset(new CSVTable<false>());
	table->stream();
	vector<string> row;
	while (table->get_next_row(&row)) {
		if (row.size() <= incol || row.size() <= outcol) {
			row.clear();
			continue;
		}
		if (row[outcol].empty()) row[outcol] = row[incol];
		cout << Tokenizer::join(row, ",") << endl;
		row.clear();
	}
}
