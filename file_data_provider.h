#ifndef __SEASICK__FILE_DATA_PROVIDER__H__
#define __SEASICK__FILE_DATA_PROVIDER__H__

#include "base_data_provider.h"

#include <fstream>
#include <set>
#include <string>

using namespace std;

namespace seasick {

class FileDataProvider : public BaseDataProvider {
public:
	FileDataProvider(const string& filename) {
		_filename = filename;
	}

	virtual ~FileDataProvider() {};

        virtual void init() {
                _fin.open(_filename);
                size_t i = 0;
                while (_fin.good()) {
                        string s;
                        getline(_fin, s);
                        if (s.empty() || _fin.eof()) break;
                        _breaks.push_back(i);
                        i += s.length() + 1;
                }
                _fin.clear();
	}

	virtual size_t rows() {
		return _breaks.size();
	}
	virtual string get(const size_t row) {
                assert(row < _breaks.size());
                _fin.clear();
                _fin.seekg(_breaks[row]);
                string retval;
                getline(_fin, retval);
                return retval;
	}

	virtual string get(const size_t col, const size_t row) {
                assert(row < _breaks.size());
                _fin.clear();
                _fin.seekg(_breaks[row]);
                string s, retval;
                getline(_fin, s);
		return cut(col, s);
	}

	virtual string get(const set<size_t>& cols, const size_t row) {
                assert(cols.size());
                _fin.seekg(_breaks[row]);
                _fin.clear();
                string s, retval;
                getline(_fin, s);
		return cut(cols, s);
        }

protected:
	string _filename;
	vector<size_t> _breaks;
	ifstream _fin;
};

}  // namespace seasick

#endif  // __SEASICK__FILE_DATA_PROVIDER__H__
