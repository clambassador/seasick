#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>


#include <iostream>
#include <string>
#include <vector>

#include "seasick.h"
#include "ib/config.h"

using namespace std;
using namespace ib;
using namespace seasick;

void run(const vector<string>& commands) {
	Seasick csick;
	string cur;
	try {
		for (auto &x : commands) {
			cur = x;
			string out;
			csick.process(x, &out);
			if (!out.empty()) {
				cout << out;
			}

		}
	} catch (string s) {
		Logger::error("error in %: %", cur, s);
	} catch (char const* s) {
		Logger::error("error in %: %", cur, s);
	}
}

void run() {
	Seasick csick;
	string prompt = " >  ";
	/* TODO: use an ncurses prompt through minibus */
	while (true) {
		string s, out;
		if (cin.eof()) { break; }
		try {
		cout << prompt << flush;
		getline(cin, s);
		if (s.empty()) continue;
		csick.process(s, &out);
		if (!out.empty()) cout << out;
		} catch (string ex) {
			Logger::error("\"%\" from %",
				      ex, s);
		} catch (char const* ex) {
			Logger::error("\"%\" from %",
				      ex, s);
		}
	}
}


int main(int argc, char** argv) {
	Config::_()->load("csick.cfg");
	if (argc == 2) {
		assert(Fileutil::exists(argv[1]));
		vector<string> commands;
		Fileutil::read_file(argv[1], &commands);
		run(commands);
		return 0;
	}

	run();
}

