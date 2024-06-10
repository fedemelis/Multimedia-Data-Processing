#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <format>
#include <string>
#include <unordered_map>

int error(const std::string& errmsg) {
	std::cerr << errmsg << std::endl;
	return EXIT_FAILURE;
}

struct table
{
	std::unordered_map<uint8_t, const std::string> map = {
		{0b00000000, "sml_d"},
{0b00000001, "sml_d"},
{0b00000010, "sml_d"},
{0b00000011, "sml_d"},
{0b00000100, "sml_d"},
{0b00000101, "sml_d"},
{0b00000110, "eos"},
{0b00000111, "lrg_d"},
{0b00001000, "sml_d"},
{0b00001001, "sml_d"},
{0b00001010, "sml_d"},
{0b00001011, "sml_d"},
{0b00001100, "sml_d"},
{0b00001101, "sml_d"},
{0b00001110, "nop"},
{0b00001111, "lrg_d"},
{0b00010000, "sml_d"},
{0b00010001, "sml_d"},
{0b00010010, "sml_d"},
{0b00010011, "sml_d"},
{0b00010100, "sml_d"},
{0b00010101, "sml_d"},
{0b00010110, "nop"},
{0b00010111, "lrg_d"},
{0b00011000, "sml_d"},
{0b00011001, "sml_d"},
{0b00011010, "sml_d"},
{0b00011011, "sml_d"},
{0b00011100, "sml_d"},
{0b00011101, "sml_d"},
{0b00011110, "udef"},
{0b00011111, "lrg_d"},
{0b00100000, "sml_d"},
{0b00100001, "sml_d"},
{0b00100010, "sml_d"},
{0b00100011, "sml_d"},
{0b00100100, "sml_d"},
{0b00100101, "sml_d"},
{0b00100110, "udef"},
{0b00100111, "lrg_d"},
{0b00101000, "sml_d"},
{0b00101001, "sml_d"},
{0b00101010, "sml_d"},
{0b00101011, "sml_d"},
{0b00101100, "sml_d"},
{0b00101101, "sml_d"},
{0b00101110, "udef"},
{0b00101111, "lrg_d" },
{0b00110000, "sml_d"},
{0b00110001, "sml_d"},
{0b00110010, "sml_d"},
{0b00110011, "sml_d"},
{0b00110100, "sml_d"},
{0b00110101, "sml_d"},
{0b00110110, "udef"},
{0b00110111, "lrg_d"},
{0b00111000, "sml_d"},
{0b00111001, "sml_d"},
{0b00111010, "sml_d"},
{0b00111011, "sml_d"},
{0b00111100, "sml_d"},
{0b00111101, "sml_d"},
{0b00111110, "udef"},
{0b00111111, "lrg_d"},
{0b01000000, "sml_d"},
{0b01000001, "sml_d"},
{0b01000010, "sml_d"},
{0b01000011, "sml_d"},
{0b01000100, "sml_d"},
{0b01000101, "sml_d"},
{0b01000110, "pre_d"},
{0b01000111, "lrg_d"},
{0b01001000, "sml_d"},
{0b01001001, "sml_d"},
{0b01001010, "sml_d"},
{0b01001011, "sml_d"},
{0b01001100, "sml_d"},
{0b01001101, "sml_d"},
{0b01001110, "pre_d"},
{0b01001111, "lrg_d"},
{0b01010000, "sml_d"},
{0b01010001, "sml_d"},
{0b01010010, "sml_d"},
{0b01010011, "sml_d"},
{0b01010100, "sml_d"},
{0b01010101, "sml_d"},
{0b01010110, "pre_d"},
{0b01010111, "lrg_d"},
{0b01011000, "sml_d"},
{0b01011001, "sml_d"},
{0b01011010, "sml_d"},
{0b01011011, "sml_d"},
{0b01011100, "sml_d"},
{0b01011101, "sml_d"},
{0b01011110, "pre_d"},
{0b01011111, "lrg_d"},
{0b01100000, "sml_d"},
{0b01100001, "sml_d"},
{0b01100010, "sml_d"},
{0b01100011, "sml_d"},
{0b01100100, "sml_d"},
{0b01100101, "sml_d"},
{0b01100110, "pre_d"},
{0b01100111, "lrg_d"},
{0b01101000, "sml_d"},
{0b01101001, "sml_d"},
{0b01101010, "sml_d"},
{0b01101011, "sml_d"},
{0b01101100, "sml_d"},
{0b01101101, "sml_d"},
{0b01101110, "pre_d"},
{0b01101111, "lrg_d"},
{0b01110000, "udef"},
{0b01110001, "udef"},
{0b01110010, "udef"},
{0b01110011, "udef"},
{0b01110100, "udef"},
{0b01110101, "udef"},
{0b01110110, "udef"},
{0b01110111, "udef"},
{0b01111000, "udef"},
{0b01111001, "udef"},
{0b01111010, "udef"},
{0b01111011, "udef"},
{0b01111100, "udef"},
{0b01111101, "udef"},
{0b01111110, "udef"},
{0b01111111, "udef"},
{0b10000000, "sml_d"},
{0b10000001, "sml_d"},
{0b10000010, "sml_d"},
{0b10000011, "sml_d"},
{0b10000100, "sml_d"},
{0b10000101, "sml_d"},
{0b10000110, "pre_d"},
{0b10000111, "lrg_d"},
{0b10001000, "sml_d"},
{0b10001001, "sml_d"},
{0b10001010, "sml_d"},
{0b10001011, "sml_d"},
{0b10001100, "sml_d"},
{0b10001101, "sml_d"},
{0b10001110, "pre_d"},
{0b10001111, "lrg_d"},
{0b10010000, "sml_d"},
{0b10010001, "sml_d"},
{0b10010010, "sml_d"},
{0b10010011, "sml_d"},
{0b10010100, "sml_d"},
{0b10010101, "sml_d"},
{0b10010110, "pre_d"},
{0b10010111, "lrg_d"},
{0b10011000, "sml_d"},
{0b10011001, "sml_d"},
{0b10011010, "sml_d"},
{0b10011011, "sml_d"},
{0b10011100, "sml_d"},
{0b10011101, "sml_d"},
{0b10011110, "pre_d"},
{0b10011111, "lrg_d"},
{0b10100000, "med_d"},
{0b10100001, "med_d"},
{0b10100010, "med_d"},
{0b10100011, "med_d"},
{0b10100100, "med_d"},
{0b10100101, "med_d"},
{0b10100110, "med_d"},
{0b10100111, "med_d"},
{0b10101000, "med_d"},
{0b10101001, "med_d"},
{0b10101010, "med_d"},
{0b10101011, "med_d"},
{0b10101100, "med_d"},
{0b10101101, "med_d"},
{0b10101110, "med_d"},
{0b10101111, "med_d"},
{0b10110000, "med_d"},
{0b10110001, "med_d"},
{0b10110010, "med_d"},
{0b10110011, "med_d"},
{0b10110100, "med_d"},
{0b10110101, "med_d"},
{0b10110110, "med_d"},
{0b10110111, "med_d"},
{0b10111000, "med_d"},
{0b10111001, "med_d"},
{0b10111010, "med_d"},
{0b10111011, "med_d"},
{0b10111100, "med_d"},
{0b10111101, "med_d"},
{0b10111110, "med_d"},
{0b10111111, "med_d"},
{0b11000000, "sml_d"},
{0b11000001, "sml_d"},
{0b11000010, "sml_d"},
{0b11000011, "sml_d"},
{0b11000100, "sml_d"},
{0b11000101, "sml_d"},
{0b11000110, "pre_d"},
{0b11000111, "lrg_d"},
{0b11001000, "sml_d"},
{0b11001001, "sml_d"},
{0b11001010, "sml_d"},
{0b11001011, "sml_d"},
{0b11001100, "sml_d"},
{0b11001101, "sml_d"},
{0b11001110, "pre_d"},
{0b11001111, "lrg_d"},
{0b11010000, "udef"},
{0b11010001, "udef"},
{0b11010010, "udef"},
{0b11010011, "udef"},
{0b11010100, "udef"},
{0b11010101, "udef"},
{0b11010110, "udef"},
{0b11010111, "udef"},
{0b11011000, "udef"},
{0b11011001, "udef"},
{0b11011010, "udef"},
{0b11011011, "udef"},
{0b11011100, "udef"},
{0b11011101, "udef"},
{0b11011110, "udef"},
{0b11011111, "udef"},
{0b11100000, "lrg_l"},
{0b11100001, "sml_l"},
{0b11100010, "sml_l"},
{0b11100011, "sml_l"},
{0b11100100, "sml_l"},
{0b11100101, "sml_l"},
{0b11100110, "sml_l"},
{0b11100111, "sml_l"},
{0b11101000, "sml_l"},
{0b11101001, "sml_l"},
{0b11101010, "sml_l"},
{0b11101011, "sml_l"},
{0b11101100, "sml_l"},
{0b11101101, "sml_l"},
{0b11101110, "sml_l"},
{0b11101111, "sml_l"},
{0b11110000, "lrg_m"},
{0b11110001, "sml_m"},
{0b11110010, "sml_m"},
{0b11110011, "sml_m"},
{0b11110100, "sml_m"},
{0b11110101, "sml_m"},
{0b11110110, "sml_m"},
{0b11110111, "sml_m"},
{0b11111000, "sml_m"},
{0b11111001, "sml_m"},
{0b11111010, "sml_m"},
{0b11111011, "sml_m"},
{0b11111100, "sml_m"},
{0b11111101, "sml_m"},
{0b11111110, "sml_m"},
{0b11111111, "sml_m"},
	};

