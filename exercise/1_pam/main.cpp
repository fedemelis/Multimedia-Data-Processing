#include <fstream>
#include <vector>
#include <print>

template<typename T>
struct mat {
	size_t rows_;
	size_t cols_;
	std::vector<T> data_;

	mat(size_t rows = 0, size_t cols = 0) : rows_(rows), cols_(cols), data_(rows* cols) {}

	size_t rows() const { return rows_; }
	size_t cols() const { return cols_; }
	size_t size() const { return rows_ * cols_; }

	/*T& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}
	const T& operator()(size_t r, size_t c) const {
		return data_[r * cols_ + c];
	}*/
	
	auto&& operator()(this auto&& self, size_t r, size_t c) {
		return self.data_[r * self.cols_ + c];
	}
	

	auto rawsize() const { return size() * sizeof(T); }
	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	auto rawdata() const {
		return reinterpret_cast<const char*>(data_.data());
	}
};

int save(mat<uint8_t>& img, std::string filename) {
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
		"ENDHDR\n", img.rows(), img.cols());

	os.write(img.rawdata(), img.rawsize());
	
}

int main(void) {
	
	size_t nrows = 256;
	size_t ncols = 256;

	mat<uint8_t> img(nrows, ncols);
	for (size_t r = 0; r < nrows; r++)
	{
		for (size_t c = 0; c < ncols; c++)
		{
			img(r, c) = r;
		}
	}

	save(img, "out.pam");



}