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
	seasick::DataFrame *output_df = &df;
	for (size_t i = 2; i < argc; ++i) {
		tokens.push_back(argv[i]);
	}
	size_t cur = 0;
	while (cur < tokens.size()) {
		string op = tokens[cur++];
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
			assert(pos.size());
			df.fill(pos);
		} else if (op == "uniq") {
			set<size_t> pos;
			set<string> vals;
			Tokenizer::numset(tokens[cur++], &pos);
			assert(pos.size());
			df.project(pos, &vals);
			tmp.init(vals);
			output_df = &tmp;
		} else if (op == "project") {
			set<size_t> pos;
			vector<string> vals;
			Tokenizer::numset(tokens[cur++], &pos);
			assert(pos.size());
			df.project(pos, &vals);
			tmp.init(vals);
			output_df = &tmp;
		} else if (op == "count") {
			set<size_t> pos;
			map<string, size_t> vals;
			Tokenizer::numset(tokens[cur++], &pos);
			assert(pos.size());
			df.project(pos, &vals);
			tmp.init(vals);
			output_df = &tmp;
		} else if (op == "print") {
			output_df->trace();
		} else if (op == "save") {
			string filename = tokens[cur++];
			ofstream fout(filename);
			output_df->save(fout);
		} else assert(0);
	}
	return 0;
}
