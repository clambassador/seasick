#ifndef __SEASICK__SEASICK__H__
#define __SEASICK__SEASICK__H__

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "ib/config.h"
#include "ib/containers.h"
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
		map<string, string> types;
		vector<string> type_list;
		Config::_()->get_all("type", &types);
		assert(types.size());
		for (auto &x : types) {
			vector<string> type;
			Tokenizer::split(x.second, ",", &type);
			assert(type.size());
			_types[x.first] = type;
			type_list.push_back(x.first);
		}

		vector<string> rules;
		vector<string> pipe;
		pipe.push_back("|");
		_commands["|"].reset(new Command(pipe));

		Fileutil::read_file(Config::_()->gets("grammar"), &rules);

		for (auto &x : rules) {
			if (x.empty()) continue;
			vector<string> tokens;
			Tokenizer::split(x, " ", &tokens);
			_commands[tokens[0]] = nullptr;
			_commands[tokens[0]].reset(new Command(tokens,
							       type_list));
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
			vector<string> all;
			tab_complete_impl(tokens, last, &all, hint);
			for (auto &x : all) {
				if (x.find(last) == 0) {
					choices->push_back(x);
				}
			}
		} else {
			tab_complete_impl(tokens, "", choices, hint);
		}

		return 0;
	}

	virtual map<string, vector<string>> get_types() { return _var_to_type; }
	virtual vector<string> get_type(const string& command) {
		vector<string> tokens;
		vector<string> retval;
		get_tokens(command, &tokens);
		if (tokens.empty()) return vector<string>();
		tokens.pop_back();
		get_type(tokens, &retval);
		return retval;
	}

protected:
	virtual void tab_filevar(vector<string>* choices,
				 string* hint) const {
		*hint = "file or variable";
		for (const auto &x : _var_to_file) {
			choices->push_back(x.first);
		}
	//	Fileutil::list_directory(".", ".csv", choices);
		// TODO: add dirs
	}

	virtual void tab_commands(vector<string>* choices,
				  string* hint) {
		*hint = "command";
		for (auto &x : _commands) {
			choices->push_back(x.first);
		}
	}

	virtual void refine_type(vector<string>* type, const string& cols) {
		vector<string> vals;
		vector<string> result;
		Tokenizer::split(cols, ",", &vals);
		assert(vals.size());
		for (auto &x : vals) {
			size_t i = atoi(x.c_str());
			if (i != 0) {
				result.push_back((*type)[i - 1]);
			} else {
				for (auto &y : *type) {
					if (y == x) {
						result.push_back(x);
						break;
					}
				}
			}
		}
		Containers::reorder_vector(*type, result, type);
	}

	virtual int process_type(const vector<string>& tokens,
				 size_t* cur,
				 vector<string>* type) {
		string command = tokens[*cur];
		size_t args = _commands[command]->args();

		if (*cur + args >= tokens.size()) return -1;
		if (command == "cut" || command == "project") {
			refine_type(type, tokens[*cur + 1]);
		} else if (command == "type") {
			if (_types.count(tokens[*cur + 1])) {
				*type = _types[tokens[*cur + 1]];
				_var_to_type[get_start(tokens)] = *type;
				// TODO: if after a cut, this is notright
			}
		}
		*cur += args + 1;
		return 0;
	}

	virtual vector<string> get_type(const vector<string>& tokens,
					size_t pos) {
		assert(tokens.size() > pos);
		vector<string> temp;
		for (size_t i = 0; i < pos; ++i) {
			temp.push_back(tokens[i]);
		}
		vector<string> retval;
		get_type(temp, &retval);
		return retval;
	}
