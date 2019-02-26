#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "ib/fileutil.h"
#include "ib/logger.h"
#include "ib/tokenizer.h"

using namespace std;
using namespace ib;

int main(int argc, char** argv) {
	if (argc < 2) {
		Logger::error("usage: % allowfile [exact]", argv[0]);
		return -1;
	}
	bool exact = false;
	if (argc == 3) exact = true;
	set<string> allowed;
        Fileutil::read_file(argv[1], &allowed);

	while (cin.good()) {
		string s, val;
		getline(cin, s);
		if (exact) {
			if (allowed.count(s)) cout << s << endl;
		} else {
			for (auto &x : allowed) {
				if (s.find(x) != string::npos) {
					cout << s << endl;
					break;
				}
			}
		}
	}
}