	const std::string& operator()(uint8_t byte) {
		return map[byte];
	}
};

auto sml_d(uint8_t& byte, std::vector<uint8_t>& decstream, std::ifstream& is, uint16_t& curdistance) {
	
	uint8_t l = (byte & 0b11000000) >> 6;
	uint8_t m = (byte & 0b00111000) >> 3;
	uint16_t d = byte & 0b111;
	d = ((d << 8) | is.get());

	for (size_t i = 0; i < l; i++)
	{
		decstream.push_back(is.get());
	}
	
	for (size_t i = 0; i < m + 3; i++)
	{
		uint8_t c = decstream[decstream.size() - d];
		decstream.push_back(c);
	}

	curdistance = d;

	return EXIT_SUCCESS;
}


auto med_d(uint8_t& byte, std::vector<uint8_t>& decstream, std::ifstream& is, uint16_t& curdistance) {
	uint8_t l = (byte & 0b00011000) >> 3;
	uint8_t m = byte & 0b111;
	uint8_t nextbyte = is.get();
	uint16_t d = (nextbyte & 0b11111100) >> 2;
	m <<= 2;
	m |= (nextbyte & 0b11);
	d |= (is.get() << 6);

	for (size_t i = 0; i < l; i++)
	{
		decstream.push_back(is.get());
	}

	for (size_t i = 0; i < m + 3; i++)
	{
		uint8_t c = decstream[decstream.size() - d];
		decstream.push_back(c);
	}

	curdistance = d;

	return EXIT_SUCCESS;
}

