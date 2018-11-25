#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

#include "data_frame.h"

using namespace std;
using namespace ib;

int main(int argc, char** argv) {
	if (argc < 2) {
		Logger::error("usage: % datafile commands...", argv[0]);
		return -1;
	}

	vector<string> tokens;
	seasick::DataFrame df(argv[1]);
	seasick::DataFrame tmp;
	for (size_t i = 2; i < argc; ++i) {
		tokens.push_back(argv[i]);
	}
	size_t cur = 0;
	while (cur < tokens.size()) {
		string op = tokens[cur++];
		Logger::info("% % %", cur, tokens, op);
		if (op == "filter") {
			string word = tokens[cur++];
			string match = tokens[cur++];
			set<size_t> pos;
			Tokenizer::numset(tokens[cur++], &pos);
			assert(pos.size() == 1);
			if (match == "in") {
				df.filter(word, false, *(pos.begin()));
			} else if (match == "==") {
				df.filter(word, true, *(pos.begin()));
			} else assert(0);
		} else if (op == "negate") {
			df.negate();
		} else if (op == "fill") {
			set<size_t> pos;
			Tokenizer::numset(tokens[cur++], &pos);
			df.fill(pos);
		} else if (op == "uniq") {
			set<size_t> pos;
			set<string> vals;
			Tokenizer::numset(tokens[cur++], &pos);
			df.project(pos, vals);
			tmp.set(vals);
		} else if (op == "project") {
			set<size_t> pos;
			vector<string> vals;
			Tokenizer::numset(tokens[cur++], &pos);
			df.project(pos, &vals);
			tmp.set(vals);
		} else if (op == "count") {
			set<size_t> pos;
			map<string, size_t> vals;
			Tokenizer::numset(tokens[cur++], &pos);
			df.project(pos, &vals);
			tmp.set(vals);
		} else if (op == "print") {
			tmp.save(cout);
		} else if (op == "save") {
			string filename = tokens[cur++];
			ofstream fout(filename);
			tmp.save(fout);
		} else assert(0);
	}
	df.trace();
	return 0;
}
