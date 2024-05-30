#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

int error(const char* err) {
	std::cerr << err << std::endl;
	return EXIT_FAILURE;
}

struct bitreader
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ifstream& is_;

	uint8_t readbit(){
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
		for (int i = numbits - 1; i >= 0 ; i--)
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

	void operator()(uint64_t val, int nbits) {
		for (int i = nbits-1; i >= 0; i--)
		{
			writebit((val >> i) & 1);
		}
	}

	~bitwriter() {
		flush();
	}

	void flush() {
		while (n_ != 0)
		{
			writebit(0);
		}
	}
};

auto to_base85(uint64_t v, std::string& s) {
	for (size_t i = 0; i < 5; i++)
	{
		auto x = v % 85;
		std::string ss(1, x);
		s.append(ss);
		v /= 85;
	}
	std::reverse(s.begin(), s.end());
}

auto to_base10(std::string& s) {
	uint32_t res = 0;
	for (size_t i = 0; i < 5; i++)
	{
		res += (s[i] * std::pow(85, i));
	}
	return res;
}

auto rotate(int N, std::string& s, int start) {

	std::unordered_map<uint8_t, uint8_t> ascii85 = {
		{0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
		{10, 'a'}, {11, 'b'}, {12, 'c'}, {13, 'd'}, {14, 'e'}, {15, 'f'}, {16, 'g'}, {17, 'h'}, {18, 'i'}, {19, 'j'},
		{20, 'k'}, {21, 'l'}, {22, 'm'}, {23, 'n'}, {24, 'o'}, {25, 'p'}, {26, 'q'}, {27, 'r'}, {28, 's'}, {29, 't'},
		{30, 'u'}, {31, 'v'}, {32, 'w'}, {33, 'x'}, {34, 'y'}, {35, 'z'}, {36, 'A'}, {37, 'B'}, {38, 'C'}, {39, 'D'},
		{40, 'E'}, {41, 'F'}, {42, 'G'}, {43, 'H'}, {44, 'I'}, {45, 'J'}, {46, 'K'}, {47, 'L'}, {48, 'M'}, {49, 'N'},
		{50, 'O'}, {51, 'P'}, {52, 'Q'}, {53, 'R'}, {54, 'S'}, {55, 'T'}, {56, 'U'}, {57, 'V'}, {58, 'W'}, {59, 'X'},
		{60, 'Y'}, {61, 'Z'}, {62, '.'}, {63, '-'}, {64, ':'}, {65, '+'}, {66, '='}, {67, '^'}, {68, '!'}, {69, '/'},
		{70, '*'}, {71, '?'}, {72, '&'}, {73, '<'}, {74, '>'}, {75, '('}, {76, ')'}, {77, '['}, {78, ']'}, {79, '{'},
		{80, '}'}, {81, '@'}, {82, '%'}, {83, '$'}, {84, '#'}
	};

	int r = start;
	for (size_t i = 0; i < 5; i++)
	{
		if ((s[i] - r) < 0)
		{
			auto x = s[i] + 85 - r;
			s[i] = ascii85[x];
		}
		else
		{
			auto x = s[i] - r;
			s[i] = ascii85[x];
		}
		r += N;
		if (r > 84)
		{
			r -= 85;
		}
	}
	return r;
}

auto backRotate(int N, std::string& s, int start) {

	std::unordered_map<uint8_t, uint8_t> ascii85 = {
		{0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
		{10, 'a'}, {11, 'b'}, {12, 'c'}, {13, 'd'}, {14, 'e'}, {15, 'f'}, {16, 'g'}, {17, 'h'}, {18, 'i'}, {19, 'j'},
		{20, 'k'}, {21, 'l'}, {22, 'm'}, {23, 'n'}, {24, 'o'}, {25, 'p'}, {26, 'q'}, {27, 'r'}, {28, 's'}, {29, 't'},
		{30, 'u'}, {31, 'v'}, {32, 'w'}, {33, 'x'}, {34, 'y'}, {35, 'z'}, {36, 'A'}, {37, 'B'}, {38, 'C'}, {39, 'D'},
		{40, 'E'}, {41, 'F'}, {42, 'G'}, {43, 'H'}, {44, 'I'}, {45, 'J'}, {46, 'K'}, {47, 'L'}, {48, 'M'}, {49, 'N'},
		{50, 'O'}, {51, 'P'}, {52, 'Q'}, {53, 'R'}, {54, 'S'}, {55, 'T'}, {56, 'U'}, {57, 'V'}, {58, 'W'}, {59, 'X'},
		{60, 'Y'}, {61, 'Z'}, {62, '.'}, {63, '-'}, {64, ':'}, {65, '+'}, {66, '='}, {67, '^'}, {68, '!'}, {69, '/'},
		{70, '*'}, {71, '?'}, {72, '&'}, {73, '<'}, {74, '>'}, {75, '('}, {76, ')'}, {77, '['}, {78, ']'}, {79, '{'},
		{80, '}'}, {81, '@'}, {82, '%'}, {83, '$'}, {84, '#'}
	};

	std::unordered_map<uint8_t, uint8_t> ascii85_map = {
		{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9},
		{'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15}, {'g', 16}, {'h', 17}, {'i', 18}, {'j', 19},
		{'k', 20}, {'l', 21}, {'m', 22}, {'n', 23}, {'o', 24}, {'p', 25}, {'q', 26}, {'r', 27}, {'s', 28}, {'t', 29},
		{'u', 30}, {'v', 31}, {'w', 32}, {'x', 33}, {'y', 34}, {'z', 35}, {'A', 36}, {'B', 37}, {'C', 38}, {'D', 39},
		{'E', 40}, {'F', 41}, {'G', 42}, {'H', 43}, {'I', 44}, {'J', 45}, {'K', 46}, {'L', 47}, {'M', 48}, {'N', 49},
		{'O', 50}, {'P', 51}, {'Q', 52}, {'R', 53}, {'S', 54}, {'T', 55}, {'U', 56}, {'V', 57}, {'W', 58}, {'X', 59},
		{'Y', 60}, {'Z', 61}, {'.', 62}, {'-', 63}, {':', 64}, {'+', 65}, {'=', 66}, {'^', 67}, {'!', 68}, {'/', 69},
		{'*', 70}, {'?', 71}, {'&', 72}, {'<', 73}, {'>', 74}, {'(', 75}, {')', 76}, {'[', 77}, {']', 78}, {'{', 79},
		{'}', 80}, {'@', 81}, {'%', 82}, {'$', 83}, {'#', 84}
	};

	int r = start;
	for (size_t i = 0; i < 5; i++)
	{
		if ((s[i] + r) > 84)
		{
			auto x = s[i] - (85 * ((r / 85) + 1)) + r;
			s[i] = ascii85[x];
			s[i] = ascii85_map[s[i]];
		}
		else
		{
			auto x = s[i] + r;
			s[i] = ascii85[x];
			s[i] = ascii85_map[s[i]];
		}
		r += N;
		if (r > 84)
		{
			r -= 85;
		}
	}
	std::reverse(s.begin(), s.end());
	return 0;
}

auto demapping(std::string& s) {

	std::unordered_map<uint8_t, uint8_t> ascii85_map = {
		{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9},
		{'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15}, {'g', 16}, {'h', 17}, {'i', 18}, {'j', 19},
		{'k', 20}, {'l', 21}, {'m', 22}, {'n', 23}, {'o', 24}, {'p', 25}, {'q', 26}, {'r', 27}, {'s', 28}, {'t', 29},
		{'u', 30}, {'v', 31}, {'w', 32}, {'x', 33}, {'y', 34}, {'z', 35}, {'A', 36}, {'B', 37}, {'C', 38}, {'D', 39},
		{'E', 40}, {'F', 41}, {'G', 42}, {'H', 43}, {'I', 44}, {'J', 45}, {'K', 46}, {'L', 47}, {'M', 48}, {'N', 49},
		{'O', 50}, {'P', 51}, {'Q', 52}, {'R', 53}, {'S', 54}, {'T', 55}, {'U', 56}, {'V', 57}, {'W', 58}, {'X', 59},
		{'Y', 60}, {'Z', 61}, {'.', 62}, {'-', 63}, {':', 64}, {'+', 65}, {'=', 66}, {'^', 67}, {'!', 68}, {'/', 69},
		{'*', 70}, {'?', 71}, {'&', 72}, {'<', 73}, {'>', 74}, {'(', 75}, {')', 76}, {'[', 77}, {']', 78}, {'{', 79},
		{'}', 80}, {'@', 81}, {'%', 82}, {'$', 83}, {'#', 84}
	};

	for (auto& elem : s) {
		elem = ascii85_map[elem];
	}

	//std::reverse(s.begin(), s.end());

}


auto compress(int N, const char* inputfile, const char* outputfile) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening input file during compression");
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file during compression");
	}

	std::string magicnumber(2, ' ');
	is >> magicnumber;
	if (magicnumber != "P6")
	{
		return error("Input image should be ppm image");
	}
	uint64_t width = 0, height = 0;
	uint64_t maxval = 0;

	is.get(); //remove
	if (is.peek() == '#')
	{
		is.get();
		while (is.peek() != '\n')
		{
			is >> magicnumber;
		}
		is.get(); // remove
	}
	is >> width;
	is >> height;

	is >> maxval;
	if (is.peek() == '\n')
	{
		is.get();
	}

	int pad = (width * height) % 4;

	int start = 0;
	os << width << ',' << height << ',';
	bitreader br(is);
	int padding = 0;
	if (pad != 0)
	{
		padding = 1;
	}
	for (size_t i = 0; i < ((3 * (width * height)) / 4) + padding; i++)
	{
		uint32_t buf = 0;
		if (i == ((3 * (width * height)) / 4))
		{
			buf = br(pad * 8);
			buf <<= (4 - pad) * 8;
		}
		else
		{
			buf = br(32);
		}

		std::string b85;
		to_base85(buf, b85);

		rotate(N, b85, start);
		start += N * 5;
		if (start > 84)
		{
			start -= 85;
		}

		os << b85;
	}
	return EXIT_SUCCESS;
}

auto decompress(int N, const char* inputfile, const char* outputfile) {
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening input file for decoding");
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file for decoding");
	}

	uint64_t width = 0, height = 0;
	is >> width;
	if (is.peek() == ',')
	{
		is.get();
	}
	is >> height;

	if (is.peek() == ',')
	{
		is.get();
	}

	int start = 0;

	std::string magicnumber("P6");

	os << magicnumber << '\n';
	os << width << ' ';
	os << height << '\n';
	os << "255\n";

	int pad = (width * height) % 4;
	int padding = 0;
	if (pad != 0)
	{
		padding = 1;
	}

	bitwriter bw(os);

	for (size_t i = 0; i < ((3 * height * width) / 4) + padding; i++)
	{
		
		uint32_t buf = 0;
		std::string s(5, ' ');

		is.read(reinterpret_cast<char*>(s.data()), 5);

		demapping(s);

		backRotate(N, s, start);

		buf = to_base10(s);

		if (i == ((3 * height * width) / 4))
		{
			buf >>= pad * 8;
			bw(buf, (4 - pad) * 8);

		}
		else
		{
			bw(buf, 32);
		}
		
		start += N * 5;
		if (start > 84)
		{
			start -= 85;
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {	

	if (argc != 5)
	{
		return error("Usage is Z85rot {c | d} <N> <input file> <output file>");
	}

	if (argv[1][0] == 'c')
	{
		compress(atoi(argv[2]), argv[3], argv[4]);
		//compress
	}
	else if(argv[1][0] == 'd')
	{
		decompress(atoi(argv[2]), argv[3], argv[4]);
		//decompress
	}

	return 0;

}