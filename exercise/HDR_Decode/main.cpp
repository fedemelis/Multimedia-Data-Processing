#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <format>

void error(const char* errmsg) {
	std::cerr << errmsg << '\n';
}

template <typename T>
struct mat
{
	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}

	const T& operator()(size_t row, size_t col) const{
		return data_[row * c_ + col];
	}

	size_t rows() {
		return r_;
	}

	size_t cols() {
		return c_;
	}

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};


void readchannel(size_t dim, std::vector<uint8_t>& channel, std::ifstream& is) {
	while (channel.size() < dim)
	{
		uint8_t L = is.get();
		if (L <= 127)
		{
			for (size_t i = 0; i < L; i++)
			{
				channel.push_back(is.get());
			}
		}
		else
		{
			uint8_t c = is.get();
			for (size_t i = 0; i < L - 128; i++)
			{
				channel.push_back(c);
			}
		}
	}

}


void construct(uint16_t N, std::vector<uint8_t>& R, std::vector<uint8_t>& G, std::vector<uint8_t>& B, std::vector<uint8_t>& E, std::vector<double>& Rdec, std::vector<double>& Gdec, std::vector<double>& Bdec) {

	for (size_t i = 0; i < N; i++)
	{
		double exp = std::pow(2, E[i] - 128);
		Rdec.push_back(((R[i] + 0.5) / 256) * exp);
		Gdec.push_back(((G[i] + 0.5) / 256) * exp);
		Bdec.push_back(((B[i] + 0.5) / 256) * exp);
	}

	R.erase(R.begin(), R.end());
	G.erase(G.begin(), G.end());
	B.erase(B.begin(), B.end());
	E.erase(E.begin(), E.end());
}

auto normalize(std::vector<double>& Rdec, std::vector<double>& Gdec, std::vector<double>& Bdec) {
	
	auto min_r = std::min_element(Rdec.begin(), Rdec.end());
	auto min_g = std::min_element(Gdec.begin(), Gdec.end());
	auto min_b = std::min_element(Bdec.begin(), Bdec.end());
	std::vector<double> min_candidate({ *min_r, *min_g, *min_b });

	auto min = std::min_element(min_candidate.begin(), min_candidate.end());

	auto max_r = std::max_element(Rdec.begin(), Rdec.end());
	auto max_g = std::max_element(Gdec.begin(), Gdec.end());
	auto max_b = std::max_element(Bdec.begin(), Bdec.end());
	std::vector<double> max_candidate({ *max_r, *max_g, *max_b });

	auto max = std::max_element(max_candidate.begin(), max_candidate.end());

	std::vector<std::array<double, 3>> pix(Rdec.size());

	for (uint64_t i = 0; i < Rdec.size(); i++)
	{
		double r_norm = 255 * std::pow(((Rdec[i] - *min)/(*max - *min) ), 0.45);
		double g_norm = 255 * std::pow(((Gdec[i] - *min) / (*max - *min)), 0.45);
		double b_norm = 255 * std::pow(((Bdec[i] - *min) / (*max - *min)), 0.45);
		/*if (r_norm != 0 || g_norm != 0 || b_norm != 0)
		{
			std::cout << i << '\n';
		}*/
		pix[i][0] = r_norm;
		pix[i][1] = g_norm;
		pix[i][2] = b_norm;

	}

	return pix;


}

int readhdr(const char* input, std::vector<std::array<double, 3>>& pix, size_t& row, size_t& col) {
	std::ifstream is(input, std::ios::binary);
	if (!is)
	{
		error("Error while opeingin input.hdr file");
		return EXIT_FAILURE;
	}

	std::string magicnumber;
	std::string format, trash, buffer;

	is.get(); // popping of #
	is >> magicnumber;
	if (magicnumber != "?RADIANCE")
	{
		error("Magicnumber should be \"Radiance\"");
		return EXIT_FAILURE;
	}

	is.get(); // popping of \n
	while (is.peek() != '\n')
	{
		if (is.peek() == '#')
		{
			while (is.peek() != '\n')
			{
				is >> trash;
			}
		}
		else {
			is >> buffer;
			if (buffer.contains("FORMAT="))
			{
				buffer.erase(buffer.begin(), buffer.begin() + 7);
				format = buffer;
			}
		}
		is.get(); // removing \n
	}
	is.get(); // removing \n

	std::string Y, X;
	size_t rows = 0, cols = 0;

	is >> Y;
	is >> rows;
	is >> X;
	is >> cols;
	is.get();

	row = rows;
	col = cols;

	std::vector<uint8_t> R, G, B, E;

	std::vector<double> Rdec, Gdec, Bdec;

	uint16_t linelen = 0;

	while (is.peek() != EOF)
	{
		is.get(); // 02
		is.get(); // 02
		linelen = is.get();
		uint8_t tmp = is.get();
		linelen = (linelen << 8) | tmp;

		readchannel(linelen, R, is);
		readchannel(linelen, G, is);
		readchannel(linelen, B, is);
		readchannel(linelen, E, is);

		construct(linelen, R, G, B, E, Rdec, Gdec, Bdec);

	}

	pix = normalize(Rdec, Gdec, Bdec);
	return EXIT_SUCCESS;
}

auto decode(const char* inputfile) {

	size_t row = 0, col = 0;
	std::vector<std::array<double, 3>> pix;
	readhdr(inputfile, pix, row, col);

	mat<std::array<uint8_t, 3>> img(row, col);
	uint64_t i = 0;

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			/*if (pix[i][0] != 0)
			{
				std::cout << i << '\n';
			}*/
			img(r, c)[0] = (pix[i][0]);
			img(r, c)[1] = (pix[i][1]);
			img(r, c)[2] = (pix[i][2]);
			++i;
		}
	}

	return img;

}

auto savepam(const char* outputfile, mat<std::array<uint8_t, 3>>& img) {
	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while opening output.pam file");
		return EXIT_FAILURE;
	}

	std::string head = std::format(

		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 3\n"
		"MAXVAL 255\n"
		"TUPLTYPE RGB\n"
		"ENDHDR\n"
		, img.cols(), img.rows());

	os << head;

	os.write(img.rawdata(), img.dim());

	return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {
	
	if (argc != 3)
	{
		error("Usage is hdr_decode <inputfile.hdr> <outputfile.pam>");
		return EXIT_FAILURE;
	}

	mat<std::array<uint8_t, 3>> img = decode(argv[1]);


	savepam(argv[2], img);

	//std::cout << "Ciao";

	return EXIT_SUCCESS;

}