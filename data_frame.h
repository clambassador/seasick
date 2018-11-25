#ifndef __SEASICK__DATA_FRAME__H__
#define __SEASICK__DATA_FRAME__H__

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ib/logger.h"
#include "ib/tokenizer.h"
#include "abstract_data_provider.h"
#include "file_data_provider.h"
#include "memory_data_provider.h"

using namespace std;
using namespace ib;

namespace seasick {

class DataFrame {
public:
	DataFrame() {}
	DataFrame(const map<string, size_t>& data) {
		init(data);
	}
	DataFrame(const set<string>& data) {
		init(data);
	}
	DataFrame(const vector<string>& data) {
		init(data);
	}
	DataFrame(const string& csv_file) {
		_adp.reset(new FileDataProvider(csv_file));
		_adp->init();
		_incl.resize(_adp->rows(), true);
	}
	virtual ~DataFrame() {}

	virtual void init(const map<string, size_t>& data) {
		clear();
		_adp.reset(new MemoryDataProvider(data));
		_adp->init();
		_incl.resize(_adp->rows(), true);
	}
	virtual void init(const set<string>& data) {
		clear();
		_adp.reset(new MemoryDataProvider(data));
		_adp->init();
		_incl.resize(_adp->rows(), true);
	}
	virtual void init(const vector<string>& data) {
		clear();
		_adp.reset(new MemoryDataProvider(data));
		_adp->init();
		_incl.resize(_adp->rows(), true);
	}

	virtual void clear() {
		_incl.clear();
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
				if (word != _adp->get(col, i)) {
					_incl[i] = 0;
				}
			} else {
				if (_adp->get(col, i).find(word) ==
				    string::npos) {
					_incl[i] = 0;
				}
			}
		}
	}

	virtual void project(const set<size_t>& cols, set<string>* out) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			out->insert(_adp->get(cols, i));
		}
	}

	virtual void project(const set<size_t>& cols, vector<string>* out) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			out->push_back(_adp->get(cols, i));
		}
	}

	virtual void project(const set<size_t>& cols, map<string, size_t>* out) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (!_incl[i]) continue;
			(*out)[_adp->get(cols, i)]++;
		}
	}

	virtual void fill(const set<size_t>& cols) {
		set<string> matching;
		project(cols, &matching);
		filter(cols, matching);
	}

	virtual void filter(const set<size_t>& cols, const set<string>& vals) {
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (vals.count(_adp->get(cols, i))) {
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
			if (_incl[i]) cout << _adp->get(i) << endl;
		}
	}

	virtual void save(ostream& out) {
		assert(_incl.size() == _adp->rows());
		for (size_t i = 0; i < _incl.size(); ++i) {
			if (_incl[i]) out << _adp->get(i) << endl;
		}
	}


protected:
	DataFrame(const DataFrame&) {}

	vector<bool> _incl;
	unique_ptr<AbstractDataProvider> _adp;
};

}  // namespace seasick

#endif  // __SEASICK__DATA_FRAME__H__
