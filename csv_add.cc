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
		Logger::error("usage: % val", argv[0]);
		return -1;
	}

	while (cin.good()) {
		string s, val;
		getline(cin, s);
		if (s.empty()) continue;
		cout << s << "," << argv[1] << endl;
	}
}
