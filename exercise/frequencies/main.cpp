#include <vector>
#include <set>
#include <fstream>
#include <map>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <chrono>

struct freq {
public:
	uint32_t count[256] = { 0 };
	void increment(const unsigned char& c){
		++count[c];
	}

};


int main(int argc, char *argv[]) {

	using std::vector;
	using namespace std::chrono;

	
	auto start = steady_clock::now();


	if (argc != 3)
	{
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1]/*, std::ios::binary*/);
	if (!is)
	{
		return EXIT_FAILURE;
	}
	//vector<char> v{ std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };

	//vado a 0 byte dalla fine
	is.seekg(0, std::ios::end);

	auto filesize = is.tellg();
	is.seekg(std::ios::beg);

	vector<char> v(filesize);

	is.read(v.data(), filesize);

	freq stats;

	for (char& c : v) {
		stats.increment(static_cast<unsigned char>(c));
	}
	
	std::ofstream os(argv[2]/* ,std::ios::binary*/);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < 256; ++i)
	{
		if (stats.count[i] > 0)
		{
			os << std::hex << std::setw(2) << std::setfill('0') << i;
			os << std::dec << '\t' << stats.count[i] << '\n';
		}
	}

	auto stop = steady_clock::now();
	auto diff = stop - start;
	auto duration_ms = duration<double, std::milli>(diff);

	std::cout << "tempo di esecuzione: " << duration_ms << '\n';

	return 0;




}