auto lrg_d(uint8_t& byte, std::vector<uint8_t>& decstream, std::ifstream& is, uint16_t& curdistance) {
	uint8_t l = (byte & 0b11000000) >> 6;
	uint8_t m = (byte & 0b00111000) >> 3;
	uint16_t d = is.get();
	d |= (is.get() << 8);
	
	for (size_t i = 0; i < l; i++)
	{
		decstream.push_back(is.get());
	}

	for (size_t i = 0; i < m + 3; i++)
	{
		uint8_t c = decstream[decstream.size() - d];
		decstream.push_back(c);
	}

	curdistance = d;

	return EXIT_SUCCESS;
}

auto pre_d(uint8_t& byte, std::vector<uint8_t>& decstream, std::ifstream& is, uint16_t& curdistance) {
	uint8_t l = (byte & 0b11000000) >> 6;
	uint8_t m = (byte & 0b00111000) >> 3;

	for (size_t i = 0; i < l; i++)
	{
		decstream.push_back(is.get());
	}

	for (size_t i = 0; i < m + 3; i++)
	{
		uint8_t c = decstream[decstream.size() - curdistance];
		decstream.push_back(c);
	}

	return EXIT_SUCCESS;
}

auto sml_m(uint8_t& byte, std::vector<uint8_t>& decstream, std::ifstream& is, uint16_t& curdistance) {
	uint8_t m = byte & 0b1111;

	for (size_t i = 0; i < m; i++)
	{
		uint8_t c = decstream[decstream.size() - curdistance];
		decstream.push_back(c);
	}

	return EXIT_SUCCESS;
}

