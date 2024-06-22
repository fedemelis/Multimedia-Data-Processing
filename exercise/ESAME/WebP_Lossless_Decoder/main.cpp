#include <cstdint>
#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>

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
		return (buffer_ >> (7 - n_)) & 1;
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

struct head
{
	std::string magicnumber;
	uint32_t chunkLen = 0;
	std::string webp;
	std::string vp8l;
	uint32_t streamSize = 0;
	
	int width = 0;
	int height = 0;
	int alphaUsed = 0;
	int versionNumber = 0;

	head() : magicnumber(4, ' '), webp(4, ' '), vp8l(4, ' ') {}
};

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
		return sizeof(T) * data_.size();
	}

	auto rawdim() {
		return reinterpret_cast<char*>(data_.data());
	}

};

auto readbits(bitreader& br, int nbits) {
	uint64_t val = 0;
	for (size_t i = 0; i < nbits; i++)
	{
		val |= br(1) << i;
	}
	return val;
}

auto normal_code_len(bitreader& br, std::unordered_map < uint64_t, std::pair<size_t, uint32_t>>& map, const char type) {
	readbits(br, 1);
	int numCodeLen = readbits(br, 4) + 4;
	std::vector<int> kCodeLengthCodeOrder{
		17, 18, 0, 1, 2, 3, 4, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};
	std::vector<std::pair<uint32_t, uint32_t>> code_length_code_lengths;
	//code_length_code_lengths.resize(19);
	for (int i = 0; i < numCodeLen; i++) {
		auto v = readbits(br, 3);
		if (v != 0)
		{
			code_length_code_lengths.emplace_back(v, kCodeLengthCodeOrder[i]);
		}
	}
	std::sort(code_length_code_lengths.begin(), code_length_code_lengths.end());
	uint64_t code = 0;
	size_t currentShift = 0;
	for (const auto& [len, sym] : code_length_code_lengths) {
		for (size_t i = 0; i < len - currentShift; i++)
		{
			code <<= 1;
		}
		map[code] = { sym, len };
		currentShift = len;
		++code;
	}

	uint8_t bb = br(1);
	int max_symbol = 0;
	if (bb == 0)
	{
		if (type == 'G')
		{
			max_symbol = 256 + 24;
		}
		else if (type == 'D') {
			max_symbol = 40;
		}
		else
		{
			max_symbol = 256;
		}
	}
	else
	{
		int length_nbits = 2 + 2 * readbits(br, 3);
		max_symbol = 2 + readbits(br, length_nbits);
	}

	std::vector<std::pair<int, int>> lenghts;
	code = 0;
	uint32_t len = 0;
	int dim = 0;
	uint32_t prev = 0;
	while (lenghts.size() < max_symbol)
	{
		code <<= 1;
		code |= br(1);
		++len;
		if (map.contains(code) == true)
		{
			if (map[code].second == len)
			{
				if (map[code].first == 16)
				{
					uint8_t rep = readbits(br, 2) + 3;
					for (size_t i = 0; i < rep; i++)
					{
						lenghts.emplace_back(prev, dim);
						++dim;
					}
				}
				else if (map[code].first == 17)
				{
					uint8_t rep = readbits(br, 3) + 3;
					for (size_t i = 0; i < rep; i++)
					{
						lenghts.emplace_back(0, dim);
						++dim;
					}
				}
				else if (map[code].first == 18) {
					uint8_t rep = readbits(br, 7) + 11;
					for (size_t i = 0; i < rep; i++)
					{
						lenghts.emplace_back(0, dim);
						++dim;

					}
				}
				else
				{
					lenghts.emplace_back(map[code].first, dim);
					if (map[code].first != 0)
					{
						prev = map[code].first;
					}
					dim += 1;
					code = 0;
					len = 0;
				}
				code = 0;
				len = 0;
			}
		}
	}

	code = 0;
	currentShift = 0;
	std::unordered_map < uint64_t, std::pair<size_t, uint32_t>> final_map;
	std::sort(lenghts.begin(), lenghts.end());
	code = 0;
	currentShift = 0;
	for (const auto& [len, sym] : lenghts) {
		if (len != 0)
		{
			for (size_t i = 0; i < len - currentShift; i++)
			{
				code <<= 1;
			}
			final_map[code] = { sym, len };
			currentShift = len;
			++code;
		}
	}
	return final_map;
}

