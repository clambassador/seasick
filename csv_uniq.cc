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
		Logger::error("usage: % col [freq]", argv[0]);
		return -1;
	}

	int col = atoi(argv[1]);
	int freq = 1;
	if (argc > 2) freq = atoi(argv[2]);
	map<string, int> result;

	int count = 1;
	while (cin.good()) {
		++count;
		if (count % 1000000 == 0) Logger::error("at %", count);
		string line, str;
		getline(cin, line);
		if (!Tokenizer::fast_split(line, ',', col, &str)) continue;

		result[str]++;
	}
	multimap<int, string> matches;
	for (auto &x : result) {
		if (x.second < freq) continue;
		matches.insert(make_pair(-x.second, x.first));
	}
	for (auto &x : matches) {
		cout << -x.first <<  " " << x.second << endl;
	}
}
