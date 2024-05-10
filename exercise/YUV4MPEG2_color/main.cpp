#include "mat.h"


#include <iostream>
#include <cstdint>
#include <fstream>
#include <array>
#include <cmath>


using vec3b = std::array<uint8_t, 3>;

void clamp(double& x) {
	if (x < 0)
	{
		x = 0;
	}
	else if (x > 255) {
		x = 255;
	}
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {

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
		mat<vec3b> frame(height, width);
		for (int r = 0; r < frame.rows(); r++)
		{
			for (int c = 0; c < frame.cols(); c++)
			{
				auto val = is.get();
				if (val < 16)
				{
					val = 16;
				}
				else if (val > 235) {
					val = 235;
				}
				frame(r, c)[0] = val;
			}
		}
		//frames.push_back(frame);
		size_t r = 0, c = 0;
		for (int i = 0; i < (frame.cols() * frame.rows()) / 4; i++)
		{
			auto Cb = is.get();
			if (Cb < 16)
			{
				Cb = 16;
			}
			else if (Cb > 240) {
				Cb = 240;
			}
			if (((c % width) == 0) && (c != 0))
			{
				c = 0;
				r+=2;
			}
			frame(r, c)[1] = Cb;
			frame(r, c + 1)[1] = Cb;
			frame(r + 1, c)[1] = Cb;
			frame(r + 1, c + 1)[1] = Cb;
			c += 2;
		}
		r = 0;
		c = 0;
		for (int i = 0; i < (frame.cols() * frame.rows()) / 4; i++)
		{
			auto Cr = is.get();
			if (Cr < 16)
			{
				Cr = 16;
			}
			else if (Cr > 240) {
				Cr = 240;
			}
			if (((c % width) == 0) && (c != 0))
			{
				c = 0;
				r += 2;
			}
			frame(r, c)[2] = Cr;
			frame(r, c + 1)[2] = Cr;
			frame(r + 1, c)[2] = Cr;
			frame(r + 1, c + 1)[2] = Cr;
			c += 2;
		}
		frames.push_back(frame);
		if (is.peek() == EOF)
		{
			break;
		}
		is >> field;
	}

	//std::cout << "ciao";
	std::vector<std::vector<double>> cv{
		{1.164, 0.000, 1.596},
		{1.164, -0.392, -0.813},
		{1.164, 2.017, 0.000}
	};

	std::vector<mat<vec3b>> rgb_frames;
	mat<vec3b> display(height, width);
	
	for (int f = 0; f < frames.size(); f++)
	{
		mat<vec3b> rgb_fr(height, width);
		//mat<vec3b> display(height, width);
		for (size_t r = 0; r < frames[f].rows(); r++)
		{
			for (int c = 0; c < frames[f].cols(); c++)
			{
				double red = 0;
				red = cv[0][0] * (frames[f](r, c)[0] - 16);
				red += cv[0][1] * (frames[f](r, c)[1] - 128);
				red += cv[0][2] * (frames[f](r, c)[2] - 128);

				double green = 0;
				green = cv[1][0] * (frames[f](r, c)[0] - 16);
				green += cv[1][1] * (frames[f](r, c)[1] - 128);
				green += cv[1][2] * (frames[f](r, c)[2] - 128);

				double blue = 0;
				blue = cv[2][0] * (frames[f](r, c)[0] - 16);
				blue += cv[2][1] * (frames[f](r, c)[1] - 128);
				blue += cv[2][2] * (frames[f](r, c)[2] - 128);


				clamp(red);
				clamp(green);
				clamp(blue);

				rgb_fr(r, c)[0] = (red);
				rgb_fr(r, c)[1] = (green);
				rgb_fr(r, c)[2] = (blue);

				display(r, c)[0] = blue;
				display(r, c)[1] = green;
				display(r, c)[2] = red;

			}
		}
		rgb_frames.push_back(rgb_fr);
	}
	frames = rgb_frames;
	return true;

}

int main(int argc, char* argv[]) {

	if (argc != 2)
	{
		std::cerr << "Usage is y4m_color <file.y4m>\n";
		return -1;
	}

	std::vector<mat<vec3b>> frames;

	y4m_extract_color(argv[1], frames);

	return 0;
}