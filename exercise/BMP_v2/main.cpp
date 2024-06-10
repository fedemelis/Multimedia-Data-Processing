#include <iostream>
#include <fstream>
#include <vector>
#include <format>
#include <string>
#include <cstdint>
#include <algorithm>
#include <array>
#include <cmath>

struct FileHead {
	std::string magicnumber;
	uint32_t size = 0;
	uint16_t reserved1 = 0, reserved2 = 0;
	uint32_t dataOffset = 0;

	FileHead() : magicnumber(2, ' ') {}
};

struct InfoHead {
	uint32_t infoHeadSize = 0;
	int width = 0;
	int height = 0;
	uint16_t colorPlaneNumber = 0;
	uint16_t bitPerPixel = 0;
	uint32_t compressionMethod = 0;
	uint32_t imageSize = 0;
	int hResolution = 0;
	int vResolution = 0;
	uint32_t numColor = 0;
	uint32_t numImportantColor = 0;
};

template <typename T>
struct mat {
	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	const size_t rows() const{
		return r_;
	}

	const size_t cols() const {
		return c_;
	}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}

	const T& operator()(size_t row, size_t col) const {
		return data_[row * c_ + col];
	}

	const size_t dim() const {
		return data_.size() * sizeof(T);
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};

int error(const std::string& errmsg) {
	std::cerr << errmsg << std::endl;
	return EXIT_FAILURE;
}

int savePAM(mat<std::array<uint8_t, 3>>& img, const char* outputfile) {
	
	std::ofstream os(outputfile, std::ios::binary);
	if (!os) {
		return error("Error while opening output file for decoding");
	}

	std::string head = std::format(
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 3\n"
		"MAXVAL 255\n"
		"TUPLTYPE RGB\n"
		"ENDHDR\n"
	, img.cols(), img.rows());

	os << head;

	os.write(img.rawdata(), img.dim());
	return EXIT_SUCCESS;

}

auto read(const char* inputfile, const char*  outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is) {
		return error("Error while opening input file for decoding");
	}

	FileHead fh;

	is.read(reinterpret_cast<char*>(fh.magicnumber.data()), 2);

	if (fh.magicnumber != "BM") {
		return error(std::format("Magic number should have been BM, but \"{}\" was found", fh.magicnumber));
	}

	is.read(reinterpret_cast<char*>(&fh.size), 4);
	is.read(reinterpret_cast<char*>(&fh.reserved1), 2);
	is.read(reinterpret_cast<char*>(&fh.reserved2), 2);
	is.read(reinterpret_cast<char*>(&fh.dataOffset), 4);

	InfoHead ih;

	is.read(reinterpret_cast<char*>(&ih.infoHeadSize), 4);
	is.read(reinterpret_cast<char*>(&ih.width), 4);
	is.read(reinterpret_cast<char*>(&ih.height), 4);
	is.read(reinterpret_cast<char*>(&ih.colorPlaneNumber), 2);
	is.read(reinterpret_cast<char*>(&ih.bitPerPixel), 2);
	is.read(reinterpret_cast<char*>(&ih.compressionMethod), 4);
	is.read(reinterpret_cast<char*>(&ih.imageSize), 4);
	is.read(reinterpret_cast<char*>(&ih.hResolution), 4);
	is.read(reinterpret_cast<char*>(&ih.vResolution), 4);
	is.read(reinterpret_cast<char*>(&ih.numColor), 4);
	is.read(reinterpret_cast<char*>(&ih.numImportantColor), 4);

	
	if (ih.bitPerPixel == 24)
	{
		mat<std::array<uint8_t, 3>> img(ih.height, ih.width);
		int bitPerRow = img.cols() * 24;
		int pad = 32 - (bitPerRow % 32);
		for (int r = static_cast<int>(img.rows()) - 1; r >= 0; r--)
		{
			for (size_t c = 0; c < img.cols(); c++)
			{
				img(r, c)[2] = is.get();
				img(r, c)[1] = is.get();
				img(r, c)[0] = is.get();
			}
			is.ignore(pad/8);
		}
		savePAM(img, outputfile);
	}
	else if (ih.bitPerPixel == 8) {
		if (ih.numColor == 0)
		{
			ih.numColor = std::pow(2, ih.bitPerPixel);
		}
		std::vector<std::array<uint8_t, 3>> palette(ih.numColor);
		for (size_t i = 0; i < ih.numColor; i++)
		{
			std::array<uint8_t, 3> color;
			color[2] = is.get();
			color[1] = is.get();
			color[0] = is.get();
			palette[i] = std::move(color);
			is.ignore(1);
		}
		mat<std::array<uint8_t, 3>> img(ih.height, ih.width);
		int bitPerRow = img.cols() * ih.bitPerPixel;
		int pad = 32 - (bitPerRow % 32);
		for (int r = static_cast<int>(img.rows()) - 1; r >= 0; r--)
		{
			for (size_t c = 0; c < img.cols(); c++)
			{
				img(r, c) = palette[is.get()];
			}
			is.ignore(pad / 8);
		}
		savePAM(img, outputfile);
	}
	else if (ih.bitPerPixel == 4) {
		if (ih.numColor == 0)
		{
			ih.numColor = std::pow(2, ih.bitPerPixel);
		}
		std::vector<std::array<uint8_t, 3>> palette(ih.numColor);
		for (size_t i = 0; i < ih.numColor; i++)
		{
			std::array<uint8_t, 3> color;
			color[2] = is.get();
			color[1] = is.get();
			color[0] = is.get();
			palette[i] = std::move(color);
			is.ignore(1);
		}
		mat<std::array<uint8_t, 3>> img(ih.height, ih.width);
		int bitPerRow = img.cols() * ih.bitPerPixel;
		int pad = 32 - (bitPerRow % 32);
		for (int r = static_cast<int>(img.rows()) - 1; r >= 0; r--)
		{
			for (size_t c = 0; c < img.cols(); c+=2)
			{
				uint8_t val = is.get();
				img(r, c) = palette[(val & 0b11110000) >> 4];
				if ((c+1) < img.cols())
				{
					img(r, c + 1) = palette[val & 0b00001111];
				}
			}
			is.ignore(pad / 8);
		}
		savePAM(img, outputfile);

	}
	else if (ih.bitPerPixel == 1)
	{
		if (ih.numColor == 0)
		{
			ih.numColor = std::pow(2, ih.bitPerPixel);
		}
		std::vector<std::array<uint8_t, 3>> palette(ih.numColor);
		for (size_t i = 0; i < ih.numColor; i++)
		{
			std::array<uint8_t, 3> color;
			color[2] = is.get();
			color[1] = is.get();
			color[0] = is.get();
			palette[i] = std::move(color);
			is.ignore(1);
		}
		mat<std::array<uint8_t, 3>> img(ih.height, ih.width);
		int bitPerRow = img.cols() * ih.bitPerPixel;
		int pad = 32 - (bitPerRow % 32);
		for (int r = static_cast<int>(img.rows()) - 1; r >= 0; r--)
		{
			for (size_t c = 0; c < img.cols(); c += 8)
			{
				uint8_t val = is.get();
				for (size_t cc = 0; cc < 8; cc++)
				{
					if ((c + cc) < img.cols())
					{
						img(r, c + cc) = palette[(val >> (7 - cc)) & 1];
					}
				}
			}
			is.ignore(pad / 8);
		}
		savePAM(img, outputfile);
	}

	// fai un ciclo for
	



	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	
	if (argc != 3) {
		return error("Usage is: bmp2pam <input file .BMP> <output file .PAM>");
	}

	return read(argv[1], argv[2]);

}