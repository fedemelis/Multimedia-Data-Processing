#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <math.h>

struct cosine
{
	std::vector<std::vector<double>> cosine;

	void build(double N) {
		
		cosine = std::vector<std::vector<double>>(N, std::vector<double>(2 * N));
		for (size_t k_ = 0; k_ < N; k_++)
		{
			for (size_t n_ = 0; n_ < 2*N; n_++)
			{
				cosine[k_][n_] = cos(M_PI / N * (n_ + 0.5 + N / 2) * (k_ + 0.5));
			}
		}
	}

	
};

struct sine
{
	std::vector<double> sine;

	void build(double N) {

		sine.resize(N * 2);
		for (size_t n = 0; n < sine.size(); n++)
		{
			sine[n] = sin((M_PI / (2 * N)) * (n + 0.5));
		}
	}
};

struct freq
{
	std::unordered_map<int16_t, uint64_t> freq;
	uint64_t tot = 0;
	double entropy = 0;

	void operator()(int16_t val) {
		++freq[val];
		++tot;
	}

	void compute_entropy() {
		for (const auto& elem : freq) {
			double prob = (double)freq[elem.first] / (double)tot;
			entropy += prob * log2(1 / prob);
		}
	}
};

auto MDCT(int N, int i, std::vector<int16_t>& xs, std::vector<int32_t>& coeff, sine& s, cosine& c) {
	for (size_t k = 0; k < N; k++)
	{
		double tot = 0.0;
		for (size_t n = 0; n < 2 * N; n++)
		{
			tot += xs[i * N + n] * s.sine[n] * c.cosine[k][n];
		}
		coeff[N * i + k] = std::round(tot);
		tot = 0;
	}
}

auto IMDCT(std::vector<int32_t>& xs, std::vector<int16_t>& rec, int offset, int i, int N, sine& s, cosine& c) {
	for (size_t n = 0; n < 2*N; n++)
	{
		double tot = 0.;
		double wn = s.sine[n];
		for (size_t k = 0; k < N; k++)
		{
			tot += xs[i * N + k] * c.cosine[k][n];
		}
		rec[offset * N + n] = std::round(tot * wn * 2 / N);
	}
}

//std::vector<int16_t> win_imdct(const std::vector<int32_t>& coeffs, size_t offset, size_t N, sine& s, cosine& c)
//{
//	using namespace std;
//	vector<int16_t> recon(2 * N);
//
//	for (size_t n = 0; n < 2 * N; ++n) {
//		double tmpn = 0.;
//		for (size_t k = 0; k < N; ++k) {
//			double xk = coeffs[offset * N + k];
//			double ck = c.cosine[k][n];
//			tmpn += ck * xk;
//		}
//
//		recon[n] = 2. / N * s.sine[n] * tmpn;
//	}
//
//	return recon;
//}
//
//std::vector<int16_t> IMDCT_g(const std::vector<int32_t>& coeffs, size_t N, sine& s, cosine& c)
//{
//	using namespace std;
//	auto nwin = coeffs.size() / N;
//	vector<int16_t> samples((nwin - 1) * N);
//
//	vector<int16_t> prev = win_imdct(coeffs, 0, N, s, c);
//	for (size_t i = 1; i < nwin; ++i) {
//		vector<int16_t> curr = win_imdct(coeffs, i, N, s, c);
//
//		for (size_t j = 0; j < N; ++j)
//			samples[(i - 1) * N + j] = int16_t(round(curr[j] + prev[N + j]));
//
//		prev = move(curr);
//	}
//
//	return samples;
//}

