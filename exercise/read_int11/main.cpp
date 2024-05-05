#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iostream>

/*


dwrwrwerwedwere*/

int main(int argc, char* argv[]) {
	using std::vector;
	using namespace std::chrono;

	auto start = steady_clock::now();

	if (argc != 3)
	{
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	vector<int> v;
	char val = 0;
	char bitbuf = 0;
	int n_readed = 0;
	int res = 0;
	while (true)
	{
		is.read(&val, sizeof(char));
		if (!is)
		{
			break;
		}
		for (int i = 7; i >= 0; i--)
		{
			bitbuf <<= 1;
			bitbuf |= ((val >> i) & 0x1);
			n_readed += 1;
			if (n_readed == 11)
			{
				res |= (bitbuf & (0x0 + (static_cast<int>(pow(2, 8 - i))) - 1));
				if (res > 1024)
				{
					//res |= ((~res + 1) & 0x7FF);
					//res = -(unsigned char)res;
 				}
				v.push_back(res);
				res = 0;
				bitbuf = 0;
				n_readed = 0;
			}
		}
		res = bitbuf;
		res <<= (11 - n_readed);
		bitbuf = 0;
	}

	std::ofstream os(argv[2]/*, std::ios::binary*/);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	for (const int& elem : v) {
		if (elem > 1024)
		{
			os << (elem - 2048) << '\n';
		}
		else
		{
			os << elem << '\n';
		}
		/*os << elem << '\n';*/
	}
	
	auto end = steady_clock::now();
	auto diff = end - start;

	auto duration_ms = duration<double, std::milli>(diff);

	std::cout  << "Tempo di esecuzione: " << duration_ms << '\n';

	return EXIT_SUCCESS;
}