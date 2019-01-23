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
		Logger::error("usage: % allowfile colnum", argv[0]);
		return -1;
	}
	set<size_t> cols;
	Tokenizer::numset(argv[2], &cols, -1);
	assert(cols.size());
	set<string> allowed;
	Fileutil::read_file(argv[1], &allowed);
	unique_ptr<CSVTable<false>> table;
	table.reset(new CSVTable<false>());
	table->stream();
	vector<string> row;
	while (table->get_next_row(&row)) {
		if (allowed.count(Tokenizer::join(row, ",", cols))) {
			cout << Tokenizer::join(row, ",") << endl;
		}
		row.clear();
	}
}
