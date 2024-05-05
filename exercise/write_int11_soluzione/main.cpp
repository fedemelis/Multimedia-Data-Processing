#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <array>
#include <chrono>
#include <format>
#include <vector>
#include <iterator>
#include <algorithm>

void error(const std::string& message) {
	std::cout << message << '\n';
	exit(EXIT_FAILURE);
}		

void syntax() {
	error(
		"SYNTAX:\n"
		"write_int32 <filein.txt> <fileout.bin>\n"
	);
}

template<typename T>
void check_open(const T& stream, const std::string& filename) {
	if (!stream) {
		error(std::string("Cannot open file ") + filename);
	}
}

class bitwriter {
	uint8_t buffer_ = 0;
	size_t n_ = 0; // 0-8
	std::ostream& os_;

	// this is the single bit inserting and eventually writing of byte
	// curbit have on the right the bit that we want
	void writebit(uint64_t curbit) {
		// preparo il buffer per il prossimo bit
		buffer_ <<= 1;
		buffer_ |= (curbit & 1);
		/*buffer_ = (buffer_ << 1) | (curbit & 1);*/
		++n_;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
	}

public:
	bitwriter(std::ostream& os) : os_(os) {}

	~bitwriter() {
		fillbuf();
	}


	// bw(number, 11) for example
	// we can pass anything as elem, because in fact the binary representation of elem doesn't change
	std::ostream& operator()(uint64_t elem, int numbits) {
		for (int i = numbits - 1; i >= 0; --i) {
			writebit(elem >> i);
		}
		return os_;
	}

	// it fills with zero the buffer using "writebit"
	std::ostream& fillbuf(int padbit = 0) {
		while (n_ > 0) {
			writebit(padbit);
		}
		return os_;
	}

	std::ostream& get_stream() {
		return os_;
	}
};

int main(int argc, char* argv[])
{
	using namespace std;
	using namespace std::chrono;

	auto start = steady_clock::now();

	if (argc != 3) {
		syntax();
	}

	std::ifstream is(argv[1]/*, std::ios::binary*/);
	check_open(is, argv[1]);

	vector<int> v{ istream_iterator<int>(is), istream_iterator<int>() };
	/*vector<int> v;
	v = { -64, -64, 510, 15, -128, 1020, 31, -256, };*/

	std::ofstream os(argv[2], std::ios::binary);
	check_open(os, argv[2]);

	bitwriter writer(os);
	for (const auto& elem : v) {
		writer(elem, 11);
	}

	auto end = steady_clock::now();
	auto diff = end - start;

	auto duration_ms = duration<double, std::milli>(diff);

	std::cout << "Tempo di esecuzione: " << duration_ms << '\n';


	return EXIT_SUCCESS;
}