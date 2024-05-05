#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <array>
#include <chrono>
#include <format>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <bit>

void error(const char *c) {
	std::cout << "ERRORE: " << c << '\n';
	exit(EXIT_FAILURE);
}


class bitwriter {
private:
	uint8_t buffer_;
	size_t n_ = 0;
	std::ostream& os_;

	//salvo il bit e eventualmente lo scrivo


	void writebit(uint64_t bit ) {
		buffer_ <<= 1;
		buffer_ |= (bit & 1);
		++n_;
		if (n_ == 8)
		{
			os_.put(buffer_);
			n_ = 0;
		}
	}

public:
	//costruttore che inizializza lo stream
	bitwriter(std::ostream& os) : os_(os){}

	//distruttore
	~bitwriter(){
		flush();
	}
	
	void operator()(uint64_t x, uint64_t nbits) {
		while (nbits --> 0)
		{
			writebit(x >> nbits);
		}
	}

	//flush
	void flush() {
		while (n_ > 0)
		{
			writebit(0);
		}
	}

	uint64_t elias_nbits_encoding(uint64_t x) {
		auto nbits = std::bit_width(x);
		return nbits * 2 - 1;
	}	

};

class bitreader {
private:
	uint8_t buffer_;
	size_t n_ = 0;
	std::istream& is_;
	
	//porto a dx il bit da leggere


	uint64_t readbit() {
		if (n_ == 0)
		{
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
public:
	//costruttore per l'istream
	bitreader(std::istream& is) : is_(is) {}

	//legge un bit alla volta finché non costruisce un numero a nbits


	uint64_t operator()(int nbits) {
		uint64_t val = 0;
		for (int i = nbits - 1; i >= 0; i--) {
			val = val << 1;
			val = val | readbit();
		}
		return val;
	}


	std::istream& get_stream() {
		return is_;
	}
};

void elias_encoding(const std::string& filenamein, const std::string& filenameout) {
	std::ifstream is(filenamein/*, std::ios::binary*/);
	if (!is)
	{
		error("Errore nell'apertura del file di input");
	}
	std::vector<int> v{ std::istream_iterator<int>(is), std::istream_iterator<int>() };

	std::ofstream os(filenameout, std::ios::binary);
	if (!os)
	{
		error("Errore nell'apertura del file di output");
	}
	for (int& elem : v) {
		if (elem < 0)
		{
			elem = (-1 * elem) * 2;
		}
		else if(elem > 0)
		{
			elem = (elem * 2) + 1;
		}
		else
		{
			elem = 1;
		}
	}
	bitwriter writer(os);
	for (const int& elem : v) {
		writer(elem, writer.elias_nbits_encoding(elem));
	}
}

void elias_decoding(const std::string& filenamein, const std::string& filenameout) {
	std::ifstream is(filenamein, std::ios::binary);
	if (!is)
	{
		error("Errore nell'apertura del file di input");
	}

	std::ofstream os(filenameout/*, std::ios::binary*/);
	if (!os)
	{
		error("Errore nell'apertura del file di output");
	}
	std::vector<int> v;
	int zeros = 0;
	bitreader reader(is);
	while (true)
	{
		int num = reader(1);
		if (!reader.get_stream())
		{
			break;
		}
		while (num == 0)
		{
			++zeros;
			num = reader(1);
			if (!reader.get_stream())
			{
				break;
			}
		}
		num <<= zeros;
		num |= reader(zeros);
		if (!reader.get_stream())
		{
			break;
		}
		zeros = 0;
		v.push_back(num);
	}

	for (int& elem : v) {
		if ((elem % 2) == 0)
		{
			elem = -(elem / 2);
		}
		else if(elem == 0)
		{
			elem = 1;
		}
		else
		{
			elem /= 2;
		}
		os << elem << '\n';
	}

}


int main(int argc, char* argv[]) {
	
	if (argc != 4)
	{
		error("Verifica usage");
	}

	std::string in = argv[2];
	std::string out = argv[3];

	if (argv[1] == std::string("c"))
	{
		elias_encoding(in, out);
	}
	else if(argv[1] == std::string("d"))
	{
		elias_decoding(in, out);
	}
	return 0;
}