#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "ib/logger.h"
#include "ib/tokenizer.h"

using namespace std;
using namespace ib;

int main(int argc, char** argv) {
	if (argc < 3) {
		Logger::error("usage: % col string", argv[0]);
		return -1;
	}

	int col = atoi(argv[1]);
	string str = argv[2];
	bool exact = false;
	if (argc == 4 && string(argv[3]) == "exact") exact = true;

	while (cin.good()) {
		string s, val;
		getline(cin, s);
		vector<string> p;
		if (!Tokenizer::fast_split(s, ',', col, &val)) continue;
		if ((exact && val != str) || (val.find(str) == string::npos)) {
			cout << s << ",0" << endl;
		} else {
			cout << s << ",1" << endl;
		}
	}
}
