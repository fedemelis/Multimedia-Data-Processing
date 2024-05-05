#include <fstream>
#include <vector>
#include <bitset>
#include <chrono>
#include <iostream>

void error(const char* msg){
	std::cout << msg;
	exit(EXIT_FAILURE);
}

void syntax() {
	error("SYNTAX:\n"
		"huffdiff [c|d] <input file> <output file>\n");
}

//struct bitwriter{	
//	uint8_t bitbuf_;
//	int i_exit;
//	std::ostream &os_;
//
//	bitwriter(std::ostream& os) : os_(os){
//		
//	}
//
//	void writebit(int curbit) {
//		bu
//	}
//};

int main(int argc, char* argv[]) {

	using std::vector;
	using namespace std::chrono;

	auto start = steady_clock::now();

	if (argc != 3)
	{
		syntax();
	}

	/*std::ofstream isbuilding(argv[1]);
	for (size_t i = 0; i < 50000; i++)
	{
		isbuilding << std::rand() % 2001 - 1000 << ' ';
	}*/

	std::ifstream is(argv[1]/*, std::ios::binary*/);
	if (!is)
	{
		error("L'apertura del file di input non è andata a buon fine");
	}

	int val;

	//is.seekg(0, std::ios::end);

	//auto filesize = is.tellg();

	//is.seekg(std::ios::beg);

	//vector<int> v(filesize/sizeof(int));

	//is.read(reinterpret_cast<char*>(v.data()), filesize);

	vector<int> v;
	while (is >> val)
	{
		v.push_back(val);
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os)
	{
		error("L'apertura del file di output non è andata a buon fine");
	}

	
	int i_exit = 0;
	char bitbuf = 0;
	int n_bit = 11;
	for (const int& elem : v) {
		for (int i = n_bit - 1; i >= 0; i--)
		{
			bitbuf = bitbuf << 1;
			/*
			porto il bit che devo leggere entro elem nella posizione più a dx
			faccio l'and fra questo elemento e 0x00000001 in modo che venga preso se e solo se è 1
			poi faccio l'or con bitbuf che è inizializzato a zero, così se (elem >> i) & 0x00000001 ha
			restituito 0x00000000 non lo aggiorno, mentre se invece ha restituito 0x00000001 lo aggiorno 
			*/
			bitbuf = bitbuf | ((elem >> i) & 0x00000001);
			i_exit += 1;
			if (((i_exit % 8) == 0) && i_exit != 0)
			{
				os.write(&bitbuf, sizeof(bitbuf));
				//non serve azzerare il buffer
				//bitbuf = 0;
				i_exit = 0;
			}
		}	
	}
	int padding = 8 - i_exit;
	bitbuf = bitbuf << padding;
	os.write(&bitbuf, sizeof(bitbuf));

	auto end = steady_clock::now();
	auto diff = end - start;
	
	auto duration_ms = duration<double, std::milli>(diff);

	std::cout << "Tempo di esecuzione: " << duration_ms << '\n';

	return 0;
}