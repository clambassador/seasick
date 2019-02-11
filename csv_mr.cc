#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <functional>

#include "ib/containers.h"
#include "ib/csv_table.h"
#include "ib/logger.h"
#include "ib/tokenizer.h"

using namespace std;
using namespace std::placeholders;
using namespace ib;

string vectorize(const vector<string>& input) {
	stringstream ss;
	ss << "||";
	for (const auto &x : input) {
		ss << x << "|";
	}
	ss << "|" << endl;
	return ss.str();
}

string count(const vector<string>& input) {
	return Logger::stringify(input.size());
}

string morethan(string param, const vector<string>& input) {
	size_t amount = atoi(param.c_str());
	if (input.size() > amount)
		return Logger::stringify(input.size());
	else throw "skip";
}

string unique_count(const vector<string>& input) {
	return Logger::stringify(Containers::unique(input).size());
}

string unique(const vector<string>& input) {
	return vectorize(Containers::unique(input));
}

string reduce_sum(const vector<string>& input) {
	int64_t sum = 0;
	for (auto &x : input) {
		sum += atoll(x.c_str());
	}
	return Logger::stringify(sum);
}

string reduce_max(const vector<string>& input) {
	int64_t largest;
	bool unset = true;
	for (auto &x : input) {
		int64_t val = atoll(x.c_str());
		if (unset) {
			largest = val;
			unset = false;
		}
		if (largest < val) largest = val;
	}
	return Logger::stringify(largest);
}

string reduce_min(const vector<string>& input) {
	int64_t smallest;
	bool unset = true;
	for (auto &x : input) {
		int64_t val = atoll(x.c_str());
		if (unset) {
			smallest = val;
			unset = false;
		}
		if (smallest > val) smallest = val;
	}
	return Logger::stringify(smallest);
}

int main(int argc, char** argv) {
	if (argc < 4) {
		Logger::error("usage: % keycols valcols operation", argv[0]);
		return -1;
	}
	bool need_sort = true;
	if (string(argv[0]).find("csv_sorted_mr") != string::npos) {
		need_sort = false;
	}
	map<string, function<string(const vector<string>&)>> operations;
	operations["max"] = bind(&reduce_max, _1);
	operations["min"] = bind(&reduce_min, _1);
	operations["sum"] = bind(&reduce_sum, _1);
	operations["vectorize"] = bind(&vectorize, _1);
	operations["unique"] = bind(&unique, _1);
	operations["count"] = bind(&count, _1);

	operations["unique_count"] = bind(&unique_count, _1);
	operations["uniqcount"] = bind(&unique_count, _1);

	if (argc > 4) {
		operations["morethan"] = bind(&morethan, argv[4], _1);
	}

	set<size_t> keycols;
	set<size_t> valcols;
	Tokenizer::numset(argv[1], &keycols, -1);
	Tokenizer::numset(argv[2], &valcols, -1);

	string operation = argv[3];
	if (operations.count(operation) == 0) {
		Logger::error("no such operation %", operation);
		return -1;
	}

	string cur = "";
	bool unset = true;
	CSVTable<false> ts;
	ts.stream();
	vector<string> vals;

	if (!need_sort) {
	while (true) {
		vector<string> row;
		ts.get_next_row(&row);
		if (!row.size()) break;
		if (row.size() == 1 && row[0].empty()) break;
		string key = Tokenizer::join(row, ",", keycols);
		if (unset || cur != key) {
			if (vals.size()) {
				try {
					string val = operations[operation](vals);
					if (keycols.size()) cout << cur << ",";
					cout << operations[operation](vals)
					     << endl;
				} catch (char const *) {}
			}
			unset = false;
			cur = key;
			vals.clear();
		}
		vals.push_back(Tokenizer::join(row, ",", valcols));
	}
	if (!unset) {
		try {
			string val = operations[operation](vals);
			cout << cur << "," << operations[operation](vals)
			     << endl;
		} catch (char const*) {}
	}
	} else {
	map<string, vector<string>> mapping;
	while(true) {
		vector<string> row;
                ts.get_next_row(&row);
                if (!row.size()) break;
                if (row.size() == 1 && row[0].empty()) break;
                string key = Tokenizer::join(row, ",", keycols);
		mapping[key].push_back(Tokenizer::join(row, ",", valcols));
	}
	for (auto &x : mapping) {
		try {
			string val = operations[operation](x.second);
			if (keycols.size()) cout << x.first << ",";
			cout << operations[operation](x.second) << endl;
		} catch (char const*) {}
	}
	}
}
