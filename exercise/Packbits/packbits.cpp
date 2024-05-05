#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void error(const char* err) {
	std::cout << err << '\n';
}

auto encode(const char* inputfile, const char* outputfile) {
	std::ifstream is(inputfile, std::ios::binary);

	if (!is)
	{
		error("Error while opening input file for encoding");
		return EXIT_FAILURE;
	}

	is.seekg(0, std::ios::end);
	int file_size = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<uint8_t> caratteri(file_size);
	is.read(reinterpret_cast<char*>(caratteri.data()), file_size);

	std::ofstream os(outputfile, std::ios::binary);

	if (!os)
	{
		error("Error while opening output file for encoding");
		return EXIT_FAILURE;
	}

	size_t processed = 0;
	bool first = true;
	uint8_t c1 = 0;
	uint8_t c2 = 0;
	uint8_t L = 0;
	while (processed < file_size)
	{
		if (first)
		{
			c1 = caratteri[processed];
			++processed;
			first = false;
		}
		else
		{
			c1 = c2;
		}
		c2 = caratteri[processed];
		++processed;
		if (c1 == c2)
		{
			L = 255;
			if (processed < file_size)
			{
				c2 = caratteri[processed];
				++processed;
			}
			while ((c1 == c2) && (processed < file_size) && (L > 129))
			{
				--L;
				c2 = caratteri[processed];
				++processed;
			}
			os.put(L);
			os.put(c1);
		}
		else
		{
			L = 0;
			std::vector<uint8_t> buffer;
			buffer.push_back(c1);
			bool entrato = false;
			bool end = false;
			while ((c1 != c2) && (processed <= file_size) && (!end) && (buffer.size() <= 128))
			{
				buffer.push_back(c2);
				c1 = c2;
				if (processed < file_size)
				{
					entrato = true;
					c2 = caratteri[processed];
					++processed;
				}
				else
				{
					entrato = false;
					end = true;
				}
			}
			if (entrato && (processed <= file_size))
			{
				buffer.pop_back();
				--processed;
			}
			if (buffer.size() == 128)
			{
				//--processed;
				c2 = c1;
			}
			L = buffer.size() - 1;
			std::cout << buffer.size() - 1 << '\n';
			os.put(L);
			for (const auto& elem : buffer) {
				os.put(elem);
			}
		}
	}
	L = 128;
	os.put(L);
	return EXIT_SUCCESS;

}

auto decode(const char* inputfile, const char* outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);

	if (!is)
	{
		error("Error while opening input file for decoding");
		return EXIT_FAILURE;
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file for decoding");
		return EXIT_FAILURE;
	}

	uint8_t c = 0;
	while (true)
	{
		c = is.get();
		if (c < 128)
		{
			auto N = c + 1;
			for (size_t i = 0; i < N; i++)
			{
				os.put(is.get());
			}
		}
		else if (c > 128) 
		{
			auto N = 257 - c;
			uint8_t val = is.get();
			for (size_t i = 0; i < N; i++)
			{
				os.put(val);
			}

		}
		else
		{
			break;
		}
	}

}

int main(int argc, char* argv[]) {
	
	if (argc != 4)
	{
		error("Usage is: [c|d] <input file> <output file>");
		return EXIT_FAILURE;
	}

	std::string mode(argv[1]);
	if (mode == "c")
	{
		encode(argv[2], argv[3]);
	}
	else if(mode == "d")
	{
		decode(argv[2], argv[3]);
	}
	

}