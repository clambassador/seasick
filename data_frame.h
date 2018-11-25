#ifndef __SEASICK__DATA_FRAME__H__
#define __SEASICK__DATA_FRAME__H__

#include "ib/logger.h"
#include "ib/tokenizer.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace ib;

namespace seasick {

class DataFrame {
public:
	DataFrame() {}
	DataFrame(const string& csv_file) {
		init(csv_file);
	}
	virtual ~DataFrame() {}

	virtual void init(const string& csv_file) {
		_fin.open(csv_file);
		size_t i = 0;
		while (_fin.good()) {
			string s;
			getline(_fin, s);
			if (s.empty() || _fin.eof()) break;
			_breaks.push_back(i);
			i += s.length() + 1;
			_incl.push_back(true);
		}
		_fin.clear();
	}

	virtual string get(size_t row) {
		assert(row < _breaks.size());
		_fin.clear();
		_fin.seekg(_breaks[row]);
		string retval;
		getline(_fin, retval);
		return retval;
	}

	virtual string get(size_t col, size_t row) {
		assert(row < _breaks.size());
		_fin.clear();
		_fin.seekg(_breaks[row]);
		string s, retval;
		getline(_fin, s);
		Tokenizer::fast_split(s, ',', col, &retval);
		return retval;
	}

	virtual string get(const set<size_t>& cols, size_t row) {
		assert(cols.size());
		_fin.seekg(_breaks[row]);
		_fin.clear();
		string s, retval;
		getline(_fin, s);
		stringstream ss;
		for (auto &x : cols) {
			Tokenizer::fast_split(s, ',', x, &retval);
			ss << retval << ',';
		}
		return ss.str().substr(0, ss.str().length() - 1);
	}

	virtual void negate() {
		for (size_t i = 0; i < _incl.size(); ++i) {
			_incl[i] = !_incl[i];
		}
	}

	virtual void filter(const string& word, bool exact, size_t col) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			if (exact) {
				if (word != get(col, i)) {
					_incl[i] = 0;
				}
			} else {
				if (get(col, i).find(word) ==
				    string::npos) {
					_incl[i] = 0;
				}
			}
		}
	}

	virtual void project(const set<size_t>& cols, set<string>* out) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			out->insert(get(cols, i));
		}
	}

	virtual void project(const set<size_t>& cols, vector<string>* out) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			out->push_back(get(cols, i));
		}
	}

	virtual void project(const set<size_t>& cols, map<string, int>* out) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			(*out)[get(cols, i)]++;
		}
	}

	virtual void fill(const set<size_t>& cols) {
		set<string> matching;
		project(cols, &matching);
		filter(cols, matching);
	}

	virtual void filter(const set<size_t>& cols, const set<string>& vals) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (vals.count(get(cols, i))) {
				_incl[i] = true;
			} else {
				_incl[i] = false;
			}
		}
	}

	virtual void all_on() {
		all(true);
	}

	virtual void all_off() {
		all(false);
	}

	virtual void all(bool val) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			_incl[i] = val;
		}
	}

	virtual void trace() {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (_incl[i]) cout << get(i) << endl;
		}
	}

protected:
	DataFrame(const DataFrame&) {}

	vector<bool> _incl;
	vector<size_t> _breaks;
	ifstream _fin;
};

}  // namespace seasick

#endif  // __SEASICK__DATA_FRAME__H__
