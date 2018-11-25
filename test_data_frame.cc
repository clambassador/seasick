#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

#include "data_frame.h"

using namespace std;
using namespace ib; 

int main(int argc, char** argv) {
	if (argc < 4) {
		Logger::error("usage: % datafile commands...", argv[0]);
		return -1;
	}

	seasick::DataFrame df(argv[1]);
	return 0;
}