auto lrg_m(std::vector<uint8_t>& decstream, std::ifstream& is, uint16_t& curdistance) {
	uint8_t m = is.get();

	for (size_t i = 0; i < m + 16; i++)
	{
		uint8_t c = decstream[decstream.size() - curdistance];
		decstream.push_back(c);
	}

	return EXIT_SUCCESS;
}

auto sml_l(uint8_t& byte, std::vector<uint8_t>& decstream, std::ifstream& is) {
	uint8_t l = byte & 0b1111;

	for (size_t i = 0; i < l; i++)
	{
		decstream.push_back(is.get());
	}

	return EXIT_SUCCESS;
}

auto lrg_l(std::vector<uint8_t>& decstream, std::ifstream& is) {
	uint8_t l = is.get();

	for (size_t i = 0; i < l + 16; i++)
	{
		decstream.push_back(is.get());
	}

	return EXIT_SUCCESS;
}


auto decode(const char* inputfile, const char* outputfile) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening input file for decoding");
	}

	std::string magicnumber(4, ' ');
	is.read(reinterpret_cast<char*>(magicnumber.data()), 4);

	if (magicnumber != "bvxn")
	{
		return error(std::format("Magicnumber should have been \"bvxn\", but it was \"{}\"", magicnumber));
	}

	uint32_t decsize = 0, blocksize = 0;
	is.read(reinterpret_cast<char*>(&decsize), sizeof(decsize));
	is.read(reinterpret_cast<char*>(&blocksize), sizeof(blocksize));
	table t;

	std::vector<uint8_t> decstream;
	uint16_t distance = 0;

	while (true)
	{
		uint8_t code = is.get();
		std::string type = t(code);

		//if (decstream.size() > 1750)
		//{
		//	std::cout << "ciao";
		//}
		
		if (type == "sml_d")
		{
			sml_d(code, decstream, is, distance);
		}
		else if (type == "med_d") {
			med_d(code, decstream, is, distance);
		}
		else if (type == "lrg_d") {
			lrg_d(code, decstream, is, distance);
		}
		else if (type == "pre_d") {
			pre_d(code, decstream, is, distance);
		}
		else if (type == "sml_m") {
			sml_m(code, decstream, is, distance);
		}
		else if (type == "lrg_m") {
			lrg_m(decstream, is, distance);
		}
		else if (type == "sml_l") {
			sml_l(code, decstream, is);
		}
		else if (type == "lrg_l") {
			lrg_l(decstream, is);
		}
		else if (type == "nop") {
			//nop
		}
		else if (type == "eos") {
			break;
		}
		else
		{
			return error(std::format("Unexpected code found: {}", code));
		}
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opeing output file for decoding");
	}

	os.write(reinterpret_cast<char*>(decstream.data()), decstream.size());

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	
	if (argc != 3)
	{
		return error("Usage is: lzvn_decode <input file> <output file>");
	}

	return decode(argv[1], argv[2]);

}