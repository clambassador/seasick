#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "ib/logger.h"
#include "ib/tokenizer.h"

using namespace std;
using namespace ib;

int main(int argc, char** argv) {
	if (argc < 2) {
		Logger::error("usage: % col", argv[0]);
		return -1;
	}

	int col = atoi(argv[1]);
	string cur = "";
	set<string> vals;
	bool unset = true;
	while (cin.good()) {
		string s, val;
		getline(cin, s);
		string in, out;
		if (!Tokenizer::pop_split(s, ',', col, &in, &out)) continue;
		if (unset || cur != in) {
			if (vals.size() == 1) {
				cout << in << ",";
				for (auto &x : vals) {
					cout << x;
				}
				cout << endl;
			}
			unset = false;
			cur = in;
			vals.clear();
		}
		vals.insert(out);
	}
}
