#ifndef __SEASICK__DATA_FRAME__H__
#define __SEASICK__DATA_FRAME__H__

#include <vector>

using namespace std;

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
		while (_fin,good()) {
			string s;
			getline(_fin, s);
			i += s.length() + 1;
			_breaks.push_back(i);
			_incl.push_back(true);
		}
	}

	virtual void negate() {
		for (size_t i = 0; i < _incl.size(); ++i) {
			_incl[i] = !_incl[i];
		}
	}

	virtual void filter(const string& word, bool exact, size_t col) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl(i)) continue;
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

protected:
	DataFrame(const DataFrame&) {}

	vector<bool> _incl;
	vector<size_t> _breaks;
	ifstream _fin;
};

}  // namespace seasick

#endif  // __SEASICK__DATA_FRAME__H__
