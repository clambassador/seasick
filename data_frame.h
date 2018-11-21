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
		_breaks.push_back(i);
		_incl.push_back(true);
		while (_fin.good()) {
			string s;
			getline(_fin, s);
			i += s.length() + 1;
			_breaks.push_back(i);
			Logger::info("% %", _breaks.size(), i);
			_incl.push_back(true);
		}
		_fin.clear();
	}

	virtual string get(size_t col, size_t row) {
		assert(row < _breaks.size());
		_fin.seekg(_breaks[row]);
		string s, retval;
		getline(_fin, s);
		cout << "seek " << col << " " << row << " " << s << endl;
		Tokenizer::fast_split(s, ',', col, &retval);
		return retval;
	}

	virtual string get(const set<size_t>& cols, size_t row) {
		assert(cols.size());
		_fin.seekg(_breaks[row]);
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

	virtual void grep(const string& word, bool exact, size_t col) {
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

	virtual void expand(const set<size_t>& cols) {
		set<string> matching;
		project(cols, &matching);
		filter(cols, matching);
	}

	virtual void filter(const set<size_t>& cols, const set<string> vals) {
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

protected:
	DataFrame(const DataFrame&) {}

	vector<bool> _incl;
	vector<size_t> _breaks;
	ifstream _fin;
};

}  // namespace seasick

#endif  // __SEASICK__DATA_FRAME__H__
