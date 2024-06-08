#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>

int error(const char* errmsg) {
	std::cerr << errmsg << std::endl;
	return EXIT_FAILURE;
}

struct head
{
	std::string magicnumber;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t maxval = 0;
};

template <typename T>
struct mat
{
	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	size_t rows() {
		return r_;
	}

	size_t cols() {
		return c_;
	}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}

	const T& operator()(size_t row, size_t col) const{
		return data_[row * c_ + col];
	}

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

};

auto readPMGhead(head& h, std::ifstream& is) {

	h.magicnumber.resize(2);
	is >> h.magicnumber;

	if (h.magicnumber != "P5")
	{
		return error("Magic number should have been P5");
	}
	is.ignore(1);

	if (is.peek() == '#')
	{
		std::string trash;
		while (is.peek() != '\n')
		{
			is >> trash;
		}
	}
	
	is >> h.width;
	is >> h.height;
	is >> h.maxval;
	is.ignore(1);

	return EXIT_SUCCESS;

}

auto saveMLT(mat<uint8_t>& img, std::string& outputfile, head& h, std::vector<std::vector<uint8_t>>& pixlvl) {
	
	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file for encoding");
	}

	std::string head = "MULTIRES";
	os << head;
	os.write(reinterpret_cast<char*>(&h.width), 4);
	os.write(reinterpret_cast<char*>(&h.height), 4);
	
	for (size_t i = 1; i < 8; i++)
	{
		os.write(reinterpret_cast<char*>(pixlvl[i].data()), pixlvl[i].size());
	}

	return EXIT_SUCCESS;
}

auto encode(std::string& inputfile, std::string& outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening input file for encoding");
	}

	head h;
	readPMGhead(h, is);

	mat<uint8_t> img(h.height, h.width);

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = is.get();
		}
	}

	std::vector<std::vector<uint8_t>> map = {
		{1, 6, 4, 6, 2, 6, 4, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{5, 6, 5, 6, 5, 6, 5, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{3, 6, 4, 6, 3, 6, 4, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{5, 6, 5, 6, 5, 6, 5, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
	};

	std::vector<std::vector<uint8_t>> pixlvl;
	pixlvl.resize(8);

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c= 0; c < img.cols(); c++)
		{
			uint8_t lvl = map[r%8][c%8];
			pixlvl[lvl].push_back(img(r, c));
		}
	}

	saveMLT(img, outputfile, h, pixlvl);
	return EXIT_SUCCESS;
}

auto savePGM(std::string& outputfile, mat<uint8_t>& img) {
	
	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file for decoding");
	}

	std::string head = std::format(
		"P5 "
		"{} "
		"{} "
		"255\n"
	, img.cols(), img.rows());

	os << head;
	os.write(img.rawdata(), img.dim());
	return EXIT_SUCCESS;
}


auto decode(const std::string& inputfile, std::string& outputprefix) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening file for decoding");
	}

	std::string magicnumber(8, ' ');
	is.read(reinterpret_cast<char*>(magicnumber.data()), 8);
	if (magicnumber != "MULTIRES")
	{
		return error("Magic number should be \"MULTIRES\"");
	}
	uint32_t width = 0, height = 0;

	is.read(reinterpret_cast<char*>(&width), 4);
	is.read(reinterpret_cast<char*>(&height), 4);

	std::vector<std::vector<uint8_t>> map = {
		{1, 6, 4, 6, 2, 6, 4, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{5, 6, 5, 6, 5, 6, 5, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{3, 6, 4, 6, 3, 6, 4, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{5, 6, 5, 6, 5, 6, 5, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
	};

	std::vector<std::vector<uint8_t>> pixlvl;
	pixlvl.resize(8);

	std::array<uint64_t, 8> countlvl = { {0} };

	for (size_t r = 0; r < height; r++)
	{
		for (size_t c = 0; c < width; c++)
		{
			uint8_t lvl = map[r % 8][c % 8];
			++countlvl[lvl];
		}
	}

	int ii = 0;
	for (const auto& elem : countlvl) 
	{
		if (elem != 0)
		{
			for (size_t i = 0; i < elem; i++)
			{
				pixlvl[ii].push_back(is.get());
			}
		}	
		++ii;
	}

	mat<uint8_t> img7(height, width);
	std::array<uint64_t, 8> indexes = { {0} };

	for (size_t r = 0; r < img7.rows(); r++)
	{
		for (size_t c = 0; c < img7.cols(); c++)
		{
			uint8_t lvl = map[r % 8][c % 8];
			auto index = indexes[lvl];
			img7(r, c) = pixlvl[lvl][index];
			++indexes[lvl];
		}
	}

	std::string out;
	out = outputprefix + "_7.pgm";
	savePGM(out, img7);

	indexes = { 0, 0, 0, 0, 0, 0, 0, 0 };
	for (size_t l = 1; l <= 6; l++)
	{
		mat<uint8_t>img(height, width);
		for (size_t r = 0; r < img.rows(); r++)
		{
			for (size_t c = 0; c < img.cols(); c++)
			{
				uint8_t lvl = map[r % 8][c % 8];
				if (lvl > l)
				{
					if (lvl == 7)
					{
						img(r, c) = img(r - 1, c);
					}
					else if (lvl == 6)
					{
						img(r, c) = img(r, c - 1);
					}
					else if (lvl == 5) {
						img(r, c) = img(r - 2, c);
					}
					else if (lvl == 4) {
						img(r, c) = img(r, c - 2);
					}
					else if (lvl == 3) {
						img(r, c) = img(r - 4, c);
					}
					else if (lvl == 2) {
						img(r, c) = img(r, c - 4);
					}
				}
				else
				{
					auto index = indexes[lvl];
					img(r, c) = pixlvl[lvl][index];
					++indexes[lvl];
				}
			}
		}
		std::string outp;
		outp = outputprefix + '_' + std::to_string(l)+ ".pgm";
		savePGM(outp, img);
		indexes = { 0, 0, 0, 0, 0, 0, 0, 0 };
	}

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	
	if (argc != 4)
	{
		return error("Usage is: multires c <input file .PGM> <output file .MLT>\nor\nmultires d <input file .MLT> <output prefix>");
	}

	std::string argv2(argv[2]);
	std::string argv3(argv[3]);
	if (argv[1][0] == 'c')
	{
		return encode(argv2, argv3);
	}
	else
	{
		return decode(argv2, argv3);
	}
}