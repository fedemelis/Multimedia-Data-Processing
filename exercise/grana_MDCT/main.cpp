#define _USE_MATH_DEFINES
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>
#include <unordered_map>
#include <cstdint>

template <typename T>
struct freq {
	std::unordered_map<T, uint32_t> histo;

	void operator()(const T& val) {
		++histo[val];
	}

	double entropy() const {
		double s = 0, h = 0;
		for (const auto& x : histo) {
			s += x.second;
			h += x.second * log2(x.second);
		}
		return log2(s) - h / s;
	}

	auto size() { return histo.size(); }

	auto begin() { return histo.begin(); }
	auto end() { return histo.end(); }
	auto begin() const { return histo.begin(); }
	auto end() const { return histo.end(); }
};

template <typename T>
std::vector<T> read_track(const std::string& fname)
{
	using namespace std;
	ifstream is(fname, ios::binary);
	is.seekg(0, ios::end);
	auto dim = is.tellg();
	is.seekg(0, ios::beg);

	vector<T> samples(size_t(dim) / sizeof(T));
	is.read(reinterpret_cast<char*>(samples.data()), dim);
	return samples;
}

template<typename T>
void write_track(const std::string& fname, const std::vector<T>& samples)
{
	using namespace std;
	ofstream os(fname, ios::binary);
	os.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(T));
}

template<typename T>
void print_freq(const std::vector<T>& samples)
{
	using namespace std;
	freq<T> f;
	f = for_each(begin(samples), end(samples), f);
	cout << "Numero di valori differenti: " << f.size() << "\n";
	cout << "Entropia: " << f.entropy() << "\n";
}

template<typename T>
T quant(const T& val, double factor)
{
	return T(lround(val / factor));
}

template<typename T>
T dequant(const T& val, double factor)
{
	return T(lround(val * factor));
}

std::vector<double> sin_tab;
std::vector<std::vector<double>> cos_tab;

void precalc_tables(size_t N)
{
	sin_tab.resize(2 * N);
	for (size_t i = 0; i < sin_tab.size(); ++i)
		sin_tab[i] = sin((M_PI / (2 * N)) * (i + 0.5));

	cos_tab = std::vector<std::vector<double>>(N, std::vector<double>(2 * N));
	for (size_t k = 0; k < N; ++k) {
		for (size_t n = 0; n < 2 * N; ++n) {
			cos_tab[k][n] = cos(M_PI / N * (n + 0.5 + N / 2) * (k + 0.5));
		}
	}
}

template<typename CT, typename ST>
void win_mdct(const std::vector<ST>& padded, std::vector<CT>& coeffs, size_t offset, size_t N)
{
	for (size_t k = 0; k < N; ++k) {
		double Xk = 0.;
		for (size_t n = 0; n < 2 * N; ++n) {
			ST xn = padded[offset * N + n];
			double wn = sin_tab[n];
			double cn = cos_tab[k][n];

			Xk += xn * cn * wn;
		}

		coeffs[offset * N + k] = CT(round(Xk));
	}
}


template<typename CT, typename ST>
std::vector<CT> MDCT(const std::vector<ST>& samples, size_t N)
{
	using namespace std;
	auto nwin = size_t(ceil(samples.size() / double(N))) + 2;
	vector<ST> padded(nwin * N, 0);
	copy(begin(samples), end(samples), begin(padded) + N);

	vector<CT> coeffs((nwin - 1) * N);
	for (size_t i = 0; i < nwin - 1; ++i)
		win_mdct(padded, coeffs, i, N);

	return coeffs;
}

template<typename CT>
std::vector<double> win_imdct(const std::vector<CT>& coeffs, size_t offset, size_t N)
{
	using namespace std;
	vector<double> recon(2 * N);

	for (size_t n = 0; n < 2 * N; ++n) {
		double tmpn = 0.;
		for (size_t k = 0; k < N; ++k) {
			CT xk = coeffs[offset * N + k];
			double ck = cos_tab[k][n];
			tmpn += ck * xk;
		}

		recon[n] = 2. / N * sin_tab[n] * tmpn;
	}

	return recon;
}

template<typename ST, typename CT>
std::vector<ST> IMDCT(const std::vector<CT>& coeffs, size_t N)
{
	using namespace std;
	auto nwin = coeffs.size() / N;
	vector<ST> samples((nwin - 1) * N);

	vector<double> prev = win_imdct(coeffs, 0, N);
	for (size_t i = 1; i < nwin; ++i) {
		vector<double> curr = win_imdct(coeffs, i, N);

		for (size_t j = 0; j < N; ++j)
			samples[(i - 1) * N + j] = ST(round(curr[j] + prev[N + j]));

		prev = move(curr);
	}

	return samples;
}

int main()
{
	using namespace std::placeholders;
	using datatype = int16_t;
	using coefftype = int32_t;

	auto original = read_track<datatype>("test.raw");

	std::cout << "Info su campioni:\n";
	print_freq(original);

	// quantize in time
	if (true) {
		double quant_fac = 2600;
		auto quant_f = bind(quant<datatype>, _1, quant_fac);
		auto dequant_f = bind(dequant<datatype>, _1, quant_fac);

		std::vector<datatype> quant_t(original.size());
		transform(begin(original), end(original), begin(quant_t), quant_f);

		std::cout << "Info su campioni quantizzati:\n";
		print_freq(quant_t);

		std::vector<datatype> recon(original.size());
		transform(begin(quant_t), end(quant_t), begin(recon), dequant_f);

		write_track("output_qt.raw", recon);

		// compute error
		std::vector<datatype> error(original.size());
		for (size_t i = 0; i < original.size(); ++i)
			error[i] = original[i] - recon[i];

		write_track("error_qt.raw", error);
	}

	// quantize in frequency
	if (true) {
		double quant_fac = 20000;
		auto quant_f = bind(quant<coefftype>, _1, quant_fac);
		auto dequant_f = bind(dequant<coefftype>, _1, quant_fac);
		// N is the number of coefficients
		size_t N = 1024;

		precalc_tables(N);
		auto coeff = MDCT<coefftype>(original, N);

		std::cout << "Info su coefficienti:\n";
		print_freq(coeff);

		// quantize
		std::vector<coefftype> coeff_q(coeff.size());
		transform(begin(coeff), end(coeff), begin(coeff_q), quant_f);

		std::cout << "Info su coefficienti quantizzati:\n";
		print_freq(coeff_q);

		// dequantize
		std::vector<coefftype> coeff_recon(coeff.size());
		transform(begin(coeff_q), end(coeff_q), begin(coeff_recon), dequant_f);

		auto recon = IMDCT<datatype>(coeff_recon, N);

		write_track("output.raw", recon);

		std::vector<datatype> error(recon.size());
		for (size_t i = 0; i < original.size(); ++i)
			error[i] = original[i] - recon[i];

		write_track("error.raw", error);
	}
}