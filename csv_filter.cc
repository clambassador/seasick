#include <cassert>
#include <iostream>
#include <set>
#include <string>

#include "ib/fileutil.h"
#include "ib/logger.h"

using namespace ib;
using namespace std;

int main(int argc, char** argv) {
	assert(argc == 4);
	size_t col = atoi(argv[2]);
	assert(col != 0);
	set<string> allowed;

	Fileutil::read_file(argv[1], &allowed);
	while (cin.good()) {
		string s;
		getline(cin, s);
		size_t cur = col - 1;
		size_t i = 0;
		for (; i < s.length(); ++i) {
			if (!cur) break;
			if (s[i] == ',') --cur;
		}
		string remain = s.substr(i);
		size_t pcomma = remain.find(",");
		string token = remain.substr(0, pcomma);
		Logger::debug("check % is allowed", token);
		if (allowed.count(token)) cout << s << endl;
	}
}
