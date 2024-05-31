#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <array>
#include <cmath>
#include <format>

struct head
{
	std::string magicnumber, tupltype, headend;
	uint32_t width, height;
	uint8_t depth, maxval;
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

	const T& operator()(size_t row, size_t col) const {
		return data_[row * c_ + col];
	}

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};

auto readHead(std::ifstream& is) {
	head h;

	is >> h.magicnumber;
	if (h.magicnumber != "P7")
	{
		return head{};
	}
	
	std::string field;
	while (true)
	{
		is >> field;
		if (field == "WIDTH")
		{
			is >> h.width;
		}
		else if (field == "HEIGHT") {
			is >> h.height;
		}
		else if (field == "DEPTH") {
			is >> h.depth;
		}
		else if (field == "MAXVAL") {
			is >> h.maxval;
		}
		else if (field == "TUPLTYPE") {
			is >> h.tupltype;
		}
		else if (field == "ENDHDR") {
			is.get();
			break;
		}
	}
	return h;
}

auto savepam(mat<std::array<uint8_t, 4>>& img, const char* out) {
	std::string head = std::format(
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 4\n"
		"MAXVAL 255\n"
		"TUPLTYPE RGB_ALPHA\n"
		"ENDHDR\n"
		, img.cols(), img.rows());

	std::ofstream os(out, std::ios::binary);
	if (!os)
	{
		return 1;
	}

	os << head;

	os.write(reinterpret_cast<char*>(img.rawdata()), img.dim());
	return 0;
}

auto compose(const char* outputfile, std::vector<std::tuple<std::string, uint32_t, uint32_t>>& list) {

	int x_def = 0, y_def = 0;

	for (const auto& [file, x, y] : list) {
		std::ifstream is(file, std::ios::binary);
		if (!is)
		{
			return 1;
		}
		head h = readHead(is);
		if (x_def < (x + h.width))
		{
			x_def = x + h.width;
		}
		if (y_def < (y + h.height))
		{
			y_def = y + h.height;
		}
	}

	mat<std::array<uint8_t, 4>> img(y_def, x_def);
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = {0, 0, 0, 0};
		}
	}

	for (const auto& [file, x, y] : list) {
		std::ifstream is(file, std::ios::binary);
		if (!is)
		{
			return 1;
		}
		head h = readHead(is);
		mat<std::array<uint8_t, 4>> fileimg(h.height, h.width);

		for (size_t r = 0; r < fileimg.rows(); r++)
		{
			for (size_t c = 0; c < fileimg.cols(); c++)
			{
				fileimg(r, c)[0] = is.get();
				fileimg(r, c)[1] = is.get();
				fileimg(r, c)[2] = is.get();
				if (h.depth == '3')
				{
					fileimg(r, c)[3] = 255;
				}
				else
				{
					fileimg(r, c)[3] = is.get();
				}
			}
		}

		for (size_t r = 0; r < fileimg.rows(); r++)
		{
			for (size_t c = 0; c < fileimg.cols(); c++)
			{
				double alphaA = (double)fileimg(r, c)[3] / 255.0 ;
				double alphaB = (double)img(r + y, c + x)[3] / 255.0;
				double alphaOUT = alphaA + alphaB * (1 - alphaA);
				for (size_t i = 0; i < 3; i++)
				{
					double c_a = (double)fileimg(r, c)[i];
					double c_b = (double)img(r + y, c + x)[i];
					double Cout = ((c_a * alphaA) + (c_b * alphaB) * (1 - alphaA)) / alphaOUT;
					img(r + y, c + x)[i] = Cout;
				}
				img(r + y, c + x)[3] = std::round(alphaOUT * 255.0);
			}
		}

	}
	savepam(img, outputfile);
	return 0;
}

int main(int argc, char** argv) {
	
	std::vector<std::tuple<std::string, uint32_t, uint32_t>> list;

	std::string outputfile(argv[1]);
	outputfile.append(".pam");

	int index = 2;
	int max_x = 0, max_y = 0;
	while (index < argc)
	{
		if ((argv[index][0] == '-') && (argv[index][1] == 'p'))
		{
			++index;
			int x = atoi(argv[index]);
			int y = atoi(argv[index + 1]);
			std::string file(argv[index + 2]);
			file.append(".pam");
			list.emplace_back(file, x, y);
			index += 3;
		}
		else
		{
			std::string file(argv[index]);
			file.append(".pam");
			list.emplace_back(file, 0, 0);
			++index;
		}
	}

	compose(outputfile.c_str(), list);

	return 0;

}

