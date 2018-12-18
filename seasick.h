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

#define NEEDS(x)	do {if (cur - 1 + x >= tokens.size()) \
	throw Logger::stringify("% needs % args", \
	tokens[cur - 1], x); } while(0);

#define INT_NON_ZERO(x) do {stringstream ss; ss << tokens[x]; int i = 0; ss >> i; \
	if (i == 0) { throw string(tokens[x] + " is NaN or 0");}} while(0);

using namespace ib;
using namespace std;

namespace seasick {

class Seasick {
public:
	Seasick() {
		load_grammar();
	}
	virtual ~Seasick() {}

	virtual void load_grammar() {
		vector<string> rules;
		vector<string> pipe;
		pipe.push_back("|");
		_commands["|"].reset(new Command(pipe));

		Fileutil::read_file(Config::_()->gets("grammar"), &rules);

		for (auto &x : rules) {
			vector<string> tokens;
			Tokenizer::split(x, " ", &tokens);
			_commands[tokens[0]] = nullptr;
			_commands[tokens[0]].reset(new Command(tokens));
		}

	}

	virtual int tab_complete(const string& line,
				 vector<string>* choices,
				 string* hint) {
		assert(choices);
		choices->clear();
		assert(hint);
		*hint = "";

		vector<string> tokens;
		get_tokens(line, &tokens);
		bool partial = false;
		if (line.size()) partial = line.at(line.size() - 1) != ' ';

		if (partial) {
			assert(tokens.size());
			string last = tokens.back();
			tokens.pop_back();
			tab_complete_impl(tokens, choices, hint);

			// TODO filter choices for last;
		} else {
			tab_complete_impl(tokens, choices, hint);
		}

		return 0;
	}
protected:
	virtual void tab_filevar(vector<string>* choices,
				 string* hint) {
		*hint = "file or variable";
		// TODO choices;
	}

	virtual void tab_commands(vector<string>* choices,
				  string* hint) {
		*hint = "command";
		for (auto &x : _commands) {
			choices->push_back(x.first);
		}
	}

	virtual void tab_complete_impl(const vector<string>& tokens,
				       vector<string>* choices,
				       string* hint) {
	//	Logger::debug("tab tokens: %", tokens);
		size_t start = 0;
		if (tokens.size() && tokens.front() == "cat")
			++start;

		if (start >= tokens.size()) {
			tab_filevar(choices, hint);
			return;
		}

		for (size_t i = start; i < tokens.size(); ++i) {
			if (tokens[i] == "=") {
				start = i + 1;
			}
		}
		if (start >= tokens.size()) {
			tab_filevar(choices, hint);
			return;
		}

		size_t cur = start + 1;
		string command = "";
		size_t arg = 0;
		size_t nargs = 0;
		while (cur < tokens.size()) {
			if (command.empty()) {
				command = tokens[cur];
				if (!_commands.count(command)) {
					command = "";
					++cur;
					continue;
				}
				nargs = _commands[command]->args();
				arg = 0;
				if (!nargs) command = "";
			} else {
				assert(nargs);
				++arg;
				if (arg == nargs) command = "";
			}
			++cur;
		}
		if (command.empty()) {
			tab_commands(choices, hint);
		} else {
			assert(_commands.count(command));
			_commands[command]->tab_complete(
			    arg, choices, hint);
		}
	}

