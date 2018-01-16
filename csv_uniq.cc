#include <fstream>
#include <set>
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
	set<string> result;

	while (cin.good()) {
		string s;
		getline(cin, s);
		vector<string> p;
		Tokenizer::split(s, ",", &p);
		if (p.size() < col) continue;

		result.insert(p[col - 1]);
	}

	for (auto &x : result) {
		cout << x << endl;
	}
}
