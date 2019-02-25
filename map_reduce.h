#ifndef __SEASICK__MAP_REDUCE__H__
#define __SEASICK__MAP_REDUCE__H__

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

namespace seasick {

class MapReduce {
public:
	static string vectorize(const vector<string>& input) {
		stringstream ss;
		ss << "||";
		for (const auto &x : input) {
			ss << x << "|";
		}
		ss << "|";
		return ss.str();
	}

	static string count(const vector<string>& input) {
		return Logger::stringify(input.size());
	}

	static string morethan(string param, const vector<string>& input) {
		size_t amount = atoi(param.c_str());
		if (input.size() > amount)
			return Logger::stringify(input.size());
		else throw "skip";
	}

	static string unique_count(const vector<string>& input) {
		return Logger::stringify(Containers::unique(input).size());
	}

	static string unique(const vector<string>& input) {
		return vectorize(Containers::unique(input));
	}

	static string reduce_sum(const vector<string>& input) {
		int64_t sum = 0;
		for (auto &x : input) {
			sum += atoll(x.c_str());
		}
		return Logger::stringify(sum);
	}

	static string reduce_max(const vector<string>& input) {
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

	static string reduce_min(const vector<string>& input) {
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

	static bool more_than_one(const vector<string>& input) {
		return input.size() == 1;
	}

	static bool is_unique(const vector<string>& input) {
		return Containers::unique(input).size() == 1;
	}

	static bool test(string operation, const vector<string>& vals) {
		static map<string, function<bool(const vector<string>&)>>
			_operations;

		if (!_operations.size()) {
		        _operations["more_than_one"] = bind(&more_than_one, _1);
		        _operations["is_unique"] = bind(&is_unique, _1);
		}

		bool negate = false;
		if (operation.substr(0, 4) == "not_") {
                	negate = true;
	                operation = operation.substr(4);
        	}

		if (!_operations.count(operation))
			throw string("no such operation: " + operation);

		return negate ^ _operations[operation](vals);
	}

	static string run(const string& operation, const vector<string>& vals) {
		static map<string, function<string(const vector<string>&)>>
			_operations;

		if (!_operations.size()) {
		        _operations["max"] = bind(&reduce_max, _1);
		        _operations["min"] = bind(&reduce_min, _1);
		        _operations["sum"] = bind(&reduce_sum, _1);
		        _operations["vectorize"] = bind(&vectorize, _1);
		        _operations["unique"] = bind(&unique, _1);
		        _operations["count"] = bind(&count, _1);
		        _operations["unique_count"] = bind(&unique_count, _1);
		        _operations["uniqcount"] = bind(&unique_count, _1);
		}
		// todo : handle morethan cleverly
		if (!_operations.count(operation))
			throw string("no such operation: " + operation);

		return _operations[operation](vals);
	}
};

}  // namespace seasick

#endif  // __SEASICK__MAP_REDUCE__H__