	virtual void get_tokens(const string& str, vector<string>* tokens) {
		assert(tokens);
		tokens->clear();
		string tmp = Tokenizer::replace(str, "==", "!!");
		tmp = Tokenizer::replace(tmp, "=", " = ");
		tmp = Tokenizer::replace(tmp, "!!", "==");
		Tokenizer::split(tmp, " ", tokens);
	}

public:
	virtual int process(const string& cs, string* output) {
		stringstream ss;
	        vector<string> tokens;
		get_tokens(cs, &tokens);
		size_t cur = 0;

		if (tokens.empty()) throw "where are the tokens, man?";
		string result = "";
		if (tokens.size() > 2 && tokens[1] == "=") {
			cur = 2;
			result = tokens[0];
			_var_to_file[result] = temp_filename(result);
			result = _var_to_file[result];
		}
		if (tokens[cur] == "cat") ++cur;
		string start = tokens[cur++];
		if (_var_to_file.count(start) == 0) {
			if (!Fileutil::exists(start))
				throw "dude, \"" + start + "\" isn't an actual file!";
			_var_to_file[start] = start;
		} else {
			start = _var_to_file[start];
		}

		stringstream command;
		command << "cat " << start;

	        while (cur < tokens.size()) {
        	        string op = tokens[cur++];
                	if (op == "filter") {
				NEEDS(3);
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
				NEEDS(2);
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
	                } else if (op == "sort") {
				INT_NON_ZERO(cur);
				command << " | sort -t, -k" << tokens[cur++];
	                } else if (op == "nsort") {
				NEEDS(1);
				INT_NON_ZERO(cur);
				command << " | sort -n -t, -k" << tokens[cur++];
	                } else if (op == "uniq") {
				command << " | sort | uniq ";
	                } else if (op == "project" || op == "cut") {
				NEEDS(1);
				command << "| cut -d, -f" << tokens[cur++];
	                } else if (op == "filter_len") {
				NEEDS(3);
				command << "| csv_filter_len "
				        << tokens[cur]
					<< " "
					<< filter_len_op(tokens[cur + 1])
					<< " "
					<< tokens[cur + 2];
				cur += 3;
	                } else if (op == "count") {
			} else if (op == "|") {
				continue;  // nop for console consistency
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

	virtual void get_vars(string* s) {
		stringstream ss;
		for (auto &x : _var_to_file) {
			ss << x.first << " \t";
		}
		*s = ss.str();
	}
protected:

	class Command {
	public:
		Command(const vector<string>& rule) {
			auto it = rule.begin();
			_name = *it++;
			set<string> types;
			types.insert("STR");
			types.insert("COL");
			types.insert("COLS");
			types.insert("INT");
			while (it != rule.end()) {
				bool val = false;
				for (auto &x : types) {
					if (it->substr(0, x.length()) == x) {
						if (it->length() == x.length()) {
							_hints.push_back(x);
						} else if (it->at(x.length()) != ':')
							continue;
						else
							_hints.push_back(it->substr(
								x.length() + 1));
						_args.push_back(x);
						_options.push_back(vector<string>());
						val = true;
						break;
					}
				}
				if (!val) {
					_hints.push_back(*it);
					_options.push_back(vector<string>());
					Tokenizer::split(*it, "|",
							 &(_options.back()));

					_args.push_back("CHOICE");
				}
				++it;
			}
		}

		virtual size_t args() const {
			return _args.size();
		}

		virtual void tab_complete(size_t arg,
					  vector<string>* choices,
					  string* hint) const {
			assert(_args.size() == _hints.size());
			assert(_args.size() == _options.size());
			assert(arg < _args.size());
			*choices = _options.at(arg);
			*hint = _hints.at(arg);
		}


	protected:
		string _name;
		vector<vector<string>> _options;
		vector<string> _args;
		vector<string> _hints;
	};

	virtual string filter_len_op(const string& op) {
		if (op == "<") return "LT";
		if (op == "!=") return "NEQ";
		if (op == "==") return "EQ";
		if (op == ">") return "GT";
		if (op == "<=") return "LEQ";
		if (op == ">=") return "GEQ";
		throw "filter_len has bad op" + op;
	}

	virtual string temp_filename(const string& name) {
		assert(Config::_()->gets("tmp").find("..") == string::npos);
		if (Config::_()->gets("tmp").empty()) return name;
		return Config::_()->gets("tmp") + "/" + name;
	}

	map<string, string> _var_to_file;
	map<string, unique_ptr<Command>> _commands;

};

}  // namespace seasick

#endif  // __SEASICK__SEASICK__H__
