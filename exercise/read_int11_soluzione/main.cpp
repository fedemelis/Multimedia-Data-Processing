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
		"read_int11 <filein.bin> <fileout.txt>\n"
	);
}

template<typename T>
void check_open(const T& stream, const std::string& filename) {
	if (!stream) {
		error(std::string("Cannot open file ") + filename);
	}
}

class bitreader {
	uint8_t buffer_;
	size_t n_ = 0; // 0-8
	std::istream& is_;

	// if i have 0 bit in the buffer i read a byte, then decement n_ and return the buffer also shifted and also anded
	uint64_t readbit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	/*void readbit() {
		if (n_ == 0)
		{
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		buffer_ >>= n_;
	}*/

public:
	bitreader(std::istream& is) : is_(is) {}

	uint64_t operator()(int numbits) {
		uint64_t valbuf = 0;
		for (int i = numbits - 1; i >= 0; --i) {
			uint64_t curbit = readbit();
			valbuf <<= 1;
			valbuf |= curbit;
			/*readbit();
			valbuf <<= 1;
			valbuf |= buffer_ & 1;*/
		}
		return valbuf;
	}

	bool fail() const {
		return is_.fail();
	}

	operator bool() const {
		return !fail();
	}

	std::istream& get_stream() {
		return is_;
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

	std::ifstream is(argv[1], std::ios::binary);
	check_open(is, argv[1]);

	bitreader reader(is);

	vector<int> v;
	while (true) {
		int num = static_cast<int>(reader(11));
		if (!reader.get_stream())
		{
			break;
		}
		if (num >= 1024) {
			num = num - 2048;
		}
		v.push_back(num);
	}

	std::ofstream os(argv[2]/*, std::ios::binary*/);
	check_open(os, argv[2]);

	std::ranges::copy(v, ostream_iterator<int>(os, "\n"));

	auto end = steady_clock::now();
	auto diff = end - start;

	auto duration_ms = duration<double, std::milli>(diff);

	std::cout << "Tempo di esecuzione: " << duration_ms << '\n';


	return EXIT_SUCCESS;
}