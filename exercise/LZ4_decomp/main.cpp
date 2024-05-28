#include <algorithm>
#include <cstdint>
#include <vector>
#include <fstream>
#include <iostream>

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

auto decode(const char* inputfile, const char* outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return 1;
	}

	is.seekg(0, std::ios::end);
	uint64_t size = is.tellg();
	is.seekg(0, std::ios::beg);

	uint32_t magicnumber = 0, dimension = 0, trash = 0;
	is.read(reinterpret_cast<char*>(&magicnumber), sizeof(uint32_t));

	if (magicnumber != 0x184c2103)
	{
		return 1;
	}

	is.read(reinterpret_cast<char*>(&dimension), sizeof(uint32_t));

	is.read(reinterpret_cast<char*>(&trash), sizeof(uint32_t));

	std::vector<uint8_t> decStream;
	bitreader br(is);
	uint32_t readed = 0;

	readed += sizeof(uint32_t) * 3;

	int current_block_pos = -1;
	uint32_t block_dim = 0;

	while (is.peek() != EOF)
	{

		if (!((size - readed) <= 6))
		{
			if ((current_block_pos == -1) || (current_block_pos >= block_dim))
			{
				block_dim = 0;
				is.read(reinterpret_cast<char*>(&block_dim), sizeof(uint32_t));
				readed += sizeof(uint32_t);
				current_block_pos = 0;
			}
			uint64_t literal_len = 0, match_len = 0;
			literal_len = br(4);
			match_len = br(4);
			++current_block_pos;
			++readed;

			if (literal_len == 15)
			{
				uint8_t nextdim = is.get();
				++readed;
				++current_block_pos;
				while (nextdim == 255)
				{
					literal_len += nextdim;
					nextdim = is.get();
					++readed;
					++current_block_pos;
				}
				literal_len += nextdim;
			}

			for (size_t i = 0; i < literal_len; i++)
			{
				decStream.push_back(is.get());
				++readed;
				++current_block_pos;
			}

			if (current_block_pos != block_dim)
			{
				uint16_t offset = 0;
				is.read(reinterpret_cast<char*>(&offset), sizeof(uint16_t));
				readed += sizeof(uint16_t);
				current_block_pos += 2;

				if (match_len == 15)
				{
					uint8_t nextdim = is.get();
					++current_block_pos;
					++readed;
					while (nextdim == 255)
					{
						match_len += nextdim;
						nextdim = is.get();
						++current_block_pos;
						++readed;
					}
					match_len += nextdim;
				}

				for (size_t i = 0; i < match_len + 4; i++)
				{
					uint8_t c = decStream[decStream.size() - offset];
					//std::cout << c;
					decStream.push_back(c);
				}
			}
		}
		else
		{
			is.get(); //removing the last 0101 0000 (this is the token)
			for (size_t i = 0; i < 5; i++)
			{
				decStream.push_back(is.get());
				++readed;
			}
			break;
		}	
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return 1;
	}

	os.write(reinterpret_cast<char*>(decStream.data()), decStream.size());

	return 0;
}

int main(int argc, char* argv[]) {
	
	if (argc != 3)
	{
		return 1;
	}

	return decode(argv[1], argv[2]);

}