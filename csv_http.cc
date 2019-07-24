#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "ib/csv_table.h"
#include "ib/fileutil.h"
#include "ib/logger.h"
#include "ib/tokenizer.h"

using namespace std;
using namespace ib;

int main(int argc, char** argv) {
	CSVTable<false> table;
	table.stream();

	cout << "<table>" << endl;
	while (cin.good()) {
		vector<string> line;
		table.get_next_row(&line);
		if (!line.size()) continue;

		cout << "  <tr>" << endl;
		for (const auto &x : line) {
			cout << "    <td>" << x << "</td>";
		}
		cout << "  </tr>" << endl;
	}
	cout << "</table>" << endl;
}
