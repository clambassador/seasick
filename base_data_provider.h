#ifndef __SEASICK__BASE_DATA_PROVIDER__H__
#define __SEASICK__BASE_DATA_PROVIDER__H__

#include <set>
#include <sstream>
#include <string>

#include "abstract_data_provider.h"
#include "ib/tokenizer.h"

using namespace ib;
using namespace std;

namespace seasick {

class BaseDataProvider : public AbstractDataProvider {
public:
	virtual ~BaseDataProvider() {};
	virtual void init() {
		assert(0);
	}
	virtual size_t rows() {
		assert(0);
	}
	virtual string get(const size_t row) {
		assert(0);
	}
	virtual string get(const size_t col, const size_t row) {
		assert(0);
	}
	virtual string get(const set<size_t>& cols, const size_t row) {
		assert(0);
	}

	virtual string cut(const size_t col, const string& str) {
		string retval;

                Tokenizer::fast_split(str, ',', col, &retval);
		return retval;
	}

	virtual string cut(const set<size_t>& cols, const string& str) {
		string retval;
		stringstream ss;

		// TODO: efficiency
		for (const auto &x : cols) {
			Tokenizer::fast_split(str, ',', x, &retval);
			ss << retval << ",";
		}
		return ss.str().substr(0, ss.str().length() - 1);
	}

};

}  // namespace seasick

#endif  // __SEASICK__BASE_DATA_PROVIDER__H__
