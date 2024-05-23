#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <array>
#include <format>
#include <cmath>

void error(const char* errmsg) {
	std::cerr << errmsg << std::endl;
}

struct reader
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

	reader(std::ifstream& is) : is_(is) {}

	uint64_t operator()(int nbits) {
		uint64_t val = 0;
		for (int i = nbits - 1; i >= 0; i--)
		{
			val <<= 1;
			val |= readbit();
		}
		return val;
	}
};

template <typename T>
struct mat
{
	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	T& operator()(int row, size_t col) {
		return data_[row * c_ + col];
	}

	const T& operator()(int row, size_t col) const {
		return data_[row * c_ + col];
	}

	size_t cols() {
		return c_;
	}

	size_t rows() {
		return r_;
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	auto dim() {
		return data_.size() * sizeof(T);
	}
};


struct head
{
	int16_t magicnumber = 0;
	uint32_t size = 0;
	int16_t reserved1 = 0, reserved2 = 0;
	uint32_t offset = 0;

	uint32_t headersize = 0;
	int32_t width = 0, height = 0;
	uint16_t colorplane = 0, bpp = 0;
	uint32_t compression_method = 0;
	uint32_t imgsize = 0, h_res = 0, v_res = 0;
	uint32_t num_colors = 0;
	uint32_t imp_color = 0;

};

auto readHeader(std::ifstream& is, head& h) {

	is.read(reinterpret_cast<char*>(&h.magicnumber), 2);
	if (h.magicnumber != 19778)
	{
		error("Magic number should be: 0x42 0x4D in hexadecimal, ");
		return EXIT_FAILURE;
	}

	is.read(reinterpret_cast<char*>(&h.size), 4); // BMP file size
	is.read(reinterpret_cast<char*>(&h.reserved1), 2); // reserved 1
	is.read(reinterpret_cast<char*>(&h.reserved2), 2); // reserved 2
	is.read(reinterpret_cast<char*>(&h.offset), 4); // offset

	is.read(reinterpret_cast<char*>(&h.headersize), 4); // header size
	is.read(reinterpret_cast<char*>(&h.width), 4); // width
	is.read(reinterpret_cast<char*>(&h.height), 4); // height
	is.read(reinterpret_cast<char*>(&h.colorplane), 2); // number of color plane
	is.read(reinterpret_cast<char*>(&h.bpp), 2); // bit per pixel
	is.read(reinterpret_cast<char*>(&h.compression_method), 4); // compression method
	is.read(reinterpret_cast<char*>(&h.imgsize), 4); // image size
	is.read(reinterpret_cast<char*>(&h.h_res), 4); // h_res
	is.read(reinterpret_cast<char*>(&h.v_res), 4); // v_res
	is.read(reinterpret_cast<char*>(&h.num_colors), 4); // num_colors
	is.read(reinterpret_cast<char*>(&h.imp_color), 4); // imp_color

	return EXIT_SUCCESS;

}

auto savepam_RGB(const char* output, mat<std::array<uint8_t, 3>>& img, head& h) {
	
	std::ofstream os(output, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}

	std::string headerPAM = std::format(
	
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 3\n"
		"MAXVAL 255\n"
		"TUPLTYPE RGB\n"
		"ENDHDR\n"

	,img.cols(), img.rows());

	os.write(reinterpret_cast<char*>(headerPAM.data()), headerPAM.size());
	os.write(img.rawdata(), img.dim());

	return EXIT_SUCCESS;

}

auto bpp24(head& h, std::ifstream& is, const char* output) {
	mat<std::array<uint8_t, 3>> img(h.height, h.width);

	int pad = std::round((img.cols() % 32) / 8.0);

	for (int r = img.rows() - 1; r >= 0; --r)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c)[2] = is.get();
			img(r, c)[1] = is.get();
			img(r, c)[0] = is.get();
		}
		for (size_t i = 0; i < pad; i++)
		{
			is.get();
		}
	}

	savepam_RGB(output, img, h);

}

