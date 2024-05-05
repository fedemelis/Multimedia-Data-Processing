#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <array>
#include <cassert>
#include <cfenv>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdlib>

void error(const char* msg) {
	std::cout << msg << '\n';
}

struct header {

	std::string magicnumber;
	size_t width;
	size_t height;
	size_t maxval;

};

template <typename T>
struct matrix
{
private:
	size_t row_;
	size_t col_;
	std::vector<T> data_;
	T zero_;

public:

	matrix(size_t r, size_t c) : row_(r), col_(c), data_(r* c){}

	T& operator()(size_t r, size_t c) {
		return data_[r * col_ + c];
	}

	const T& operator()(size_t r, size_t c) const{
		return data_[r * col_ + c];
	}

	T& operator()(int r, int c, bool safe) {
		if (r < 0 || r >= row_ || c < 0 || c >= col_)
		{
			return zero_;
		}
		return data_[r * col_ + c];
	}

	const T& operator()(int r, int c, bool safe) const {
		if (r < 0 || r >= row_ || c < 0 || c >= col_)
		{
			return zero_;
		}
		return data_[r * col_ + c];
	}

	size_t get_row() {
		return row_;
	}
	size_t get_col() {
		return col_;
	}

	size_t dim() {
		return row_ * col_;
	}

