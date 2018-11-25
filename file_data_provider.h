#ifndef __SEASICK__FILE_DATA_PROVIDER__H__
#define __SEASICK__FILE_DATA_PROVIDER__H__

#include <fstream>
#include <set>
#include <string>

#include "base_data_provider.h"
#include "ib/fileutil.h"

using namespace std;
using namespace ib;

namespace seasick {

class FileDataProvider : public BaseDataProvider {
public:
	FileDataProvider(const string& filename) {
		_filename = filename;
	}

	virtual ~FileDataProvider() {};

        virtual void init() {
                _fin.open(_filename, ios::in | ios::binary);
                bool parsed = false;
                if (Fileutil::exists(_filename + ".h")) {
                        ifstream fin(_filename + ".h");
                        while (fin.good()) {
                                string s;
                                getline(fin, s);
                                if (s == "---") {
                                        parsed = true;
                                        break;
                                }
                                if (s.empty() || _fin.eof()) break;
                                if (_breaks.size()) assert(atoi(s.c_str()) > _breaks.back());
                                _breaks.push_back(atoi(s.c_str()));
                        }
                }

                if (!parsed) {
                        size_t i = 0;
                        ofstream fout(_filename + ".h");
                        while (_fin.good()) {
                                string s;
                                getline(_fin, s);
                                if (_fin.eof()) break;
                                if (s.empty()) continue;
                                _breaks.push_back(i);
                                fout << i << endl;
                                i += s.length() + 1;
                        }
                        fout << "---" << endl;
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
