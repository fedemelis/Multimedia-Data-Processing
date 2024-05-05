#include <fstream>
#include <vector>
#include <bitset>

int main(int argc, char* argv[]) {

	using std::vector;

	if (argc != 3)
	{
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	vector<int32_t> v;
	int32_t val;	
	while (true)
	{
		is.read(reinterpret_cast<char*>(&val), sizeof(int32_t));
		if (!is)
		{
			break;
		}
		v.push_back(val);
	}


	
	std::ofstream os(argv[2]/*, std::ios::binary*/);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	for (const auto& elem : v) {
		os << elem << '\n';
	}

	return 0;

}