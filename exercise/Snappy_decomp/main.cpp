#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

struct bitreader
{
	uint8_t buffer_ = 0;
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

int error(const char* errmsg) {
	std::cerr << errmsg << '\n';
	return EXIT_FAILURE;
}

auto decode(const char* inputfile, const char* outputfile) {
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening input file");
	}

	bitreader br(is);
	uint64_t size = 0;
	int n = 0;

	uint8_t flag = br(1);
	while (flag == 1)
	{
		size |= (br(7) << (n*7));
		++n;
		flag = br(1);
	}
	size |= (br(7) << (n*7));

	std::vector<uint8_t> dec_stream;

	while (dec_stream.size() < size)
	{
		uint8_t pre = is.get();
		uint8_t tag = pre & 0b00000011;
		if (tag == 0)
		{
			uint8_t len = pre >> 2;
			if (len < 60)
			{
				for (size_t i = 0; i < len + 1; i++)
				{
					dec_stream.push_back(is.get());
				}
			}
			else
			{
				len -= 59;
				uint32_t lenght = 0;
				is.read(reinterpret_cast<char*>(&lenght), len);
				for (size_t i = 0; i < lenght + 1; i++)
				{
					dec_stream.push_back(is.get());
				}
			}
		}
		else if (tag == 1) {
			uint8_t len = (pre >> 2) & 0b00000111;
			uint16_t offset = pre >> 5;
			offset <<= 8;
			offset |= is.get();

			for (size_t i = 0; i < len + 4; i++)
			{
				uint8_t val = dec_stream[dec_stream.size() - offset];
				dec_stream.push_back(val);
			}
		}
		else if (tag == 2) {
			uint8_t len = pre >> 2;
			uint16_t offset = 0;
			is.read(reinterpret_cast<char*>(&offset), 2);

			for (size_t i = 0; i < len + 1; i++)
			{
				uint8_t val = dec_stream[dec_stream.size() - offset];
				dec_stream.push_back(val);
			}
		}
		else if (tag == 3) {
			uint8_t len = pre >> 2;
			uint32_t offset = 0;
			is.read(reinterpret_cast<char*>(&offset), 4);

			for (size_t i = 0; i < len + 1; i++)
			{
				uint8_t val = dec_stream[dec_stream.size() - offset];
				dec_stream.push_back(val);
			}
		}
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file");
	}

	os.write(reinterpret_cast<char*>(dec_stream.data()), dec_stream.size());
	return EXIT_SUCCESS;


}

int main(int argc, char** argv) {
	
	if (argc != 3)
	{
		return error("Usage is: snappy_decomp <input file> <output file>");
	}

	return decode(argv[1], argv[2]);

}

