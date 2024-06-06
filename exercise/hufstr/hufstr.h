#pragma once
#ifndef DATE_H
#define DATE_H
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <memory>

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
			writebit(1);
		}
	}

};


class hufstr {
public:
	std::unordered_map<uint8_t, uint32_t>freq;
	std::unordered_map<uint8_t, std::pair<uint8_t, uint32_t>> compress_map{};
	std::unordered_map<uint32_t, std::pair<uint8_t, uint8_t>> decompress_map{};
    std::vector<uint8_t> compress(const std::string& s) const;
    std::string decompress(const std::vector<uint8_t>& v) const;
    hufstr();

private:
};

#endif