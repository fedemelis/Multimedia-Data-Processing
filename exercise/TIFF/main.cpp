#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <string>
#include <set>
#include <unordered_map>
#include <array>
#include <format>

struct IDF
{
	uint16_t tag;
	uint16_t type;
	uint32_t count = 0;
	uint32_t value = 0;
};

template <typename T>
struct mat {

	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}

	const T& operator()(size_t row, size_t col) const{
		return data_[row * c_ + col];
	}

	size_t rows() {
		return r_;
	}

	size_t cols() {
		return c_;
	}

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	auto rawread() {
		return reinterpret_cast<char*>(data_.data());
	}
};

auto decode(const char* inputfile, const char* outputfile) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return 2;
	}

	std::string magicnumber(2, ' ');
	is.read(reinterpret_cast<char*>(magicnumber.data()), 2);

	if (magicnumber != "II")
	{
		return 3;
	}

	is.ignore(2); // skipping 42

	uint32_t offset = 0;
	is.read(reinterpret_cast<char*>(&offset), 4);

	uint16_t nEntry = 0;
	is.read(reinterpret_cast<char*>(&nEntry), 2);

	std::set<uint16_t> fields = { 256, 257, 278, 259, 262, 279, 258, 273 };
	std::unordered_map<uint16_t, uint32_t> values;

	int skip = 0;

	for (size_t i = 0; i < nEntry; i++)
	{
		IDF f;
		is.read(reinterpret_cast<char*>(&f.tag), 2);
		is.read(reinterpret_cast<char*>(&f.type), 2);
		is.read(reinterpret_cast<char*>(&f.count), 4);
		if (fields.find(f.tag) != fields.end())
		{

			if (f.type == 1) // uint8
			{
				is.read(reinterpret_cast<char*>(&f.value), 1);
				is.ignore(3);
			}
			else if (f.type == 2) {
				is.read(reinterpret_cast<char*>(&f.value), 1);
				is.ignore(3);
			}
			else if (f.type == 3) {
				is.read(reinterpret_cast<char*>(&f.value), 2);
				is.ignore(2);
			}
			else if (f.type == 4) {
				is.read(reinterpret_cast<char*>(&f.value), 4);
			}
			values[f.tag] = f.value;
		}
		else
		{
			if (f.type == 3 || f.type == 2)
			{
				f.type -= 1;
			}
			is.ignore(4);
			if (f.count > 1)
			{
				skip += f.type * f.count;
			}
		}
	}
	is.seekg(values[273]);
	//is.ignore(skip);

	auto x = is.peek();
	std::cout << "test";

	mat<uint8_t> img(values[257], values[256]);

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = is.get();	
		}
	}

	std::ofstream os(outputfile, std::ios::binary);

	std::string head = std::format(
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 1\n"
		"MAXVAL 255\n"
		"TUPLTYPE GRAYSCALE\n"
		"ENDHDR\n"
	, img.cols(), img.rows());

	os << head;
	os.write(img.rawread(), img.dim());

	return 0;
}


int main(int argc, char** argv) {
	
	if (argc != 3)
	{
		return 1;
	}

	decode(argv[1], argv[2]);

	return 0;

}