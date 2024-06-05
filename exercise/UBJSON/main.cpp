#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <unordered_map>

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

struct canvas
{
	size_t width = 0;
	size_t height = 0;
	std::array<uint8_t, 3> background;
};

template <typename T>
struct image
{
	size_t x;
	size_t y;
	size_t width;
	size_t height;
	std::vector<T> data;
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

	auto dim() {
		return sizeof(T) * data_.size();
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	
};

int error(const char* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}

auto computesize(char& c) {
	if (c == 'i')
	{
		return 1;
	}
	else if (c == 'I')
	{
		return 2;
	}
	else if (c == 'U') {
		return 1;
	}
	else if (c == 'l') {
		return 4;
	}
	else if (c == 'd') {
		return 4;
	}
	return 0;
}


auto saveppm(std::string& file, mat<std::array<uint8_t, 3>>& img) {
	std::ofstream os(file, std::ios::binary);
	if (!os)
	{
		return 1;
	}

	/*std::string head = std::format(
		"P6 {} {} 255 "
		, img.cols(), img.rows());*/
	
	std::string head = std::format(
		"P6\n#PPM creato nel corso di SdEM\n{} {}\n255\n"
		, img.cols(), img.rows());

	os << head;

	os.write(img.rawdata(), img.dim());

	return EXIT_SUCCESS;
}

int readelem(std::ifstream& is, mat<std::array<uint8_t, 3>>& canvas) {
	char type = 0;
	size_t size = 0;
	std::string name, fieldname;
	uint32_t val;
	uint8_t dim;
	bitreader br(is);

	int nimage = 0;

	std::unordered_map<std::string, uint32_t> image;
	std::vector<uint8_t> dataimage;

	while (is.peek() != '}')
	{
		type = is.get();
		size = is.get();

		name.resize(size);
		is.read(reinterpret_cast<char*>(name.data()), size);

		std::cout << name << " : ";

		if (is.peek() == '{')
		{
			is.get();
			while (is.peek() != '}')
			{
				type = is.get();
				size = is.get();

				fieldname.erase();
				fieldname.resize(size);
				is.read(reinterpret_cast<char*>(fieldname.data()), size);
				std::cout << fieldname << ',';
				if (is.peek() == '[')
				{
					is.ignore(2);
					type = is.get();
					dim = computesize(type);
					is.ignore(1);
					type = is.get();
					uint32_t len = computesize(type);
					int count = br(len * 8);
					if (name == "image")
					{
						mat<std::array<uint8_t, 3>> img(image["height"], image["width"]);
						for (size_t r = 0; r < img.rows(); r++)
						{
							for (size_t c = 0; c < img.cols(); c++)
							{
								img(r, c)[0] = is.get();
								img(r, c)[1] = is.get();
								img(r, c)[2] = is.get();
							}
						}

						++nimage;
						std::string imgname("image");
						imgname.append(std::to_string(nimage));
						imgname.append(".ppm");

						saveppm(imgname, img);

						uint32_t x = image["x"];
						uint32_t y = image["y"];
						for (size_t r = 0; r < img.rows(); r++)
						{
							for (size_t c = 0; c < img.cols(); c++)
							{
								canvas(r + y, c + x) = img(r, c);
							}
						}
					}
					else
					{
						is.ignore(count);
					}
				}
				else
				{
					type = is.get();
					dim = computesize(type);

					val = br(dim * 8);
					if (name == "image")
					{
						image[fieldname] = val;
					}
				}
			}
		}
		std::cout << std::endl;
		is.get();

	}

	return EXIT_SUCCESS;
	
}

auto decode(const char* inputfile, const char* outputfile) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening input file for decoding");
	}

	canvas c;
	image<uint8_t> i;
	char type = 0;
	uint8_t dim = 0;
	uint8_t strlen = 0;
	int32_t val = 0;
	bitreader br(is);

	is.ignore(1); // {
	type = is.get();
	dim = computesize(type);
	strlen = is.get();
	is.ignore(strlen + 1); // ignoro la stringa e {
	
	//width
	type = is.get();
	dim = computesize(type);
	strlen = is.get();
	is.ignore(strlen);
	type = is.get();
	dim = computesize(type);
	c.width = br(dim * 8);

	//height
	type = is.get();
	dim = computesize(type);
	strlen = is.get();
	is.ignore(strlen);
	type = is.get();
	dim = computesize(type);
	c.height = br(dim * 8);

	type = is.get();
	dim = computesize(type);
	strlen = is.get();
	is.ignore(strlen);
	is.ignore(6); // [$U#i + '3'
	c.background[0] = is.get();
	c.background[1] = is.get();
	c.background[2] = is.get();
	is.ignore(12); // skip background string

	uint8_t r = c.background[0];
	uint8_t g = c.background[1];
	uint8_t b = c.background[2];
	std::array<uint8_t, 3> pix{ r, g, b };
	mat<std::array<uint8_t, 3>> img(c.height, c.width);
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = pix;
		}
	}

	std::string s("canvas.ppm");
	saveppm(s, img);

	readelem(is, img);

	std::string ss(outputfile);
	saveppm(ss, img);
	return EXIT_SUCCESS;

}

int main(int argc, char** argv) {
	
	if (argc != 3)
	{
		return error("Usage is: ubj2ppm <input .UBJ> <output .PPM>");
	}

	decode(argv[1], argv[2]);

	return EXIT_SUCCESS;

}