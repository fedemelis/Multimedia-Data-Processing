#include "pgm16.h"
#include <fstream>
#include <vector>
#include <algorithm>

struct bitreader
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ifstream& is_;

	uint8_t readbit() {
		if (n_ == 0)
		{
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	bitreader(std::ifstream& is) : is_(is) {}

	uint64_t operator()(int numbits) {
		uint64_t val = 0;
		for (int i = numbits - 1; i >= 0; i--)
		{
			val <<= 1;
			val |= readbit();
		}
		return val;
	}
};

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {

	std::ifstream is(filename, std::ios::binary);
	if (!is)
	{
		return false;
	}

	std::string magicnumber(2, ' ');
	std::string trash;

	uint32_t width = 0, height = 0;
	uint16_t maxval = 0;

	is >> magicnumber;
	if (magicnumber != "P5")
	{
		return false;
	}

	is.ignore(1);
	if (is.peek() == '#')
	{
		while (is.peek() != '\n')
		{
			is >> trash;
		}
	}

	is >> width;
	is >> height;
	is >> maxval;
	is.ignore(1); // removing last \n

	img.resize(height, width);
	if (maxval <= 255)
	{
		for (int r = 0; r < img.rows(); r++)
		{
			for (int c = 0; c < img.cols(); c++)
			{
				img(r, c) = is.get();
			}
		}
	}
	else
	{
		bitreader br(is);
		for (int r = 0; r < img.rows(); r++)
		{
			for (int c = 0; c < img.cols(); c++)
			{
				img(r, c) = br(16);
			}
		}
	}

	maxvalue = maxval;
	return true;
}