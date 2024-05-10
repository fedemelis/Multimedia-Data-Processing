#include "mat.h"
//#include "pgm.h"
//#include "ppm.h"
//#include "types.h"
//#include "utils.h"

#include <iostream>
#include <cstdint>
#include <fstream>

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {

	std::ifstream is(filename, std::ios::binary);
	if (!is)
	{
		std::cerr << "Error while opening input file\n";
		return false;
	}

	// reading header
	std::string field(8, ' ');
	std::string chroma_subsampling(8, ' ');
	size_t width = 0, height = 0;
	std::string trash(3, ' ');
	std::string interlacing(1, ' ');
	is >> field;
	if (field == "YUV4MPEG2")
	{
		is.get(); // removing whitespace
		//keep reading
	}
	else
	{
		std::cerr << "Magic number should be YUV4MPEG2\n";
		return false;
	}
	while (is.peek() != '\n')
	{
		field = is.get();
		if (field == "W") {
			is >> width;
		}
		else if (field == "H") {
			is >> height;
		}
		else if (field == "C") {
			is >> chroma_subsampling;
		}
		else if (field == "I") {
			is >> interlacing;
		}
		else if (field == "F") {
			is >> trash;
		}
		else if (field == "A") {
			is >> trash;
		}
		else if (field == "X") {
			is >> trash;
		}
	}
	is.get();

	is >> field;
	while (field == "FRAME")
	{
		is.get(); //removing newline / space
		mat<uint8_t> frame(height, width);
		for (int r = 0; r < frame.rows(); r++)
		{
			for (int c = 0; c < frame.cols(); c++)
			{
				frame(r, c) = is.get();
			}
		}
		frames.push_back(frame);
		for (int i = 0; i < (frame.rows() * frame.cols()) / 2; i++)
		{
			auto trash = is.get(); // skipping Cb Cr
			//std::cout << trash;
		}
		if (is.peek() == EOF)
		{
			break;
		}
		is >> field;
	}

	//std::cout << "ciao";


	return true;

}

//int main(int argc, char* argv[]) {
//
//	if (argc != 2)
//	{
//		std::cerr << "Usage is y4m_gray <file.y4m>\n";
//		return -1;
//	}
//
//	std::vector<mat<uint8_t>> Y_planes;
//
//	y4m_extract_gray(argv[1], Y_planes);
//
//	return 0;
//}