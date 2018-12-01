#ifndef __SEASICK__MEMORY_DATA_PROVIDER__H__
#define __SEASICK__MEMORY_DATA_PROVIDER__H__

#include <set>
#include <string>

#include "base_data_provider.h"
#include "ib/fileutil.h"

using namespace std;
using namespace ib;

namespace seasick {

class MemoryDataProvider : public BaseDataProvider {
public:
	MemoryDataProvider() {
	}

	MemoryDataProvider(const map<string, size_t>& data) {
		clear();
		process(data);
	}

	MemoryDataProvider(const set<string>& data) {
		clear();
		process(data);
	}

	MemoryDataProvider(const vector<string>& data) {
		clear();
		process(data);
	}

	virtual ~MemoryDataProvider() {};

	virtual void add_row(const string& row) {
		_rows.push_back(row);
	}

	virtual void clear() {
		_rows.clear();
	}

        virtual void init() {
        }

	virtual size_t rows() {
		return _rows.size();
	}
	virtual string get(const size_t row) {
		assert(row < _rows.size());
                return _rows[row];
	}

	virtual string get(const size_t col, const size_t row) {
		assert(row < _rows.size());
		return cut(col, _rows[row]);
	}

	virtual string get(const set<size_t>& cols, const size_t row) {
		assert(row < _rows.size());
                assert(cols.size());
		return cut(cols, _rows[row]);
        }

protected:
	vector<string> _rows;
};

}  // namespace seasick

#endif  // __SEASICK__MEMORY_DATA_PROVIDER__H__
