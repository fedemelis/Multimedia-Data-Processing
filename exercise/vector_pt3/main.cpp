#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <utility>
#include <crtdbg.h>
#include <iostream>
#include <fstream>
#include <sstream>


namespace mdp {

	template<typename T>
	struct vector {
	private:
		size_t capacity_;
		size_t size_;
		T* data_;
	public:

		friend void swap(mdp::vector<T>& x, mdp::vector<T>& y) {
			using std::swap; // Enables ADL (Argument Dependant Lookup)
			swap(x.capacity_, y.capacity_);
			swap(x.size_, y.size_);
			swap(x.data_, y.data_);
		}

		vector() { //base constructor
			printf("base constructor\n");
			capacity_ = 10;
			size_ = 0;
			data_ = new T[capacity_];
		}
		vector(const vector& other) { // copy constructor
			printf("copy constructor\n");
			capacity_ = other.capacity_;
			size_ = other.size_;
			data_ = new T[capacity_];
			for (size_t i = 0; i < size_; ++i) {
				data_[i] = other.data_[i];
			}
		}
		vector(vector&& other) noexcept { // move constructor
			printf("move constructor\n");
			capacity_ = other.capacity_;
			size_ = other.size_;
			data_ = other.data_;
			other.capacity_ = 0;
			other.size_ = 0;
			other.data_ = nullptr;
		}
		// Copy and Swap Idiom
		vector& operator=(vector rhs) noexcept { // copy assignment operator
			printf("Copy and Swap Idiom\n");
			swap(*this, rhs);
			return *this;
		}

		~vector() {
			if (data_) {
				printf("~destructor\n");
			}
			else {
				printf("~destructor-empty\n");
			}
			delete[](data_);
		}

		void push_back(const T& num) {
			// Resize the array if necessary
			if (size_ == capacity_) {
				capacity_ *= 2;
				auto tmp = new T[capacity_];
				for (size_t i = 0; i < size_; ++i) {
					tmp[i] = data_[i];
				}
				delete[] data_;
				data_ = tmp;
			}
			data_[size_] = num;
			size_++;
		}

		const T& at(size_t i) const {
			assert(i >= 0 && i < size_);
			return data_[i];
		}
		T& at(size_t i) {
			assert(i >= 0 && i < size_);
			return data_[i];
		}

		const T& operator[](size_t i) const {
			return data_[i];
		}
		T& operator[](size_t i) {
			return data_[i];
		}

		size_t size() const {
			return size_;
		}
	};
}

template <typename T>
void print_vector(std::ostream& os, const mdp::vector<T>& x)
{
	for (size_t i = 0; i < x.size(); i++) {
		os << x[i] << '\n';

	}
}

mdp::vector<double> read_vector(const char* filename)
{
	mdp::vector<double> q;

	std::ifstream is(filename /*, std::ios:binary*/);
	if (!is)
	{
		return q;
	}

	double num;
	while (true) {
		//da testo a binario
		is >> num;
		q.push_back(num);
	}
	return q;
}

mdp::vector<int> SimpleVec(const char* filename)
{
	mdp::vector<int> q;
	mdp::vector<int> z;
	z.push_back(1);
	z.push_back(2);
	z.push_back(3);

	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		return z;
	}

	int num;
	while (fscanf(f, "%d", &num) == 1) {
		q.push_back(num);
	}
	fclose(f);
	return q;
}





int main(int argc, char* argv[])
{
	using mdp::vector;

	std::ifstream ifs("input.txt", std::ios::binary);
	if (!ifs)
	{
		return -1;
	}


	char c[1024];
	ifs.read(c, sizeof(c));
	std::cout.write(c, ifs.gcount());

	return 0;



	if (argc != 3) {
		return 1;
	}

	vector<double> v;
	v = read_vector(argv[1]);
	if (v.size() == 0) {
		return 1;
	}

	//v.sort();
	//NB: std::ios::binary
	std::ofstream os(argv[2]/*, std::ios::binary*/);
	if (!os)
	{
		return 1;
	}

	FILE* output_file = fopen(argv[2], "w");
	if (output_file == NULL) {
		return 1;
	}



	print_vector(os, v);
	return 0;
}