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

	while (cin.good()) {
		string s;
		getline(cin, s);
		vector<string> p;
		Tokenizer::split(s, ",", &p);
		if (p.size() < col) continue;

		if (p[col - 1].find(str) == string::npos) continue;

		cout << s << endl;
	}
}