	void setzero() {
		zero_ = { 0, 0, 0 };
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	auto bytedim() {
		return sizeof(T) * data_.size();
	}
};

auto interpolate_red_blue(matrix<std::array<uint8_t, 3>>& rgb_img) {
	for (int r = 0; r < rgb_img.get_row(); r++)
	{
		for (int c = 0; c < rgb_img.get_col(); c++)
		{
			auto rowodds = (r % 2);
			auto colodds = (c % 2);
			//horizontal interpolation for red
			//blue interpolated alternating cross and vertical interpolation
			if (rowodds == 0)
			{
				//no interpolation for red and cross for blue
				// R pari, C pari
				if (colodds == 0)
				{
					float diag1grad = std::abs(rgb_img(r - 1, c - 1, true)[2] - rgb_img(r + 1, c + 1, true)[2]);
					float gneigh1 = std::abs(rgb_img(r, c)[1] - rgb_img(r - 1, c - 1, true)[1]
						+ rgb_img(r, c)[1] - rgb_img(r + 1, c + 1, true)[1]);

					float diag2grad = std::abs(rgb_img(r - 1, c + 1, true)[2] - rgb_img(r + 1, c - 1, true)[2]);
					float gneigh2 = std::abs(rgb_img(r, c)[1] - rgb_img(r - 1, c + 1, true)[1]
						+ rgb_img(r, c)[1] - rgb_img(r + 1, c - 1, true)[1]);

					diag1grad += gneigh1;
					diag2grad += gneigh2;

					if (diag1grad < diag2grad)
					{
						float comp = rgb_img(r - 1, c - 1, true)[2] + rgb_img(r + 1, c + 1, true)[2];
						float comp2 = rgb_img(r, c)[1] - rgb_img(r - 1, c - 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c + 1, true)[1];
						rgb_img(r, c)[2] = std::round((comp / 2) + (comp2 / 4));
					}
					else if (diag1grad > diag2grad)
					{
						float comp = rgb_img(r - 1, c + 1, true)[2] + rgb_img(r + 1, c - 1, true)[2];
						float comp2 = rgb_img(r, c)[1] - rgb_img(r - 1, c + 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c - 1, true)[1];
						rgb_img(r, c)[2] = std::round((comp / 2) + (comp2 / 4));
					}
					else
					{
						float comp = rgb_img(r - 1, c + 1, true)[2] + rgb_img(r + 1, c - 1, true)[2]
							+ rgb_img(r - 1, c - 1, true)[2] + rgb_img(r + 1, c + 1, true)[2];
						float comp2 = rgb_img(r, c)[1] - rgb_img(r - 1, c - 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c + 1, true)[1] +
							rgb_img(r, c)[1] - rgb_img(r - 1, c + 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c - 1, true)[1];

						rgb_img(r, c)[2] = std::round((comp / 4) + (comp2 / 8));
					}
				}
				//h interpolation for red and v interpolation for blue
				else
				{
					// R pari, C dispari
					rgb_img(r, c)[0] = std::round(((rgb_img(r, c - 1, true)[0] + rgb_img(r, c + 1, true)[0]) / 2));
					rgb_img(r, c)[2] = std::round(((rgb_img(r - 1, c, true)[2] + rgb_img(r + 1, c, true)[2]) / 2));
				}
			}
			else
			{
				//v interpolation for red and h interpolation for blue
				if (colodds == 0)
				{
					// R dispari, C pari
					rgb_img(r, c)[0] = std::round(((rgb_img(r - 1, c, true)[0] + rgb_img(r + 1, c, true)[0]) / 2));
					rgb_img(r, c)[2] = std::round(((rgb_img(r, c - 1, true)[2] + rgb_img(r, c + 1, true)[2]) / 2));
					
				}
				//no interpolation for blue and cross interpolation for red
				else
				{
					//R dispari, C dispari
					float diag1grad = std::abs(rgb_img(r - 1, c - 1, true)[0] - rgb_img(r + 1, c + 1, true)[0]);
					float gneigh1 = std::abs(rgb_img(r, c)[1] - rgb_img(r - 1, c - 1, true)[1]
						+ rgb_img(r, c)[1] - rgb_img(r + 1, c + 1, true)[1]);

					float diag2grad = std::abs(rgb_img(r - 1, c + 1, true)[0] - rgb_img(r + 1, c - 1, true)[0]);
					float gneigh2 = std::abs(rgb_img(r, c)[1] - rgb_img(r - 1, c + 1, true)[1]
						+ rgb_img(r, c)[1] - rgb_img(r + 1, c - 1, true)[1]);

					diag1grad += gneigh1;
					diag2grad += gneigh2;

					if (diag1grad < diag2grad)
					{
						float comp = rgb_img(r - 1, c - 1, true)[0] + rgb_img(r + 1, c + 1, true)[0];
						float comp2 = rgb_img(r, c)[1] - rgb_img(r - 1, c - 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c + 1, true)[1];
						rgb_img(r, c)[0] = std::round((comp / 2) + (comp2 / 4));
					}
					else if (diag1grad > diag2grad)
					{
						float comp = rgb_img(r - 1, c + 1, true)[0] + rgb_img(r + 1, c - 1, true)[0];
						float comp2 = rgb_img(r, c)[1] - rgb_img(r - 1, c + 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c - 1, true)[1];
						rgb_img(r, c)[0] = std::round((comp / 2) + (gneigh2 / 4));
					}
					else
					{
						float comp = rgb_img(r - 1, c + 1, true)[0] + rgb_img(r + 1, c - 1, true)[0]
							+ rgb_img(r - 1, c - 1, true)[0] + rgb_img(r + 1, c + 1, true)[0];
						float comp2 = rgb_img(r, c)[1] - rgb_img(r - 1, c - 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c + 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r - 1, c + 1, true)[1]
							+ rgb_img(r, c)[1] - rgb_img(r + 1, c - 1, true)[1];

						rgb_img(r, c)[0] = std::round((comp / 4) + (comp2 / 8));
					}
				}
			}
		}
	}
}

auto interpolate_green(matrix<std::array<uint8_t, 3>>& rgb_img) {

	for (int r = 0; r < rgb_img.get_row(); r++)
	{
		//se riga pari -> s = 0
		//se riga dispari -> s = 2
		auto s = (r % 2) * 2;

		//se r pari -> start = 0
		//se r dispari -> start = 1
		auto startrow = (r % 2);
		for (int c = startrow; c < rgb_img.get_col(); c+=2)
		{
			// |G4 - G6|
			int hg1 = std::abs(rgb_img(r, c - 1, true)[1] - rgb_img(r, c + 1, true)[1]);
			// |X5 - X3 + X5 - X7|
			int hg2 = std::abs(rgb_img(r, c)[s] - rgb_img(r, c - 2, true)[s] + rgb_img(r, c)[s] - rgb_img(r, c + 2, true)[s]);
			int hgradient = hg1 + hg2;

			// |Gsup - Ginf|
			int vg1 = std::abs(rgb_img(r - 1, c, true)[1] - rgb_img(r + 1, c, true)[1]);
			// |X5 - Xsup2 + X5 - Xinf2|
			int vg2 = std::abs(rgb_img(r, c)[s] - rgb_img(r - 2, c, true)[s] + rgb_img(r, c)[s] - rgb_img(r + 2, c, true)[s]);
			int vgradient = vg1 + vg2;

			if (hgradient < vgradient)
			{
				float comp = rgb_img(r, c - 1, true)[1] + rgb_img(r, c + 1, true)[1];
				float comp2 = rgb_img(r, c)[s] - rgb_img(r, c - 2, true)[s] + rgb_img(r, c)[s] - rgb_img(r, c + 2, true)[s];
				rgb_img(r, c)[1] = std::round((comp/2) + (comp2/4));
			}
			else if (hgradient > vgradient)
			{
				float comp = rgb_img(r - 1, c, true)[1] + rgb_img(r + 1, c, true)[1];
				float comp2 = rgb_img(r, c)[s] - rgb_img(r - 2, c, true)[s] + rgb_img(r, c)[s] - rgb_img(r + 2, c, true)[s];
				rgb_img(r, c)[1] = std::round((comp/2) + (comp2 / 4));
			}
			else
			{
				float comp = rgb_img(r, c - 1, true)[1] + rgb_img(r, c + 1, true)[1] +
					rgb_img(r - 1, c, true)[1] + rgb_img(r + 1, c, true)[1];

				float comp2 = rgb_img(r, c)[s] - rgb_img(r, c - 2, true)[s] + rgb_img(r, c)[s] - rgb_img(r, c + 2, true)[s]
					+ rgb_img(r, c)[s] - rgb_img(r - 2, c, true)[s] + rgb_img(r, c)[s] - rgb_img(r + 2, c, true)[s];

				rgb_img(r, c)[1] = std::round((comp / 4) + (comp2 / 8));
			}

		}
	}
}



auto save_rgb(std::ofstream& os, matrix<std::array<uint8_t, 3>>& rgb_img) {
	os << "P6" << '\n';
	os << rgb_img.get_col() << '\n';
	os << rgb_img.get_row() << '\n';
	os << "255" << '\n';

	os.write(rgb_img.rawdata(), rgb_img.bytedim());

	return EXIT_SUCCESS;
}

auto build_rgb(matrix<std::array<uint8_t, 3>>& rgb_img, matrix<uint8_t>& img) {


	for (size_t r = 0; r < img.get_row(); r++)
	{
		auto line = r % 2;
		for (size_t c = 0; c < img.get_col(); c+=2)
		{
			if (line == 0)
			{
				rgb_img(r, c)[0] = img(r, c);
				rgb_img(r, c + 1)[1] = img(r, c + 1);
			}
			else
			{
				rgb_img(r, c)[1] = img(r, c);
				rgb_img(r, c + 1)[2] = img(r, c + 1);
			}
		}
	}
	
	return EXIT_SUCCESS;

}

auto save_gray(matrix<uint8_t>& img, std::ostream& os) {
	os << "P5" << '\n';
	os << img.get_col() << '\n';
	os << img.get_row() << '\n';
	os << "255" << '\n';

	os.write(img.rawdata(), img.bytedim());

	return EXIT_SUCCESS;
	
}

auto read_pgm_head(std::istream& is, header& head) {

	is >> head.magicnumber;
	is >> head.width;
	is >> head.height;
	is >> head.maxval;
	is.get();
	
	return EXIT_SUCCESS;
}


auto read_pmg(matrix<uint8_t>& img, std::istream& is) {

	for (size_t r = 0; r < img.get_row(); r++)
	{
		for (size_t c = 0; c < img.get_col(); c++)
		{
			uint32_t val = 0;
			val = is.get();
			val <<= 8;
			val |= is.get();
			float f = std::round(static_cast<float>(val) / 256);
			img(r, c) = f;
		}
	}
}


int main(int argc, char* argv[]) {
	
	if (argc != 3)
	{
		error("Usage is: input.pgm output");
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		error("Error while opening input pgm");
		return EXIT_FAILURE;
	}

	header h;

	read_pgm_head(is, h);

	matrix<uint8_t> img(h.height, h.width);
	//img.setzero();

	read_pmg(img, is);

	std::string out(argv[2]);
	out += "_gray.pgm";
	std::ofstream os(out, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file for gray img");
		return EXIT_FAILURE;
	}

	save_gray(img, os);

	matrix<std::array<uint8_t, 3>>rgb_img(img.get_row(), img.get_col());
	rgb_img.setzero();

	build_rgb(rgb_img, img);

	out = argv[2];
	out += "_bayer.ppm";
	std::ofstream os_ppm(out, std::ios::binary);
	if (!os_ppm)
	{
		error("Error while opening output file for gray img");
		return EXIT_FAILURE;
	}

	save_rgb(os_ppm, rgb_img);

	//fill_corner(rgb_img);

	interpolate_green(rgb_img);

	out = argv[2];
	out += "_green.ppm";
	std::ofstream os_green(out, std::ios::binary);
	if (!os_ppm)
	{
		error("Error while opening output file for green img");
		return EXIT_FAILURE;
	}

	save_rgb(os_green, rgb_img);

	interpolate_red_blue(rgb_img);

	out = argv[2];
	out += "_interp.ppm";
	std::ofstream os_interp(out, std::ios::binary);
	if (!os_ppm)
	{
		error("Error while opening output file img");
		return EXIT_FAILURE;
	}

	save_rgb(os_interp, rgb_img);

	return EXIT_SUCCESS;
}