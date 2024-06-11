#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
#include <fstream>

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


struct bitwriter
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ofstream& os_;

	void writebit(uint8_t bit) {
		buffer_ <<= 1;
		buffer_ |= bit;
		++n_;
		if (n_ == 8)
		{
			os_.put(buffer_);
			n_ = 0;
		}
	}

	bitwriter(std::ofstream& os) : os_(os) {}

	~bitwriter() {
		flush();
	}

	void operator()(uint64_t val, int numbits) {
		for (int i = numbits - 1; i >= 0; i--)
		{
			writebit((val >> i) & 1);
		}
	}

	void flush() {
		while (n_ != 0)
		{
			writebit(0);
		}
	}
};


int main(int argc, char** argv) {
	


}