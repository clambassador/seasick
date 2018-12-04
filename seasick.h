#ifndef __SEASICK__SEASICK__H__
#define __SEASICK__SEASICK__H__

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "ib/config.h"
#include "ib/fileutil.h"
#include "ib/logger.h"
#include "ib/run.h"
#include "ib/tokenizer.h"

using namespace ib;
using namespace std;

namespace seasick {

class Seasick {
public:
	Seasick() {}
	virtual ~Seasick() {}

	virtual int process(const string& cs, string* output) {
		stringstream ss;
		string tmp = Tokenizer::replace(cs, "==", "!!");
		tmp = Tokenizer::replace(tmp, "=", " = ");
		tmp = Tokenizer::replace(tmp, "!!", "==");
	        vector<string> tokens;
		Tokenizer::split(tmp, " ", &tokens);
		size_t cur = 0;

		if (tokens.empty()) throw "where are the tokens, man?";
		string result = "";
		if (tokens.size() > 2 && tokens[1] == "=") {
			cur = 2;
			result = tokens[0];
			_var_to_file[result] = temp_filename(result);
			result = _var_to_file[result];
		}
		string start = tokens[cur++];
		if (_var_to_file.count(start) == 0) {
			_var_to_file[start] = start;
		} else {
			start = _var_to_file[start];
		}

		stringstream command;
		command << "cat " << start;

	        while (cur < tokens.size()) {
        	        string op = tokens[cur++];
                	if (op == "filter") {
				if (cur + 2 >= tokens.size())
					throw "filter needs more args";
	                        string word = tokens[cur++];
        	                string match = tokens[cur++];
	                        set<size_t> pos;
        	                Tokenizer::numset(tokens[cur++], &pos);
				if (pos.size() == 0)
					throw "filter needs a column";
				if (pos.size() != 1)
					throw "filter only works for one col---for now!";
                        	if (match == "in") {
					command << "| csv_grep " << *pos.begin()
					        << " " << word;
        	                } else if (match == "==") {
					command << "| csv_grep " << *pos.begin()
					        << " " << word << " exact ";
                        	} else throw "bad filter: either in or ==";
	                } else if (op == "negate") {
	                } else if (op == "fill") {
				if (_var_to_file.count(tokens[cur]) == 0)
					throw "can't find " + tokens[cur]
						+ " in my file list.";
				if (_var_to_file[tokens[cur]].empty())
					throw "filename for " + tokens[cur]
					+ " is empty.";
				if (!Fileutil::exists(_var_to_file[tokens[cur]]))
					throw "can't find a file at "
					    + _var_to_file[tokens[cur]];

				command << "| csv_filter "
				        << _var_to_file[tokens[cur]]
					<< " "
					<< tokens[cur + 1];
				cur += 2;
	                } else if (op == "uniq") {
				command << "| cut -d, -f" << tokens[cur++]
				        << " | sort | uniq ";
	                } else if (op == "project") {
				command << "| cut -d, -f" << tokens[cur++];
	                } else if (op == "count") {
		        } else throw "hmm, " + op + " doesn't seem like anything I support";
	        }

		if (result.empty()) {
			command << " | head -n 200";
		}
		Run run(command.str());
		run();
		if (result.empty()) {
			*output = run.read();
		} else {
			run.redirect(result);
		}

	        return 0;
	}
protected:

	virtual string temp_filename(const string& name) {
		assert(Config::_()->gets("tmp").find("..") == string::npos);
		if (Config::_()->gets("tmp").empty()) return name;
		return Config::_()->gets("tmp") + "/" + name;
	}

	map<string, string> _var_to_file;

};

}  // namespace seasick

#endif  // __SEASICK__SEASICK__H__