public:
	virtual string typeset_column(const vector<string>& type,
				      const string& columns) {
		if (type.empty()) return columns;
		stringstream ss;
		map<string, size_t> lookup;
		vector<string> pieces;
		Containers::lookup_vector(type, &lookup);
		Tokenizer::split(columns, ",", &pieces);

		for (auto &x: pieces) {
			size_t col = atoi(x.c_str());
			if (col) ss << col << ",";
			else {
				if (!lookup.count(x))
					throw Logger::stringify(
					    "no match for % in % while %",
					    x, type, columns);
				ss << lookup[x] + 1 << ",";
			}
		}
		return ss.str().substr(0, ss.str().length() - 1);
	}

	virtual int get_type(const vector<string>& tokens,
			     vector<string>* type) {
		assert(type);
		type->clear();
		size_t cur = 0;
		string start = get_start(tokens, &cur);
		if (start.empty()) return -1;
		++cur;
		if (_var_to_type.count(start))
			*type = _var_to_type[start];

		while (cur < tokens.size()) {
			if (process_type(tokens, &cur, type)) return -1;
		}
		return 0;
	}

	virtual void tab_complete_impl(const vector<string>& tokens,
				       const string& last,
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
			vector<string> type;
			get_type(tokens, &type);
			_commands[command]->tab_complete(
			    arg, last, choices, hint, type);
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
	virtual int enter_complete(const string& cs, string* output) {

		string command = "";
		string error = "";
		string result = "";
		size_t pos = 0;

		if (process(cs + " done", &command, &result, &error, &pos)) {
			throw error;
		}

		Run run(command);
		run();
		if (result.empty()) {
			*output = run.read();
		} else {
			run.redirect(result);
		}
		return 0;
	}

	virtual string get_start(const string& command) {
	        vector<string> tokens;
		get_tokens(command, &tokens);
		return get_start(tokens);
	}

	virtual string get_start(const vector<string>& tokens) {
		size_t cur = 0;
		return get_start(tokens, &cur);
	}

	virtual string get_start(const vector<string>& tokens, size_t* cur) {
		if (tokens.empty()) return "";
		if (tokens.size() == 2 && tokens[1] == "=") return "";
		if (tokens.size() > 2 && tokens[1] == "=") {
			*cur = 2;
		}
		if (tokens[*cur] == "cat") ++*cur;
		return tokens[*cur];
	}

	virtual int process(const string& cs, string* output,
			    string* result, string* error,
			    size_t* parsed) {
		stringstream ss;
	        vector<string> tokens;
		get_tokens(cs, &tokens);
		vector<string> end_type = get_type(cs);
		size_t cur = 0;
		*parsed = 0;

		if (tokens.empty() || tokens[0] == "done") {
			*error = "where are the tokens, man?";
			return -1;
		}

		*result = "";
		if (tokens.size() > 2 && tokens[1] == "=") {
			cur = 2;
			*result = tokens[0];
			_var_to_type[*result] = end_type;
			_var_to_file[*result] = temp_filename(*result);
			*result = _var_to_file[*result];
		}
		if (tokens[cur] == "cat") ++cur;
		string start = tokens[cur++];
		assert(start == get_start(cs));
		*parsed = cur;
		if (_var_to_file.count(start) == 0) {
			if (!Fileutil::exists(start)) {
				*error = "dude, \"" + start + "\" isn't an actual file!";
				return -1;
			}
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
				if (pos.size() == 0) {
					*error = "filter needs a column";
					*parsed = cur;
					return -1;
				}
				if (pos.size() != 1) {
					*error = "filter only works for one col---for now!";
					*parsed = cur;
					return -1;
				}
                        	if (match == "in") {
					command << "| csv_grep " << *pos.begin()
					        << " " << word;
        	                } else if (match == "==") {
					command << "| csv_grep " << *pos.begin()
					        << " " << word << " exact ";
                        	} else {
					*error = "bad filter: either in or ==";
					*parsed = cur;
					return -1;
				}
	                } else if (op == "negate") {
	                } else if (op == "fill") {
				NEEDS(2);
				if (_var_to_file.count(tokens[cur]) == 0) {
					*error = "can't find " + tokens[cur]
						 + " in my file list.";
					*parsed = cur;
					return -1;
				}

				if (_var_to_file[tokens[cur]].empty()) {
					*error = "filename for " + tokens[cur]
						 + " is empty.";
					*parsed = cur;
					return -1;
				}
				if (!Fileutil::exists(_var_to_file[tokens[cur]])) {
					*error = "can't find a file at "
					    + _var_to_file[tokens[cur]];
					*parsed = cur;
					return -1;
				}

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
				// TODO translate tokens to columns
				command << "| cut -d, -f"
				        << typeset_column(get_type(tokens, cur),
							  tokens[cur]);
				++cur;
	                } else if (op == "filter_len") {
				NEEDS(3);
				command << "| csv_filter_len "
				        << tokens[cur]
					<< " "
					<< filter_len_op(tokens[cur + 1])
					<< " "
					<< tokens[cur + 2];
				cur += 3;
	                } else if (op == "type") {
				NEEDS(1);
				++cur;
	                } else if (op == "count") {
			} else if (op == "|") {
				continue;  // nop for console consistency
		        } else if (op == "done") {
				assert(cur == tokens.size());
			} else {
				*error = "hmm, " + op + " doesn't seem like anything I support";
				*parsed = cur;
				return -1;
			}
	        }

		if (result->empty()) {
			command << " | head -n 200";
		}

		*output = command.str();
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
		Command(const vector<string>& rule)
			: Command(rule, vector<string>()) {}
		Command(const vector<string>& rule,
			const vector<string>& type_list) {
			auto it = rule.begin();
			_name = *it++;
			set<string> types;
			types.insert("TYPE");
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
						if (x == "TYPE")
							_options.push_back(type_list);
						else
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
					  const string& last,
					  vector<string>* choices,
					  string* hint,
					  const vector<string>& type) const {
			assert(_args.size() == _hints.size());
			assert(_args.size() == _options.size());
			assert(arg < _args.size());
			*choices = _options.at(arg);
			if (_args.at(arg) == "COL" || _args.at(arg) == "COLS") {
				vector<string> pieces;
				Tokenizer::split(last, ",", &pieces);
				if (pieces.size()) pieces.pop_back();
				string prefix = Tokenizer::join(pieces, ",");
				if (!prefix.empty()) prefix += ",";
				for (auto &x: type) {
					choices->push_back(prefix + x);
					for (auto &y: type) {
						choices->push_back(prefix + x + "," + y);
					}
				}
			}
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
	map<string, vector<string>> _var_to_type;
	map<string, vector<string>> _types;

};

}  // namespace seasick

#endif  // __SEASICK__SEASICK__H__
