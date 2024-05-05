#pragma once
#include <algorithm>
#include <ostream>

class Bitwriter
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ostream& os_;
	void writebit(uint64_t bit);

	void writebit(uint64_t bit) {
		buffer_ <<= 1;
		buffer_ |= bit;
		++n_;
		if (n_ == 8)
		{
			os_.put(buffer_);
			n_ = 0;
		}
	}

public:
	Bitwriter(std::ostream& os) : os_(os) {}

	~Bitwriter() {
		flush();
	}

	void flush() {
		while (n_ > 0)
		{
			writebit(0);
		}
	}

	void operator()(uint64_t val, uint64_t nbits) {
		while (nbits --> 0)
		{
			writebit((val >> nbits) & 1);
		}
	}

	uint64_t elias_nbits_encoding(uint64_t x) {
		auto nbits = std::bit_width(x);
		return nbits * 2 - 1;
	}
};

