#include <cassert>
#include <iostream>
#include <set>
#include <string>

#include "ib/fileutil.h"
#include "ib/logger.h"
#include "ib/tokenizer.h"

using namespace ib;
using namespace std;

int main(int argc, char** argv) {
	if (argc != 4) {
		Logger::error("usage: % colnum op len", argv[0]);
		return -1;
	}
	size_t col = atoi(argv[1]);
	string op = string(argv[2]);
	size_t len = atoi(argv[3]);
	assert(col != 0);
	while (cin.good()) {
		string s;
		string val;
		getline(cin, s);
		if (s.empty()) continue;
		Tokenizer::fast_split(s, ',', col, &val);
		assert(val.length() > 4);
		assert(val[0] == '|');
		assert(val[1] == '|');
		assert(val[val.length() - 1] == '|');
		assert(val[val.length() - 2] == '|');
		int vals = 1;
		for (size_t i = 2; i < val.length() - 2; ++i) {
			if (val[i] == '|') {
				vals += 1;
			}
		}
		if (op == "EQ" && vals == len) cout << s << endl;
		if (op == "NEQ" && vals != len) cout << s << endl;
		if (op == "LEQ" && vals <= len) cout << s << endl;
		if (op == "LT" && vals < len) cout << s << endl;
		if (op == "GEQ" && vals >= len) cout << s << endl;
		if (op == "GT" && vals > len) cout << s << endl;
	}
}
