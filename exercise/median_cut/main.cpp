#include <vector>


template <typename T>
struct image
{
	size_t height_;
	size_t width_;
	std::vector<T> data_;

	image(size_t h, size_t w) : height_(h), width_(w), data_(h* w) {}

	const T& operator()(size_t r, size_t c) const {
		return data_[r * height_ + c];
	}

	T& operator()(size_t r, size_t c) {
		return data_[r * height_ + c];
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t dim() {
		return height_ * width_;
	}
};



int main(int argc, char* argv[]) {
	
}