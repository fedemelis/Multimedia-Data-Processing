#include <vector>
//#include <set>
#include <fstream>
//#include <map>
#include <iomanip>
#include <bitset>
#include <iostream>
#include <sstream>
//#include <algorithm>
#include <chrono>

int main(int argc, char* argv[]) {

	using std::vector;


	if (argc != 3)
	{
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1]/*, std::ios::binary*/);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	vector<int> v;
	int val;
	while (is >> val)
	{
		v.push_back(val);
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	for (int& elem : v) {
		os.write(reinterpret_cast<char*>(&elem), sizeof(int));
	}

	return 0;
}