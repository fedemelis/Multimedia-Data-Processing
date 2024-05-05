#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <print>
#include <array>
#include <iostream>
#include <chrono>

template<typename T>
struct matrix {
private:
	size_t rows_;
	size_t cols_;
	std::vector<T> data_;

public:
	matrix(size_t r = 0, size_t c = 0) : rows_(r), cols_(c), data_(c * r) {}

	size_t rows() {
		return rows_;
	}

	size_t cols() {
		return cols_;
	}

	size_t dim() {
		return rows_ * cols_;
	}

	T& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}

	const T& operator()(size_t r, size_t c) const {
		return data_[r * cols_ + c];
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}	

	auto rawdim() {
		return data_.size() * sizeof(T);
	}

};

struct header
{
	size_t width, height, depth, maxval;
	std::string tupltype;

};

auto readhead(std::ifstream& is, header& head) {
	std::string str;
	is >> str; //p7
	
	is >> str;
	is >> head.width;

	is >> str;
	is >> head.height;

	is >> str;
	is >> head.depth;

	is >> str;
	is >> head.maxval;

	is >> str;
	is >> head.tupltype;

	is >> str; //endhdr
	is.get(); //remove newline
}


auto readimg(std::ifstream& is, matrix<std::array<uint8_t, 3>>& img, header& head) {
	
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c)[0] = is.get();
			img(r, c)[1] = is.get();
			img(r, c)[2] = is.get();
		}
	}
}

void mirrorimg(matrix<std::array<uint8_t, 3>>& img, header& head) {
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0;  c < img.cols()/2;  c++)
		{
			std::swap(img(r, c), img(r, img.cols() - 1 - c));
		}
	}
}


int writeimg(std::string filename, matrix<std::array<uint8_t, 3>>& img, header& head) {
	std::ofstream os(filename, std::ios::binary);
	if (!os)
	{
		return EXIT_FAILURE;
	}
	
	std::print(os,
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH {}\n"
		"MAXVAL {}\n"
		"TUPLTYPE {}\n"
		"ENDHDR\n", img.cols(), img.rows(), head.depth, head.maxval, head.tupltype);

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			os.put(img(r, c)[0]);
			os.put(img(r, c)[1]);
			os.put(img(r, c)[2]);
		}
	}
	//std::cout << "Image saved\n";
	return EXIT_SUCCESS;
}



int main(void) {

	using namespace std::chrono;

	auto start = steady_clock::now();

	header head;

	std::ifstream is("laptop.pam", std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	readhead(is, head);
	matrix<std::array<uint8_t, 3>> img(head.height, head.width);
	readimg(is, img, head);
	mirrorimg(img, head);
	writeimg("mirroredlaptop.pam", img, head);

	auto end = steady_clock::now();
	auto diff = end - start;
	auto duration_ms = duration<double, std::milli>(diff);
	std::cout << "Tempo di esecuzione: " << duration_ms << '\n';

	return EXIT_SUCCESS;

}