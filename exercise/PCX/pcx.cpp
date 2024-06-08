#include "mat.h"
#include "pcx.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <cstdint>
#include <cstdbool>

struct head
{
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bitsPerPlane;
	uint16_t Xmin, Ymin, Xmax, Ymax;
	uint8_t colorPlanes;
	uint16_t bytesPerPlaneLine;

};

bool load_pcx(const std::string& filename, mat<uint8_t>& img) {
	
	std::ifstream is(filename, std::ios::binary);
	if (!is)
	{
		return false;
	}

	head h;

	h.manufacturer = is.get();
	if (h.manufacturer != 0x0A)
	{
		return false;
	}

	h.version = is.get();
	h.encoding = is.get();
	h.bitsPerPlane = is.get();

	is.read(reinterpret_cast<char*>(&h.Xmin), 2);
	is.read(reinterpret_cast<char*>(&h.Ymin), 2);
	is.read(reinterpret_cast<char*>(&h.Xmax), 2);
	is.read(reinterpret_cast<char*>(&h.Ymax), 2);

	is.ignore(48+5);
	h.colorPlanes = is.get();
	is.read(reinterpret_cast<char*>(&h.bytesPerPlaneLine), 2);
	is.ignore(54+6);

	std::vector<uint8_t> dec_stream;

	auto Xsize = h.Xmax - h.Xmin + 1;
	auto Ysize = h.Ymax - h.Ymin + 1;
	uint64_t totalB = h.colorPlanes * h.bytesPerPlaneLine; 

	uint64_t readed = 0;
	uint64_t tot_readed = 0;
	int linereaded = 0;

	while (dec_stream.size() < ((Xsize * Ysize) / 8))
	{
		tot_readed += readed;
		readed = 0;
		int pad = 1;
		if ((Xsize % totalB) == 0)
		{
			pad = 0;
		}
		//int pad = (totalB - (Xsize % totalB)) / (std::round((Xsize / totalB)));
		while (readed < totalB)
		{
			uint8_t first = is.get();
			if ((first >> 6) == 0b11)
			{
				uint8_t count = first & 0b00111111;
				uint8_t val = is.get();
				for (size_t ii = 0; ii < count; ii++)
				{
					dec_stream.push_back(val);
				}
				readed += count;
			}
			else
			{
				dec_stream.push_back(first);
				++readed;
			}
		}
		for (size_t i = 0; i < pad; i++)
		{
			dec_stream.pop_back();
		}

	}
	if (is.peek() == EOF)
	{
		std::cout << "siamo a EOF";
	}

	img.resize(Ysize, Xsize);
	int index = 0;
	for (int r = 0; r < img.rows(); r++)
	{
		for (int c = 0; c < img.cols(); c+=8)
		{
			auto v = dec_stream[index];
			for (int k = 7; k >= 0; k--)
			{
				if (((v >> k) & 1) == 1)
				{
					img(r, c + 7 - k) = 255;
				}
				else
				{
					img(r, c + 7 - k) = 0;
				}
			}
			++index;
		}
	}
	std::cout << "ciao";
	return true;
}