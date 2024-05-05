#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>
#include <format>
#include <cstdint>

void error(const char* errmsg) {

	std::cerr << errmsg;
}

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

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}

	const T& operator()(size_t row, size_t col) const {
		return data_[row * c_ + col];
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};

auto savepam(mat<std::array<uint8_t, 3>>& rgbimg, std::string prefix) {

	std::string out(prefix.append("reconstructed.pam"));
	std::ofstream os(out, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}

	std::string head = std::format("P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 3\n"
		"MAXVAL 255\n"
		"TUPLTYPE RGB\n"
		"ENDHDR\n", rgbimg.cols(), rgbimg.rows());

	os << head;

	os.write(rgbimg.rawdata(), rgbimg.dim());

	return EXIT_SUCCESS;
}

auto loadpam(std::string& out) {

	std::ifstream is(out, std::ios::binary);
	if (!is)
	{
		error("Error while reading input file");
		/*mat<uint8_t> falsematr(0, 0);
		delete(&falsematr);
		return falsematr;*/
	}


	size_t width = 0, height = 0, maxval = 0, depth = 0;
	std::string tupltype;
	std::string field(12, ' ');
	while (field != "ENDHDR")
	{
		is >> field;
		if (field == "WIDTH")
		{
			is >> width;
		}
		else if (field == "HEIGHT")
		{
			is >> height;
		}
		else if (field == "MAXVAL")
		{
			is >> maxval;
		}
		else if (field == "DEPTH")
		{
			is >> depth;
		}
		else if (field == "TUPLTYPE")
		{
			is >> tupltype;
		}
	}
	is.get(); //remoing last \n

	mat<uint8_t> img(height, width);
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = is.get();
		}
	}

	return img;
}


auto loadrgb(std::string prefix) {

	std::string prefix_r(prefix + "_R.pam");
	mat<uint8_t> red = loadpam(prefix_r);

	std::string prefix_g(prefix + "_G.pam");;
	mat<uint8_t> green = loadpam(prefix_g);

	std::string prefix_b(prefix + "_B.pam");
	mat<uint8_t> blue = loadpam(prefix_b);

	mat<std::array<uint8_t, 3>> rgbimg(red.rows(), red.cols());
	for (size_t r = 0;  r < rgbimg.rows();  r++)
	{
		for (size_t c = 0; c < rgbimg.cols(); c++)
		{
			rgbimg(r, c)[0] = red(r, c);
			rgbimg(r, c)[1] = green(r, c);
			rgbimg(r, c)[2] = blue(r, c);
		}
	}
	savepam(rgbimg, prefix);
	
}


int main(int argc, char* argv[]) {

	if (argc != 2)
	{
		error("Usaege is combine <filename>");
		return EXIT_FAILURE;
	}

	loadrgb(argv[1]);

	return EXIT_SUCCESS;

}