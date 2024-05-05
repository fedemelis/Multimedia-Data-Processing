#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <array>
#include <cstdint>


void error(const char* errmsg) {

	std::cerr << errmsg << '\n';
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


auto savepam(std::string& out, mat<uint8_t>& img) {
	std::ofstream os(out, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}

	os << "P7\n";
	os << "WIDTH ";
	os << img.cols() << '\n';
	os << "HEIGHT ";
	os << img.rows() << '\n';
	os << "DEPTH ";
	os << 1 << '\n';
	os << "MAXVAL ";
	os << 255 << '\n';
	os << "TUPLTYPE ";
	os << "GRAYSCALE" << '\n';
	os << "ENDHDR" << '\n';

	os.write(img.rawdata(), img.dim());

	return EXIT_SUCCESS;
}


auto split(const char* inputfile) {
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return EXIT_FAILURE;
	}

	std::string pref(inputfile);
	auto res = pref.find('.');
	auto prefix = pref.substr(0, res);

	std::string magicnumber(8, ' ');
	is >> magicnumber;

	if (magicnumber != "P7")
	{
		error("PAM is the format supported in this exercise");
		return EXIT_FAILURE;
	}

	size_t width = 0;
	size_t height = 0;
	size_t  maxval = 0;
	size_t depth = 0;

	is >> magicnumber; // width
	is >> width;
	is >> magicnumber; // heigth
	is >> height;
	is >> magicnumber; // depth
	is >> depth;
	is >> magicnumber; // maxval
	is >> maxval;

	std::string tupltype(3, ' ');
	is >> magicnumber; // tupltype
	is >> tupltype;

	std::string endhdr(6, ' ');
	is >> endhdr;

	is.get(); //removing return

	mat<std::array<uint8_t, 3>> colorimg(height, width);
	mat<std::array<uint8_t, 3>> BGR(height, width);


	for (size_t r = 0; r < colorimg.rows(); r++)
	{
		for (size_t c = 0; c < colorimg.cols(); c++)
		{
			colorimg(r, c)[0] = is.get();
			colorimg(r, c)[1] = is.get();
			colorimg(r, c)[2] = is.get();
			BGR(r, c)[0] = colorimg(r, c)[2];
			BGR(r, c)[1] = colorimg(r, c)[1];
			BGR(r, c)[2] = colorimg(r, c)[0];
		}
	}

	mat<uint8_t> red(colorimg.rows(), colorimg.cols());
	for (size_t r = 0; r < colorimg.rows(); r++)
	{
		for (size_t c = 0; c < colorimg.cols(); c++)
		{
			red(r, c) = colorimg(r, c)[0];
		}
	}
	std::string outR(prefix);
	outR.append("_R.pam");

	savepam(outR, red);


	mat<uint8_t> green(colorimg.rows(), colorimg.cols());
	for (size_t r = 0; r < colorimg.rows(); r++)
	{
		for (size_t c = 0; c < colorimg.cols(); c++)
		{
			green(r, c) = colorimg(r, c)[1];
		}
	}
	std::string outG(prefix);
	outG.append("_G.pam");

	savepam(outG, green);

	mat<uint8_t> blue(colorimg.rows(), colorimg.cols());
	for (size_t r = 0; r < colorimg.rows(); r++)
	{
		for (size_t c = 0; c < colorimg.cols(); c++)
		{
			blue(r, c) = colorimg(r, c)[2];
		}
	}
	std::string outB(prefix);
	outB.append("_B.pam");

	savepam(outB, blue);

	return EXIT_SUCCESS;

}


int main(int argc, char* argv[]) {
	
	if (argc != 2)
	{
		error("Usage is split <inputfile>");
		return EXIT_FAILURE;
	}
	
	split(argv[1]);

	return EXIT_SUCCESS;

}