#include <vector>
#include <fstream>
#include <algorithm>
#include <print>

template<typename T>
struct matrix {

	size_t rows_;
	size_t cols_;
	std::vector<T> data_;

public:
	matrix(size_t r, size_t c) : rows_(r), cols_(c), data_(r* c) {}

	T& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}
	const T& operator()(size_t r, size_t c) const {
		return data_[r * cols_ + c];
	}

	size_t rows() {
		return rows_;
	}
	size_t cols() {
		return cols_;
	}
	size_t dim() {
		return rows_ * cols_;
	}

	auto rawdata() {
		return reinterpret_cast<char *>(data_.data());
	}
	auto rawdim() {
		return data_.size() * sizeof(T);
	}
};


struct header {

	size_t width = 0;
	size_t height = 0;
	size_t depth = 0;
	size_t maxval = 0;
	std::string tupltype;

};

void readhead(std::istream& is, header& head) {
	

	std::string str;
	is >> str; //p7
	is >> str; //widht
	is >> head.width;

	is >> str; //height
	is >> head.height;

	is >> str; //depth
	is >> head.depth;

	is >> str; //maxval
	is >> head.maxval;

	is >> str; //tupltype
	is >> head.tupltype;

	is >> str; //endhdr
	is.get(); //removing newline
}

void readimg(matrix<uint8_t>& img, std::istream& is, header& head) {

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = is.get();
		}
	}
}

void build_reverse(matrix<uint8_t>& img, header& head, matrix<uint8_t>& newimg) {

	for (rsize_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			newimg(r, c) = img(img.rows() - r - 1, c);	
		}
	}
}

auto save_reverse(std::string filename, matrix<uint8_t>& reversed_image) {
	std::ofstream os(filename, std::ios::binary);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	std::print(os,
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 1\n"
		"MAXVAL 255\n"
		"TUPLTYPE GRAYSCALE\n"
		"ENDHDR\n", reversed_image.cols(), reversed_image.rows());

	os.write(reversed_image.rawdata(), reversed_image.rawdim());
}

int main(void) {

	std::ifstream is("frog.pam", std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}
	header head;
	readhead(is, head);
	matrix<uint8_t> img(head.height, head.width);
	readimg(img, is, head);
	matrix<uint8_t> newimg(head.height, head.width);
	build_reverse(img, head, newimg);
	save_reverse("reversed.pam", newimg);
	return EXIT_SUCCESS;

}