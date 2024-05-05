#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <cstdlib>
#include <cstdint>

void error(const char* error) {
	std::cerr << error;
}

struct matrix {
private:
	size_t rows_;
	size_t cols_;
	std::vector<uint8_t> data_;

public:
	matrix(size_t h, size_t w) : rows_(h), cols_(w), data_(h* w) {}
	
	uint8_t& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}

	const uint8_t& operator()(size_t r, size_t c) const {
		return data_[r * cols_ + c];
	}

	auto rowdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rows() const {
		return rows_;
	}

	size_t cols() const {
		return cols_;
	}
};




auto encode(const char* inputfile, const char* outputfile) {
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		error("Error while opening file for input");
		return EXIT_FAILURE;
	}

	std::string hfield(8, ' ');
	is >> hfield;
	if (hfield != "P5")
	{
		return EXIT_FAILURE;
	}
	size_t width = 0;
	
	is >> hfield;
	char c = 0;
	if (hfield == "#")
	{
		while (c != '\n')
		{
			is >> hfield;
			c = is.get();
		}
	}

	
	is >> width;
	size_t height = 0;
	is >> height;
	size_t maxval = 0;
	is >> maxval;

	is.get(); // removing newline

	matrix img(height, width);

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = is.get();
		}
	}

	std::vector<uint8_t> pattern0 = { 1, 6, 4, 6, 2, 6, 4, 6 };
	std::vector<uint8_t> pattern2 = { 5, 6, 5, 6, 5, 6, 5, 6 };
	std::vector<uint8_t> pattern4 = { 3, 6, 4, 6, 3, 6, 4, 6 };

	std::vector<uint8_t>p1;
	std::vector<uint8_t>p2;
	std::vector<uint8_t>p3;
	std::vector<uint8_t>p4;
	std::vector<uint8_t>p5;
	std::vector<uint8_t>p6;
	std::vector<uint8_t>p7;

	for (size_t r = 0; r < img.rows(); r++)
	{
		auto sel_r = r % 8;
		for (size_t c = 0; c < img.cols(); c++)
		{
			if ((r % 2) == 1)
			{
				p7.push_back(img(r, c));
			}
			else
			{
				if (sel_r == 0)
				{
					auto sel = c % 8;
					if (sel == 0)
					{
						p1.push_back(img(r, c));
					}
					else if ((sel == 2) || (sel == 6))
					{
						p4.push_back(img(r, c));
					}
					else if((sel == 4))
					{
						p2.push_back(img(r, c));
					}
					else
					{
						p6.push_back(img(r, c));
					}
				}
				else if ((sel_r == 2) || (sel_r == 6))
				{
					auto sel = c % 2;
					if (sel == 0)
					{
						p5.push_back(img(r, c));
					}
					else
					{
						p6.push_back(img(r, c));
					}
				}
				else
				{
					auto sel = c % 8;
					if ((sel == 0) || (sel == 4))
					{
						p3.push_back(img(r, c));
					}
					else if ((sel == 2) || (sel == 6))
					{
						p4.push_back(img(r, c));
					}
					else
					{
						p6.push_back(img(r, c));
					}
				}
			}
		}
	}
	
	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while openig output file");
		return EXIT_FAILURE;
	}

	os.write("MULTIRES", 8);
	uint32_t w = img.cols();
	uint32_t h = img.rows();

	os.write(reinterpret_cast<char*>(&w), 4);
	os.write(reinterpret_cast<char*>(&h), 4);

	for (const auto& elem : p1) {
		os.put(elem);
	}
	for (const auto& elem : p2) {
		os.put(elem);
	}
	for (const auto& elem : p3) {
		os.put(elem);
	}
	for (const auto& elem : p4) {
		os.put(elem);
	}
	for (const auto& elem : p5) {
		os.put(elem);
	}
	for (const auto& elem : p6) {
		os.put(elem);
	}
	for (const auto& elem : p7) {
		os.put(elem);
	}
	return EXIT_SUCCESS;
}

auto savepgm(matrix& img, const char* out) {
	
	std::ofstream os(out, std::ios::binary);

	os << "P5\n";
	os << img.cols() << '\n';
	os << img.rows() << '\n';
	os << "255" << '\n';

	os.write(img.rowdata(), img.rows() * img.cols());

	return EXIT_SUCCESS;

}