auto bpp8(head& h, std::ifstream& is, const char* output) {
	auto n_col = std::pow(2, h.bpp);
	std::vector<std::array<uint8_t, 3>> table(n_col);

	for (size_t i = 0; i < n_col; i++)
	{
		std::array<uint8_t, 3> color;
		color[2] = is.get();
		color[1] = is.get();
		color[0] = is.get();
		table[i] = color;
		is.get();
	}

	mat<std::array<uint8_t, 3>> img(h.height, h.width);

	int pad = (32 - ((h.width * h.bpp) % 32)) / 8;

	for (int r = img.rows() - 1; r >= 0; --r)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = table[is.get()];
		}
		for (size_t i = 0; i < pad; i++)
		{
			is.get();
		}
	}
	savepam_RGB(output, img, h);
}

auto bpp4(head& h, std::ifstream& is, const char* output) {
	int n_col = 0;
	if (h.num_colors == 0)
	{
		n_col = std::pow(2, h.bpp);
	}
	else
	{
		n_col = h.num_colors;
	}
	std::vector<std::array<uint8_t, 3>> table(n_col);

	for (size_t i = 0; i < n_col; i++)
	{
		std::array<uint8_t, 3> color;
		color[2] = is.get();
		color[1] = is.get();
		color[0] = is.get();
		table[i] = color;
		is.get();
	}

	mat<std::array<uint8_t, 3>> img(h.height, h.width);
	reader br(is);

	int pad = (32 - ((h.width * h.bpp) % 32)) / 8;

	for (int r = img.rows() - 1; r >= 0; --r)
	{
		for (size_t c = 0; c < img.cols(); c+=2)
		{
			uint8_t index = br(4);
			img(r, c) = table[index];
			index = br(4);
			if ((c + 1) < img.cols())
			{
				
				img(r, c + 1) = table[index];
			}
		}
		for (size_t i = 0; i < pad; i++)
		{
			is.get();
		}
	}
	savepam_RGB(output, img, h);
}

auto bpp1(head& h, std::ifstream& is, const char* output) {
	int n_col = 0;
	if (h.num_colors == 0)
	{
		n_col = std::pow(2, h.bpp);
	}
	else
	{
		n_col = h.num_colors;
	}
	std::vector<std::array<uint8_t, 3>> table(n_col);

	for (size_t i = 0; i < n_col; i++)
	{
		std::array<uint8_t, 3> color;
		color[2] = is.get();
		color[1] = is.get();
		color[0] = is.get();
		table[i] = color;
		is.get();
	}

	mat<std::array<uint8_t, 3>> img(h.height, h.width);
	reader br(is);

	int pad = (32 - ((h.width * h.bpp) % 32)) / 8;

	for (int r = img.rows() - 1; r >= 0; --r)
	{
		for (size_t c = 0; c < img.cols(); c += 8)
		{
			for (size_t i = 0; i < 8; i++)
			{
				uint8_t index = br(1);
				if ((c + i) < img.cols())
				{
					img(r, c + i) = table[index];
				}
			}
		}
		for (size_t i = 0; i < pad; i++)
		{
			is.get();
		}
	}
	savepam_RGB(output, img, h);
}


auto decode(const char* input, const char* output) {
	
	std::ifstream is(input, std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return EXIT_FAILURE;
	}

	head h; 
	readHeader(is, h);

	if (h.bpp == 24)
	{
		bpp24(h, is, output);
	}
	else if (h.bpp == 8) {
		bpp8(h, is, output);
	}
	else if (h.bpp == 4) {
		bpp4(h, is, output);
	}
	else if (h.bpp == 1) {
		bpp1(h, is, output);
	}

	

	return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {
	
	if (argc != 3)
	{
		error("Usage is bmp2pam <input file .BMP> <output file .PAM>");
		return EXIT_FAILURE;
	}

	decode(argv[1], argv[2]);
	
	return EXIT_SUCCESS;

}