int main(int argc, char* argv[]) {
		
	if (argc != 2)
	{
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<int16_t> data(size/2);

	is.read(reinterpret_cast<char*>(data.data()), (size/2) * sizeof(int16_t));

	freq f1;

	for (const auto& elem : data) {
		f1(elem);
	}

	f1.compute_entropy();

	std::cout << "Entropia della traccia audio originale: " << f1.entropy << '\n';


	std::vector<int16_t> dataquant;
	for (auto& elem : data) {
		dataquant.push_back(elem / 2600);
	}

	freq f2;

	for (const auto& elem : dataquant) {
		f2(elem);
	}

	f2.compute_entropy();

	std::cout << "Entropia della traccia audio quantizzata: " << f2.entropy << '\n';

	std::vector<int16_t> datarec;
	for (auto& elem : dataquant) {
		datarec.push_back(elem * 2600);
	}

	std::ofstream os("output_qt.raw", std::ios::binary);
	os.write(reinterpret_cast<char*>(datarec.data()), datarec.size() * 2);
	os.close();

	std::vector<int16_t> error;
	for (size_t i = 0; i < data.size(); i++)
	{
		error.push_back(data[i] - datarec[i]);
	}

	std::ofstream os_err("error_qt.raw", std::ios::binary);
	os_err.write(reinterpret_cast<char*>(error.data()), error.size() * 2);
	os_err.close();


	// MDCT
	
	double N = 1024;

	// devo costruire un nuovo vettore con il numero giusto di valori
	// il + 2 viene dal fatto che la dimensione deve considerare due finestre di padding 
	size_t nwindow = (data.size() / N) + 2;
	
	//inizializzo un vettore con zeri
	// alloco la dimensione gisuta del vettore
	std::vector<int16_t> padded_data(nwindow * N, 0);

	// ci metto dentro i valori precedenti considerando che la prima finestra del nuovo vettore va lasciata vuota quindi salto 1024
	std::copy(data.begin(), data.end(), padded_data.begin() + N);

	// i coefficienti 
	std::vector<int32_t> coeff((nwindow - 1) * N);

	// costruisco le tabelle per seno e coseno
	sine s;
	cosine c;
	s.build(N);
	c.build(N);

	// nwindow - 1 perché l'ultima è un padding e non devo farla
	for (size_t i = 0; i < nwindow - 1; i++)
	{
		MDCT(N, i, padded_data, coeff, s, c);
		std::cout << i << '\n';
	}

	std::vector<int16_t> quantized_coeff;
	for (const auto& elem : coeff) {
		quantized_coeff.push_back(elem / 10000);
	}

	freq f3;

	for (const auto& elem : quantized_coeff) {
		f3(elem);
	}

	f3.compute_entropy();

	std::cout << "Entropia dei coefficienti quantizzati:  " << f3.entropy << '\n';

	std::vector<int32_t> dequantized_coeff;
	for (const auto& elem : quantized_coeff) {
		dequantized_coeff.push_back(elem * 10000);
	}

	// IMDCT
	N = 1024;
	int inverseWindow = dequantized_coeff.size() / N;
	std::vector<int16_t> rec(inverseWindow * N * 2);

	int offset = 0;
	for (size_t i = 0; i < inverseWindow; i++)
	{
		IMDCT(dequantized_coeff, rec, offset, i, N, s, c);
		offset += 2;
		//std::cout << i << std::endl;
	}

	std::vector<int16_t> final_rec(inverseWindow * N);
	int off = 1;
	for (size_t i = 0; i < inverseWindow - 1; i++)
	{
		for (size_t ii = 0; ii < N; ii++)
		{
			final_rec[N * i + ii] = rec[N * off + ii] + rec[N * (off + 1) + ii];
		}
		off += 2;
	}

	//auto final_rec = IMDCT_g(coeff, N, s, c);;

	std::ofstream os_rec("output_rec.raw", std::ios::binary);
	os_rec.write(reinterpret_cast<char*>(final_rec.data()), final_rec.size() * 2);
	os_rec.close();

	std::vector<int16_t> err_rec(data.size());
	for (size_t i = 0; i < err_rec.size(); i++)
	{
		err_rec[i] = data[i] - final_rec[i];
	}

	std::ofstream os_rec_err("error_rec.raw", std::ios::binary);
	os_rec_err.write(reinterpret_cast<char*>(err_rec.data()), err_rec.size() * 2);
	os_rec_err.close();

	return EXIT_SUCCESS;

}