auto decode(const char* inputfile, const char* outputprefix) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		error("Error while opening input file for decoding\n");
		return EXIT_FAILURE;
	}

	std::string magicnumber(8, ' ');
	is.read(const_cast<char *>(magicnumber.c_str()), 8);
	if (magicnumber != "MULTIRES")
	{
		error("This program only support MULTIRES format\n");
		return EXIT_FAILURE;
	}

	uint32_t width, height;

	is.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
	is.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));

	std::vector<uint8_t>pix(width * height);

	is.read(reinterpret_cast<char*>(pix.data()), width * height);

	matrix img(height, width);

	int np1 = 0;
	int np2 = 0;
	int np3 = 0;
	int np4 = 0;
	int np5 = 0;
	int np6 = 0;
	int np7 = 0;

	for (size_t r = 0; r < img.rows(); r++)
	{
		auto sel_r = r % 8;
		for (size_t c = 0; c < img.cols(); c++)
		{
			if ((r % 2) == 1)
			{
				++np7;
			}
			else
			{
				if (sel_r == 0)
				{
					auto sel = c % 8;
					if (sel == 0)
					{
						++np1;
					}
					else if ((sel == 2) || (sel == 6))
					{
						++np4;
					}
					else if ((sel == 4))
					{
						++np2;
					}
					else
					{
						++np6;
					}
				}
				else if ((sel_r == 2) || (sel_r == 6))
				{
					auto sel = c % 2;
					if (sel == 0)
					{
						++np5;
					}
					else
					{
						++np6;
					}
				}
				else
				{
					auto sel = c % 8;
					if ((sel == 0) || (sel == 4))
					{
						++np3;
					}
					else if ((sel == 2) || (sel == 6))
					{
						++np4;
					}
					else
					{
						++np6;
					}
				}
			}
		}
	}

	std::vector<std::vector<uint8_t>> pattern({

		{1, 6, 4, 6, 2, 6, 4, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{5, 6, 5, 6, 5, 6, 5, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{3, 6, 4, 6, 3, 6, 4, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},
		{5, 6, 5, 6, 5, 6, 5, 6},
		{7, 7, 7, 7, 7, 7, 7, 7},

		});

	std::vector<uint64_t>index({ 0, 0, 0, 0, 0, 0, 0 });
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			switch (pattern[r % 8][c % 8]) {
			case 1:
				img(r, c) = pix[index[0]];
				++index[0];
				break;
			case 2:
				img(r, c) = pix[index[1] + np1];
				++index[1];
				break;
			case 3:
				img(r, c) = pix[index[2] + np1 + np2];
				++index[2];
				break;
			case 4:
				img(r, c) = pix[index[3] + np1 + np2 + np3];
				++index[3];
				break;
			case 5:
				img(r, c) = pix[index[4] + np1 + np2 + np3 + np4];
				++index[4];
				break;
			case 6:
				img(r, c) = pix[index[5] + np1 + np2 + np3 + np4 + np5];
				++index[5];
				break;
			case 7:
				img(r, c) = pix[index[6] + np1 + np2 + np3 + np4 + np5 + np6];
				++index[6];
				break;
			default:
				break;
			}
		}
	}

	//savepgm(img, "rec_7.pgm");


	matrix img_lvl(height, width);

	for (size_t l = 1; l < 8; l++)
	{
		for (size_t r = 0; r < img.rows(); r++)
		{
			for (size_t c = 0; c < img.cols(); c++)
			{
				if (pattern[r % 8][c % 8] <= l)
				{
					img_lvl(r, c) = img(r, c);
				}
				else
				{
					switch (pattern[r % 8][c % 8])
					{
					case 2:
						img_lvl(r, c) = img_lvl(r, c - 4);
						break;
					case 3:
						img_lvl(r, c) = img_lvl(r - 4, c);
						break;
					case 4: 
						img_lvl(r, c) = img_lvl(r, c - 2);
						break;
					case 5:
						img_lvl(r, c) = img_lvl(r- 2, c);
						break;
					case 6:
						img_lvl(r, c) = img_lvl(r, c - 1);
						break;
					case 7:
						img_lvl(r, c) = img_lvl(r - 1, c);
						break;
					default:
						break;
					}
				}
			}
		}
		std::string out(outputprefix);
		out.append("_");
		out.append(std::to_string(l));
		out.append(".pgm");

		savepgm(img_lvl, out.c_str());
		//salva
	}

	return 0;
}




int main(int argc, char** argv) {
	if (argc != 4)
	{
		error("Usage is [c|d] inputfile outpufile\n");
		return EXIT_FAILURE;
	}

	std::string mode = argv[1];

	if (mode == "c")
	{
		encode(argv[2], argv[3]);
	}
	else if(mode == "d")
	{
		decode(argv[2], argv[3]);
	}

	return 0;
}