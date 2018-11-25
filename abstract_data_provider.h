#ifndef __SEASICK__ABSTRACT_DATA_PROVIDER__H__
#define __SEASICK__ABSTRACT_DATA_PROVIDER__H__

#include <set>

using namespace std;

namespace seasick {

class AbstractDataProvider {
public:
	virtual ~AbstractDataProvider() {};
	virtual void init() = 0;
	virtual size_t rows() = 0;
	virtual string get(const size_t row) = 0;
	virtual string get(const size_t col, const size_t row) = 0;
	virtual string get(const set<size_t>& cols, const size_t row) = 0;
};

}  // namespace seasick

#endif  // __SEASICK__ABSTRACT_DATA_PROVIDER__H__
