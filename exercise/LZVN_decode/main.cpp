#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <map>

void error(const char* errmsg) {
	std::cerr << errmsg << '\n';
}


struct bitreader
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ifstream& is_;
	
	auto readbit() {
		if (n_ == 0)
		{
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	bitreader(std::ifstream& is) : is_(is) {}

	uint64_t operator()(int nbits) {
		uint64_t val = 0;
		for (size_t i = nbits - 1 ; i <= 0; i--)
		{
			val |= readbit();
		}
		return val;
	}
};

auto small_distance(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte, uint64_t& dist) {

	uint32_t L = firstbyte >> 6;
	uint32_t M = (firstbyte & 0b00111000) >> 3;
	uint16_t Dh = firstbyte & 0b0000000000000111;

	uint16_t Dl = is.get();

	auto next = is.peek();

	Dh = (Dh << 8) | Dl;
	dist = Dh;


	for (size_t i = 0; i < L; i++)
	{
		outstream.push_back(is.get());
	}

	for (size_t i = 0; i < M + 3; i++)
	{
		auto ch = outstream[outstream.size() - dist];
		outstream.push_back(ch);
	}
}


auto small_literal(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte) {
	uint8_t L = firstbyte & 0b00001111;

	for (size_t i = 0; i < L; i++)
	{
		outstream.push_back(is.get());
	}
}


auto small_match(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte, uint64_t& dist) {
	
	uint8_t M = firstbyte & 0b00001111;

	for (size_t i = 0; i < M; i++)
	{
		auto ch = outstream[outstream.size() - dist];
		outstream.push_back(ch);
	}

}


auto large_literal(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte) {
	
	uint16_t L = is.get();

	for (size_t i = 0; i < L + 16; i++)
	{
		outstream.push_back(is.get());
	}

}

auto large_match(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte, uint64_t& dist) {

	uint16_t M = is.get();

	for (size_t i = 0; i < M + 16; i++)
	{
		auto ch = outstream[outstream.size() - dist];
		outstream.push_back(ch);
	}

}


auto previous_distance(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte, uint64_t& dist) {

	uint32_t L = firstbyte >> 6;
	uint32_t M = (firstbyte & 0b00111000) >> 3;

	for (size_t i = 0; i < L; i++)
	{
		outstream.push_back(is.get());
	}

	for (size_t i = 0; i < M + 3; i++)
	{
		auto ch = outstream[outstream.size() - dist];
		outstream.push_back(ch);
	}
}

auto large_distance(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte, uint64_t& dist) {

	uint32_t L = firstbyte >> 6;
	uint32_t M = (firstbyte & 0b00111000) >> 3;
	
	uint8_t Dh = 0, Dl = 0;

	Dh = is.get();
	Dl = is.get();

	/*std::ofstream os("tmpout.txt", std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}
	os.write(reinterpret_cast<char*>(outstream.data()), outstream.size());
	os.close();
	return 0;*/

	uint16_t tmpdist = 0;
	tmpdist |= Dl;
	tmpdist <<= 8;
	tmpdist |= Dh;

	dist = tmpdist;

	for (size_t i = 0; i < L; i++)
	{
		outstream.push_back(is.get());
	}

	for (size_t i = 0; i < M + 3; i++)
	{
		auto ch = outstream[outstream.size() - dist];
		outstream.push_back(ch);
	}
}

auto medium_distance(std::vector<uint8_t>& outstream, std::ifstream& is, uint8_t firstbyte, uint64_t& dist) {

	uint32_t L = (firstbyte & 0b00011000) >> 3;
	uint32_t Mh = firstbyte & 0b00000111;

	uint8_t secondbyte = is.get();

	uint8_t Ml = secondbyte & 0b00000011;
	uint8_t Dh = secondbyte >> 2;
	uint8_t Dl = is.get();

	uint8_t M = Mh << 2;
	M |= (Ml & 0b00000011);

	uint16_t tmpdist = 0;
	tmpdist |= Dl;
	tmpdist <<= 6;
	tmpdist |= (Dh & 0b00111111);

	dist = tmpdist;

	for (size_t i = 0; i < L; i++)
	{
		outstream.push_back(is.get());
	}

	for (size_t i = 0; i < M + 3; i++)
	{
		auto ch = outstream[outstream.size() - dist];
		outstream.push_back(ch);
	}
}


auto decode(const char* inputfile, const char* outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return EXIT_FAILURE;
	}

	std::map<uint8_t, std::string> lookup{
		{0b00000000,	"sml_d"},
		{0b00000001,	"sml_d"},
		{0b00000010,	"sml_d"},
		{0b00000011,	"sml_d"},
		{0b00000100,	"sml_d"},
		{0b00000101,	"sml_d"},
		{0b00000110,	"eos"},
		{0b00000111,	"lrg_d"},
		{0b00001000,	"sml_d"},
		{0b00001001,	"sml_d"},
		{0b00001010,	"sml_d"},
		{0b00001011,	"sml_d"},
		{0b00001100,	"sml_d"},
		{0b00001101,	"sml_d"},
		{0b00001110,	"nop"},
		{0b00001111,	"lrg_d"},
		{0b00010000,	"sml_d"},
		{0b00010001,	"sml_d"},
		{0b00010010,	"sml_d"},
		{0b00010011,	"sml_d"},
		{0b00010100,	"sml_d"},
		{0b00010101,	"sml_d"},
		{0b00010110,	"nop"},
		{0b00010111,	"lrg_d"},
		{0b00011000,	"sml_d"},
		{0b00011001,	"sml_d"},
		{0b00011010,	"sml_d"},
		{0b00011011,	"sml_d"},
		{0b00011100,	"sml_d"},
		{0b00011101,	"sml_d"},
		{0b00011110,	"udef"},
		{0b00011111,	"lrg_d"},
		{0b00100000,	"sml_d"},
		{0b00100001,	"sml_d"},
		{0b00100010,	"sml_d"},
		{0b00100011,	"sml_d"},
		{0b00100100,	"sml_d"},
		{0b00100101,	"sml_d"},
		{0b00100110,	"udef"},
		{0b00100111,	"lrg_d"},
		{0b00101000,	"sml_d"},
		{0b00101001,	"sml_d"},
		{0b00101010,	"sml_d"},
		{0b00101011,	"sml_d"},
		{0b00101100,	"sml_d"},
		{0b00101101,	"sml_d"},
		{0b00101110,	"udef"},
		{0b00101111,	"lrg_d"},
		{0b00110000,	"sml_d"},
		{0b00110001,	"sml_d"},
		{0b00110010,	"sml_d"},
		{0b00110011,	"sml_d"},
		{0b00110100,	"sml_d"},
		{0b00110101,	"sml_d"},
		{0b00110110,	"udef"},
		{0b00110111,	"lrg_d"},
		{0b00111000,	"sml_d"},
		{0b00111001,	"sml_d"},
		{0b00111010,	"sml_d"},
		{0b00111011,	"sml_d"},
		{0b00111100,	"sml_d"},
		{0b00111101,	"sml_d"},
		{0b00111110,	"udef"},
		{0b00111111,	"lrg_d"},
		{0b01000000,	"sml_d"},
		{0b01000001,	"sml_d"},
		{0b01000010,	"sml_d"},
		{0b01000011,	"sml_d"},
		{0b01000100,	"sml_d"},
		{0b01000101,	"sml_d"},
		{0b01000110,	"pre_d"},
		{0b01000111,	"lrg_d"},
		{0b01001000,	"sml_d"},
		{0b01001010,	"sml_d"},
		{0b01001001,	"sml_d"},
		{0b01001010,	"sml_d"},
		{0b01001011,	"sml_d"},
		{0b01001100,	"sml_d"},
		{0b01001101,	"sml_d"},
		{0b01001110,	"pre_d"},
		{0b01001111,	"lrg_d"},
		{0b01010000,	"sml_d"},
		{0b01010001,	"sml_d"},
		{0b01010010,	"sml_d"},
		{0b01010011,	"sml_d"},
		{0b01010100,	"sml_d"},
		{0b01010101,	"sml_d"},
		{0b01010110,	"pre_d"},
		{0b01010111,	"lrg_d"},
		{0b01011000,	"sml_d"},
		{0b01011001,	"sml_d"},
		{0b01011010,	"sml_d"},
		{0b01011011,	"sml_d"},
		{0b01011100,	"sml_d"},
		{0b01011101,	"sml_d"},
		{0b01011110,	"pre_d"},
		{0b01011111,	"lrg_d"},
		{0b01100000,	"sml_d"},
		{0b01100001,	"sml_d"},
		{0b01100010,	"sml_d"},
		{0b01100011,	"sml_d"},
		{0b01100100,	"sml_d"},
		{0b01100101,	"sml_d"},
		{0b01100110,	"pre_d"},
		{0b01100111,	"lrg_d"},
		{0b01101000,	"sml_d"},
		{0b01101001,	"sml_d"},
		{0b01101010,	"sml_d"},
		{0b01101011,	"sml_d"},
		{0b01101100,	"sml_d"},
		{0b01101101,	"sml_d"},
		{0b01101110,	"pre_d"},
		{0b01101111,	"lrg_d"},
		{0b01110000,	"udef "},
		{0b01110001,	"udef "},
		{0b01110010,	"udef "},
		{0b01110011,	"udef "},
		{0b01110100,	"udef "},
		{0b01110101,	"udef "},
		{0b01110110,	"udef "},
		{0b01110111,	"udef "},
		{0b01111000,	"udef "},
		{0b01111001,	"udef "},
		{0b01111010,	"udef "},
		{0b01111011,	"udef "},
		{0b01111100,	"udef "},
		{0b01111101,	"udef "},
		{0b01111110,	"udef "},
		{0b01111111,	"udef "},
		{0b10000000,	"sml_d"},
		{0b10000001,	"sml_d"},
		{0b10000010,	"sml_d"},
		{0b10000011,	"sml_d"},
		{0b10000100,	"sml_d"},
		{0b10000101,	"sml_d"},
		{0b10000110,	"pre_d"},
		{0b10000111,	"lrg_d"},
		{0b10001000,	"sml_d"},
		{0b10001001,	"sml_d"},
		{0b10001010,	"sml_d"},
		{0b10001011,	"sml_d"},
		{0b10001100,	"sml_d"},
		{0b10001101,	"sml_d"},
		{0b10001110,	"pre_d"},
		{0b10001111,	"lrg_d"},
		{0b10010000,	"sml_d"},
		{0b10010001,	"sml_d"},
		{0b10010010,	"sml_d"},
		{0b10010011,	"sml_d"},
		{0b10010100,	"sml_d"},
		{0b10010101,	"sml_d"},
		{0b10010110,	"pre_d"},
		{0b10010111,	"lrg_d"},
		{0b10011000,	"sml_d"},
		{0b10011001,	"sml_d"},
		{0b10011010,	"sml_d"},
		{0b10011011,	"sml_d"},
		{0b10011100,	"sml_d"},
		{0b10011101,	"sml_d"},
		{0b10011110,	"pre_d"},
		{0b10011111,	"lrg_d"},
		{0b10100000,	"med_d"},
		{0b10100001,	"med_d"},
		{0b10100010,	"med_d"},
		{0b10100011,	"med_d"},
		{0b10100100,	"med_d"},
		{0b10100101,	"med_d"},
		{0b10100110,	"med_d"},
		{0b10100111,	"med_d"},
		{0b10101000,	"med_d"},
		{0b10101001,	"med_d"},
		{0b10101010,	"med_d"},
		{0b10101011,	"med_d"},
		{0b10101100,	"med_d"},
		{0b10101101,	"med_d"},
		{0b10101110,	"med_d"},
		{0b10101111,	"med_d"},
		{0b10110000,	"med_d"},
		{0b10110001,	"med_d"},
		{0b10110010,	"med_d"},
		{0b10110011,	"med_d"},
		{0b10110100,	"med_d"},
		{0b10110101,	"med_d"},
		{0b10110110,	"med_d"},
		{0b10110111,	"med_d"},
		{0b10111000,	"med_d"},
		{0b10111001,	"med_d"},
		{0b10111010,	"med_d"},
		{0b10111011,	"med_d"},
		{0b10111100,	"med_d"},
		{0b10111101,	"med_d"},
		{0b10111110,	"med_d"},
		{0b10111111,	"med_d"},
		{0b11000000,	"sml_d"},
		{0b11000001,	"sml_d"},
		{0b11000010,	"sml_d"},
		{0b11000011,	"sml_d"},
		{0b11000100,	"sml_d"},
		{0b11000101,	"sml_d"},
		{0b11000110,	"pre_d"},
		{0b11000111,	"lrg_d"},
		{0b11001000,	"sml_d"},
		{0b11001001,	"sml_d"},
		{0b11001010,	"sml_d"},
		{0b11001011,	"sml_d"},
		{0b11001100,	"sml_d"},
		{0b11001101,	"sml_d"},
		{0b11001110,	"pre_d"},
		{0b11001111,	"lrg_d"},
		{0b11010000,	"udef "},
		{0b11010001,	"udef "},
		{0b11010010,	"udef "},
		{0b11010011,	"udef "},
		{0b11010100,	"udef "},
		{0b11010101,	"udef "},
		{0b11010110,	"udef "},
		{0b11010111,	"udef "},
		{0b11011000,	"udef "},
		{0b11011001,	"udef "},
		{0b11011010,	"udef "},
		{0b11011011,	"udef "},
		{0b11011100,	"udef "},
		{0b11011101,	"udef "},
		{0b11011110,	"udef "},
		{0b11011111,	"udef "},
		{0b11100000,	"lrg_l"},
		{0b11100001,	"sml_l"},
		{0b11100010,	"sml_l"},
		{0b11100011,	"sml_l"},
		{0b11100100,	"sml_l"},
		{0b11100101,	"sml_l"},
		{0b11100110,	"sml_l"},
		{0b11100111,	"sml_l"},
		{0b11101000,	"sml_l"},
		{0b11101001,	"sml_l"},
		{0b11101010,	"sml_l"},
		{0b11101011,	"sml_l"},
		{0b11101100,	"sml_l"},
		{0b11101101,	"sml_l"},
		{0b11101110,	"sml_l"},
		{0b11101111,	"sml_l"},
		{0b11110000,	"lrg_m"},
		{0b11110001,	"sml_m"},
		{0b11110010,	"sml_m"},
		{0b11110011,	"sml_m"},
		{0b11110100,	"sml_m"},
		{0b11110101,	"sml_m"},
		{0b11110110,	"sml_m"},
		{0b11110111,	"sml_m"},
		{0b11111000,	"sml_m"},
		{0b11111001,	"sml_m"},
		{0b11111010,	"sml_m"},
		{0b11111011,	"sml_m"},
		{0b11111100,	"sml_m"},
		{0b11111101,	"sml_m"},
		{0b11111110,	"sml_m"},
		{0b11111111,	"sml_m"}
		};

	std::string magicnumber(4, ' ');
	std::string end(4, ' ');
	is.read(reinterpret_cast<char*>(magicnumber.data()), 4);
	if (magicnumber != "bvxn")
	{
		error("Magicnumber should be \"bvxn\"");
		return EXIT_FAILURE;
	}

	uint32_t outputsize = 0, inputsize = 0;
	is.read(reinterpret_cast<char*>(&outputsize), sizeof(uint32_t));
	is.read(reinterpret_cast<char*>(&inputsize), sizeof(uint32_t));

	std::vector<uint8_t> outputstream;
	uint64_t dist = 0;

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}

	while (end != "bvx$")
	{
		//if (outputstream.size() >= 2090)
		//{
		//	std::cout << "ciao";
		//	/*os.write(reinterpret_cast<char*>(outputstream.data()), outputstream.size());
		//	os.close();
		//	break;*/
		//}
		uint8_t buffer = is.get();
		std::string opcode;
		opcode = lookup[buffer];
		//std::cout << opcode << '\n';
		if (opcode == "sml_d")
		{
			small_distance(outputstream, is, buffer, dist);
		}
		else if (opcode == "med_d") {
			medium_distance(outputstream, is, buffer, dist);
		}
		else if (opcode == "lrg_d") {
			large_distance(outputstream, is, buffer, dist);
		}
		else if (opcode == "pre_d") {
			previous_distance(outputstream, is, buffer, dist);
		}
		else if (opcode == "sml_m") {
			small_match(outputstream, is, buffer, dist);
		}
		else if (opcode == "lrg_m") {
			large_match(outputstream, is, buffer, dist);
		}
		else if (opcode == "sml_l") {
			small_literal(outputstream, is, buffer);
		}
		else if (opcode == "lrg_l") {
			large_literal(outputstream, is, buffer);
		}
		else if (opcode == "nop") {

		}
		else if (opcode == "udef" || opcode == "udef ") {

		}
		else {
			for (size_t i = 0; i < 7; i++)
			{
				is.get();
			}
			is >> end;
		}
	}

	//while (end != "bvx$")
	//{
	//	uint8_t buffer = is.get();
	//	if ((buffer & 0b11100000) == 0b1010000) // medium distance
	//	{
	//		medium_distance(outputstream, is, buffer, dist);
	//	}
 	//	else if (((buffer & 0b00000111) == 0b00000111) && (buffer != 0b11111111)){ // large distance 
	//		large_distance(outputstream, is, buffer, dist);
    //	}
	//	else if (((buffer & 0b00000111) == 0b110) && (buffer != 0b00000110) && ((buffer & 0b11110000) >> 4) != 0b1110) { // previous distance 
	//		previous_distance(outputstream, is, buffer, dist);
	//	}
	//	else if (((buffer >> 4) & 0b1111) == 0b1111) { // match 
	//		if (buffer == 0b11110000) // large match
	//		{
	//			large_match(outputstream, is, buffer, dist);
	//		}
	//		else // small match
	//		{
	//			small_match(outputstream, is, buffer, dist);
	//		}
	//	}
	//	else if (((buffer >> 5) & 0b111) == 0b111) { // literal 
	//		if (buffer == 0b11100000) // large literal
	//		{
	//			large_literal(outputstream, is, buffer);
	//		}
	//		else // small literal
	//		{
	//			small_literal(outputstream, is, buffer);
	//		}
	//	}
	//	else if ((buffer == 0b00001110) || (buffer == 0b00010110)) {
	//		// pass
	//	}
	//	else if (buffer == 0b00000110) { // end of stream
	//		for (size_t i = 0; i < 7; i++)
	//		{
	//			is.get();
	//		}
	//		is >> end;
	//	}
	//	else // small distance
	//	{
	//		small_distance(outputstream, is, buffer, dist);
	//	}
	//}

	/*std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}*/

	os.write(reinterpret_cast<char*>(outputstream.data()), outputstream.size());

}


int main(int argc, char* argv[]) {

	if (argc != 3)
	{
		error("Usage is lzvn_decode <input file> <output file>");
		return EXIT_FAILURE;
	}

	decode(argv[1], argv[2]);

	
	
}