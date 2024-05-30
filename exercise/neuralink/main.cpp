#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <bit>
#include <bitset>
#include <cmath>
#include <unordered_map>
#include <filesystem>

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

	~bitwriter() {
		flush();
	}

	void operator()(uint64_t value, int nbits) {
		for (int i = nbits - 1; i >= 0; i--)
		{
			writebit((value >> i) & 1);
		}
	}

	void flush() {
		while (n_ != 0)
		{
			writebit(0);
		}
	}
};

struct bitreader
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ifstream& is_;

	uint8_t readbit() {
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
		for (int i = nbits - 1; i >= 0; i--)
		{
			val <<= 1;
			val |= readbit();
		}
		return val;
	}
};

struct freq
{
	std::unordered_map<int16_t, uint64_t> freq;
	uint64_t tot = 0;
	double entropy = 0;

	std::unordered_map<int16_t, double> logs;
	std::unordered_map< int16_t, double> probs;

	void tables() {
		probsTable();
		logsTable();
	}

	void probsTable() {
		for (const auto& elem : freq) {
			if (probs.find(elem.first) == probs.end())
			{
				probs[elem.first] = (double)freq[elem.first] / (double)tot;
			}
		}
	}

	void logsTable() {
		for (const auto& elem: probs) {
			if (logs.find(elem.first) == logs.end())
			{
				logs[elem.first] = log2(1 / elem.second) * elem.second;
			}
		}
	}

	void operator()(int16_t val) {
		++freq[val];
		++tot;
	}

	void compute_entropy() {
		for (const auto& elem : freq) {
			entropy += logs[elem.first];
		}
	}


};

auto encode() {
	std::vector<int16_t> data;
	std::vector<uint8_t> head(40);
	uint32_t size = 0;
	double entrop = 0;
	int n = 0;

	std::ofstream os2("diff.bin", std::ios::binary);

#pragma omp parallel for
	for (auto const& dir_entry : std::filesystem::directory_iterator{ "data" }) {
		std::ifstream is(dir_entry.path(), std::ios::binary);
		if (!is)
		{
			return 1;
		}

		while (is.peek() != EOF)
		{
			is.read(reinterpret_cast<char*>(head.data()), 40);
			is.read(reinterpret_cast<char*>(&size), 4);

			is.seekg(0, std::ios::end);
			auto size = is.tellg();
			is.seekg(44, std::ios::beg);

			size -= 44;

			data.resize(size / 2);

			is.read(reinterpret_cast<char*>(data.data()), size);

			/*for (size_t i = 0; i < size; i += 2)
			{
				uint16_t s = 0;
				is.read(reinterpret_cast<char*>(&s), 2);
				data.push_back(s);
			}*/

			/*auto itmax = std::max_element(data.begin(), data.end());
			auto itmin = std::min_element(data.begin(), data.end());

			auto bmax = std::bit_width((uint16_t)*itmax);
			auto bmin = std::bit_width((uint16_t)std::abs(*itmin));

			std::cout << "Max: " << *itmax << "\tMin: " << *itmin << std::endl;
			std::cout << "Max: " << bmax << "\tMin: " << bmin << std::endl;

			int win = 0;
			if (bmax > bmin)
			{
				win = bmax;
			}
			else
			{
				win = bmin;
			}*/


			freq f1;
			for (const auto& elem : data) {
				f1(elem);
			}
			f1.tables();
			f1.compute_entropy();
			std::cout << "Entropia della traccia audio originale: " << f1.entropy << '\n';

			/*bitwriter bw(os);
			std::vector<int16_t> trunc;
			os.write(reinterpret_cast<char*>(head.data()), 40);
			os.write(reinterpret_cast<char*>(&size), 4);
			os.write(reinterpret_cast<char*>(&win), 2);
			for (const auto& elem : data) {
				bw(elem, win);
			}

			std::ifstream isout(argv[2], std::ios::binary);
			isout.ignore(40);
			uint32_t sz = 0;
			isout.read(reinterpret_cast<char*>(&sz), 4);

			uint16_t bitdim = 0;
			isout.read(reinterpret_cast<char*>(&bitdim), 2);

			bitreader br(isout);

			for (size_t i = 0; i < data.size(); i++)
			{
				int vv = (int)br(bitdim);
				if (vv >= (std::pow(2, win - 1)))
				{
					vv -= std::pow(2, win);
				}
				trunc.push_back(vv);
			}

			freq f2;
			for (const auto& elem : trunc) {
				f2(elem);
			}
			f2.compute_entropy();
			std::cout << "Entropia della traccia audio troncata: " << f2.entropy << '\n';*/

			int16_t prec = 0;
			std::vector<int16_t> diff;
			for (size_t i = 0; i < data.size(); i++)
			{
				diff.push_back(data[i] - prec);
				prec = data[i];
			}

			freq f3;
			for (const auto& elem : diff) {
				f3(elem);
			}
			f3.tables();
			f3.compute_entropy();
			std::cout << "Entropia della traccia audio diff: " << f3.entropy << '\n' << '\n';

			entrop += f3.entropy;
			++n;

			/*auto itmax2 = std::max_element(diff.begin(), diff.end());
			auto itmin2 = std::min_element(diff.begin(), diff.end());

			auto bmax2 = std::bit_width((uint16_t)*itmax2);
			auto bmin2 = std::bit_width((uint16_t)std::abs(*itmin2));

			std::cout << "Max: " << *itmax2 << "\tMin: " << *itmin2 << std::endl;
			std::cout << "Max: " << bmax2 << "\tMin: " << bmin2 << std::endl;

			int win2 = 0;
			if (bmax2 > bmin2)
			{
				win2 = bmax2;
			}
			else
			{
				win2 = bmin2;
			}*/
			os2.write(reinterpret_cast<char*>(diff.data()), diff.size() * 2);

		}
	}
	std::cout << "Entropia finale delle traccie audio diff: " << entrop / (double)n << '\n' << '\n';

	return 0;
}

auto decode() {
	std::vector<int16_t> diff_rec;
	
	std::ifstream is("diff.decompressed", std::ios::binary);
	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(0, std::ios::beg);

	diff_rec.resize(size / 2);

	is.read(reinterpret_cast<char*>(diff_rec.data()), size);

	std::vector<int16_t> rec;
	
	int16_t prec = 0;
	for (size_t i = 0; i < diff_rec.size(); i++)
	{
		int16_t x = diff_rec[i] + prec;
		prec = x;
		rec.push_back(x);
	}

	std::cout << "stop";

}

int main(int argc, char** argv) {

	//encode();

	decode();
	
}