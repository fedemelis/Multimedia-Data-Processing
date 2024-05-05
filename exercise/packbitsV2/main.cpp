#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

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
	
	uint32_t pos = 0;

	std::vector<uint8_t>copybuf;
	uint8_t copylen = 0;

	uint8_t runchar = 0;
	uint8_t runlen = 0;

	uint8_t c1 = 0;
	uint8_t c2 = 0;

	bool breaked = false;

	c1 = caratteri[pos];
	++pos;
	copybuf.push_back(c1);
	while (pos < file_size)
	{
		c2 = caratteri[pos];
		++pos;
		if (c1 == c2)
		{
			++runlen;
			while ((c1 == c2) && (pos <= file_size) && (runlen <= 127))
			{
				c1 = c2;
				if (pos < file_size)
				{
					c2 = caratteri[pos];
				}
				++runlen;
				++pos;
				copybuf.push_back(c2);
			}
			//quando smettono di leggere roba uguale
			if ((runlen > 2) || (copybuf.size() < 3))
			{
				for (size_t i = 0; i < runlen; i++)
				{
					if (copybuf.size() > 0)
					{
						copybuf.pop_back();
					}
				}
				//scrivi quello che c'è in copybuf
				if (copybuf.size() > 0)
				{
					std::cout << copybuf.size() - 1 << " - > copia in posizione " << pos << '\n';
					os.put(copybuf.size() - 1);
					for (const auto& elem : copybuf) {
						os.put(elem);
					}

				}

				//scrivi la run
				//std::cout << 257 - runlen << '\n';
				std::cout << copybuf.size() - 1 << " - >run in posizione " << pos << '\n';
				os.put(257 - runlen);
				os.put(c1);
				copybuf.erase(copybuf.begin(), copybuf.end());
				runlen = 0;
				copylen = 0;
				--pos;
				c2 = -1;
			}
			else
			{
				copybuf.pop_back();
				copybuf.push_back(c1);
				copybuf.push_back(c2);
				runlen = 0;
			}
		}
		else
		{
			if (copylen <= 127)
			{
				copylen += 1;
				copybuf.push_back(c2);
			}
			else
			{
				if ((copybuf[copylen-1] == copybuf[copylen-2]) && (pos < (file_size-1)))
				{
					if (caratteri[pos + 1] == copybuf[copylen])
					{
						copybuf.pop_back();
						copybuf.pop_back();
						pos -= 2;
					}
				}
				while (copybuf.size() > 128)
				{
					copybuf.pop_back();
					--pos;
				}
				if (copybuf.size() > 0)
				{
					std::cout << copybuf.size() - 1 << " - >copia in posizione" << pos << '\n';
					os.put(copybuf.size() - 1);
					for (const auto& elem : copybuf) {
						os.put(elem);
					}
					copybuf.erase(copybuf.begin(), copybuf.end());
					runlen = 0;
					copylen = 0;
					--pos;
					c2 = -1;
				}
			}
		}
		c1 = c2;
	}
	if ((runlen > 2) || (copybuf.size() < 2))
	{
		for (size_t i = 0; i < runlen; i++)
		{
			if (copybuf.size() > 0)
			{
				copybuf.pop_back();
			}
		}
		//scrivi quello che c'è in copybuf
		if (copybuf.size() > 0)
		{
			os.put(copybuf.size() - 1);
			for (const auto& elem : copybuf) {
				os.put(elem);
			}
			copybuf.erase(copybuf.begin(), copybuf.end());
			runlen = 0;
			copylen = 0;

		}
	}
	else if (copybuf.size() > 0)
	{
		copybuf.pop_back();
		if (copybuf.size() > 0)
		{
			copybuf.pop_back();
		}
		os.put(copybuf.size() - 1);
		for (const auto& elem : copybuf) {
			os.put(elem);
		}
	}
	os.put(128);
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
	return EXIT_SUCCESS;
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
	else if (mode == "d")
	{
		decode(argv[2], argv[3]);
	}

	return EXIT_SUCCESS;
}