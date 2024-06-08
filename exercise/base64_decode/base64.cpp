#include "base64.h"
#include <unordered_map>
#include <cstdint>
#include <math.h>
#include <cmath>

std::string base64_decode(const std::string& input) {
	
	std::unordered_map<char, uint8_t> ascii65{
		{0, 'A'},
		{1, 'B'},
		{2, 'C'},
		{3, 'D'},
		{4, 'E'},
		{5, 'F'},
		{6, 'G'},
		{7, 'H'},
		{8, 'I'},
		{9, 'J'},
		{10, 'K'},
		{11, 'L'},
		{12, 'M'},
		{13, 'N'},
		{14, 'O'},
		{15, 'P'},
		{16, 'Q'},
		{17, 'R'},
		{18, 'S'},
		{19, 'T'},
		{20, 'U'},
		{21, 'V'},
		{22, 'W'},
		{23, 'X'},
		{24, 'Y'},
		{25, 'Z'},
		{26, 'a'},
		{27, 'b'},
		{28, 'c'},
		{29, 'd'},
		{30, 'e'},
		{31, 'f'},
		{32, 'g'},
		{33, 'h'},
		{34, 'i'},
		{35, 'j'},
		{36, 'k'},
		{37, 'l'},
		{38, 'm'},
		{39, 'n'},
		{40, 'o'},
		{41, 'p'},
		{42, 'q'},
		{43, 'r'},
		{44, 's'},
		{45, 't'},
		{46, 'u'},
		{47, 'v'},
		{48, 'w'},
		{49, 'x'},
		{50, 'y'},
		{51, 'z'},
		{52, '0'},
		{53, '1'},
		{54, '2'},
		{55, '3'},
		{56, '4'},
		{57, '5'},
		{58, '6'},
		{59, '7'},
		{60, '8'},
		{61, '9'},
		{62, '+'},
		{63, '/'},
	};

	std::unordered_map<uint8_t, char> inverted_ascii65{
		{'A', 0}, {'B', 1}, {'C', 2}, {'D', 3}, {'E', 4}, {'F', 5}, {'G', 6},
		{'H', 7}, {'I', 8}, {'J', 9}, {'K', 10}, {'L', 11}, {'M', 12}, {'N', 13},
		{'O', 14}, {'P', 15}, {'Q', 16}, {'R', 17}, {'S', 18}, {'T', 19}, {'U', 20},
		{'V', 21}, {'W', 22}, {'X', 23}, {'Y', 24}, {'Z', 25}, {'a', 26}, {'b', 27},
		{'c', 28}, {'d', 29}, {'e', 30}, {'f', 31}, {'g', 32}, {'h', 33}, {'i', 34},
		{'j', 35}, {'k', 36}, {'l', 37}, {'m', 38}, {'n', 39}, {'o', 40}, {'p', 41},
		{'q', 42}, {'r', 43}, {'s', 44}, {'t', 45}, {'u', 46}, {'v', 47}, {'w', 48},
		{'x', 49}, {'y', 50}, {'z', 51}, {'0', 52}, {'1', 53}, {'2', 54}, {'3', 55},
		{'4', 56}, {'5', 57}, {'6', 58}, {'7', 59}, {'8', 60}, {'9', 61}, {'+', 62},
		{'/', 63}
	};

	std::string out;
	int n = 0;

	for (size_t i = 0; i < input.size(); i+=4)
	{
		uint32_t val = 0;
		char i1 = input[i];
		char i2 = input[i + 1];
		char i3 = input[i + 2];
		char i4 = input[i + 3];

		if (i1 != '=')
		{
			val += inverted_ascii65[i1] * std::pow(64, 3);
		}
		if (i2 != '=')
		{
			val += inverted_ascii65[i2] * std::pow(64, 2);
		}
		if (i3 != '=')
		{
			val += inverted_ascii65[i3] * std::pow(64, 1);
		}
		if (i4 != '=')
		{
			val += inverted_ascii65[i4] * std::pow(64, 0);
		}

		if (i2 != '=')
		{
			if (i3 != '=')
			{
				if (i4 != '=')
				{
					out.push_back((uint8_t)((val >> 16) & 0b11111111));
					out.push_back((uint8_t)((val >> 8) & 0b11111111));
					out.push_back((uint8_t)(val & 0b11111111));
					n = 0;
				}
				else
				{
					out.push_back((uint8_t)((val >> 16) & 0b11111111));
					out.push_back((uint8_t)((val >> 8) & 0b11111111));
					//out.push_back(0);
					n = 1;
				}
			}
			else
			{
				out.push_back((uint8_t)((val >> 16) & 0b11111111));
				//out.push_back(0);
				//out.push_back(0);
				n = 2;
			}
		}
	}

	out.resize((input.size() / 4 * 3) - n);



	return out;

}