auto simple_code_len(bitreader& br, std::unordered_map<uint8_t, size_t>& alphaLen) {
	readbits(br, 1);
	std::unordered_map<uint64_t, std::pair<size_t, uint8_t>> final_map;
	uint8_t numsym = br(1) + 1;
	uint8_t isFirst8bit = br(1);
	uint8_t s0 = 0;
	s0 = readbits(br, 1 + (7 * isFirst8bit));
	alphaLen[s0] = 1;
	final_map[1] = { s0, 1 };
	if (numsym == 2)
	{
		uint8_t s1 = 0;
		s1 = readbits(br, 8);
		alphaLen[s1] = 1;
		final_map[0] = { s1, 1 };
	}
	return final_map;
}


auto decode(const char* inpufile, const char* outputfile) {

	std::ifstream is(inpufile, std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	head h;
	is.read(reinterpret_cast<char*>(h.magicnumber.data()), 4);
	if (h.magicnumber != "RIFF")
	{
		return EXIT_FAILURE;
	}

	is.read(reinterpret_cast<char*>(&h.chunkLen), 4);
	is.read(reinterpret_cast<char*>(h.webp.data()), 4);
	is.read(reinterpret_cast<char*>(h.vp8l.data()), 4);
	is.read(reinterpret_cast<char*>(&h.streamSize), 4);

	if (is.peek() == 0x2f)
	{
		is.ignore(1);
	}

	bitreader br(is);

	for (size_t i = 0; i < 14; i++)
	{
		h.width |= (br(1) << i);
	}
	++h.width;

	for (size_t i = 0; i < 14; i++)
	{
		h.height |= (br(1) << i);
	}
	++h.height;

	h.alphaUsed = br(1);

	for (size_t i = 0; i < 3; i++)
	{
		h.versionNumber |= (br(1) << i);
	}


	readbits(br, 3);

	std::unordered_map < uint64_t, std::pair<size_t, uint32_t>> green_back_map; // mappa code -> (sym, len)
	auto GBACKmap = normal_code_len(br, green_back_map, 'G');

	std::unordered_map < uint64_t, std::pair<size_t, uint32_t>> red_map;
	auto Rmap = normal_code_len(br, red_map, 'R');

	std::unordered_map < uint64_t, std::pair<size_t, uint32_t>> blue_map;
	auto Bmap = normal_code_len(br, blue_map, 'B');

	std::unordered_map<uint8_t, size_t>alphaLen;
	auto Amap = simple_code_len(br, alphaLen);

	std::unordered_map < uint64_t, std::pair<size_t, uint32_t>> b_distance_map;
	auto DISTmap = normal_code_len(br, b_distance_map, 'D');


	mat<std::array<uint8_t, 4>> img(h.height, h.width);
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			uint64_t code = 0;
			size_t len = 0;
			bool find = false;

			while (!find)
			{
				code <<= 1;
				code |= br(1);
				++len;
				if (GBACKmap.contains(code) == true)
				{
					auto& [sym, lenc] = GBACKmap[code];
					if (lenc == len)
					{
						if (sym <= 256)
						{
							find = true;
							img(r, c)[1] = sym;
						}
						else
						{
							find = true;
							int win = 0;
							int prefixCode = sym - 256;
							if (prefixCode < 4) {
								win = prefixCode + 1;
							}
							int extra_bits = (prefixCode - 2) >> 1;
							int offset = (2 + (prefixCode & 1)) << extra_bits;
							win = offset + readbits(br, extra_bits) + 1;

						}
					}
				}
			}
			find = false;
			len = 0;
			code = 0;
			while (!find)
			{
				code <<= 1;
				code |= br(1);
				++len;
				if (Rmap.contains(code) == true)
				{
					auto& [sym, lenc] = Rmap[code];
					if (lenc == len)
					{
						find = true;
						img(r, c)[0] = sym;
					}
				}
			}
			find = false;
			len = 0;
			code = 0;
			while (!find)
			{
				code <<= 1;
				code |= br(1);
				++len;
				if (Bmap.contains(code) == true)
				{
					auto& [sym, lenc] = Bmap[code];
					if (lenc == len)
					{
						find = true;
						img(r, c)[2] = sym;
					}
				}
			}
			find = false;
			len = 0;
			code = 0;
			while (!find)
			{
				code <<= 1;
				code |= br(1);
				++len;
				if (Amap.contains(code) == true)
				{
					auto& [sym, lenc] = Amap[code];
					if (lenc == len)
					{
						find = true;
						img(r, c)[3] = sym;
					}
				}
			}
			std::cout << "Fatto";
			while (!find)
			{
				code <<= 1;
				code |= br(1);
				++len;
				if (DISTmap.contains(code) == true)
				{
					auto& [sym, lenc] = DISTmap[code];
					if (lenc == len)
					{
						find = true;
						//todo
					}
				}
			}
			
		}
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	
	if (argc != 3)
	{
		return EXIT_FAILURE;
	}


	return decode(argv[1], argv[2]);

}