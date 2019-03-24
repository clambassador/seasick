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
	if (argc < 3) {
		Logger::error("usage: % file1 cols file2 [default]", argv[0]);
		return -1;
	}
	vector<set<size_t>> cols;
	map<string, string> newcol;
	string default_val = "";
	if (argc == 5) default_val = argv[4];
	CSVTable<false>::load_map(argv[3], &newcol);

	CSVTable<false> main, out;
	main.stream(argv[1]);
	int col = atoi(argv[2]) - 1;

	while (true) {
		vector<string> row;
		main.get_next_row(&row);
		if (!row.size()) break;
		if (row.size() == 1 && row[0] == "") break;
		string newval = default_val;
		if (newcol.count(row[col])) {
			newval = newcol[row[col]];
		}
		for (auto &x : row) {
			cout << Formatting::csv_escape(x)
			     << ",";
		}
		cout << newval << endl;
	}
}

