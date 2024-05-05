#include <algorithm>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <fstream>
#include <unordered_map>
#include <format>
#include <memory>

void error(const char* errmsg) {
	std::cerr << errmsg << '\n';
}

struct bitwriter
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ofstream& os_;

	void writebit(uint8_t bit) {
		buffer_ <<= 1;
		buffer_ |= (bit & 1);
		++n_;
		if (n_ == 8)
		{
			os_.put(buffer_);
			n_ = 0;
		}
	}
	
	bitwriter(std::ofstream& os) : os_(os) {}

	void operator()(uint64_t val, int bit) {

		for (int i = bit - 1; i >= 0; i--)
		{
			writebit(val >> i);
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

struct bitreader
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ifstream& is_;

	auto readbit() {
		if (n_ == 0)
		{
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

	bitreader(std::ifstream& is) : is_(is) {}

	auto operator()(int numbits) {
		uint64_t val = 0;
		for (int i = numbits - 1; i >= 0; i--)
		{
			val <<= 1;
			val |= readbit();
		}
		return val;
	}
};


struct freq
{
	std::unordered_map<int, size_t> occ;
	uint64_t tot = 0;

	void operator()(int c) {
		++occ[c];
		++tot;
	}

};

template <typename T>
struct huffman
{
	struct node
	{
		int sym_;
		uint64_t freq_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(T sym, uint64_t freq) : sym_(sym), freq_(freq) {}

		node(T sym, uint64_t freq, node* l, node* r) : sym_(sym), freq_(freq), left_(l), right_(r) {}
	};

	std::vector<std::unique_ptr<node>> nodes_;
	std::unordered_map<int, std::pair<uint64_t, uint64_t>> code_map; // (sym (code, len))

	bool sorting(const node* lhs, const node* rhs) {
		return lhs->freq_ > rhs->freq_;
	}

	auto build(const freq& f) {
		
		std::vector<node*> v;
		for (auto& [sym, sfreq] : f.occ) {
			if (sfreq > 0)
			{
				nodes_.emplace_back(std::make_unique<node>(sym, sfreq));
				v.emplace_back(nodes_.back().get());
			}
		}
		std::sort(v.begin(), v.end(), [](const node* lhs, const node* rhs) {return lhs->freq_ > rhs->freq_; });
		while (v.size() > 1)
		{
			node* n1 = v.back();
			v.pop_back();
			node* n2 = v.back();
			v.pop_back();

			nodes_.emplace_back(std::make_unique<node>(T{}, n1->freq_ + n2->freq_, n1, n2));
			auto n = nodes_.back().get();

			auto pos = std::lower_bound(v.begin(), v.end(), n, [](const node* lhs, const node* rhs) {return lhs->freq_ > rhs->freq_; });

			v.insert(pos, n);

		}
		node* root = v.back();
		generate_codes(root);
		std::cout << "ciao";
	}

	void generate_codes(const node* root, uint64_t code = 0, uint64_t len = 0) {
		if (root->left_ == nullptr)
		{
			code_map[root->sym_] = { code, len };
		}
		else
		{
			generate_codes(root->left_, ((code << 1) | 0), len + 1);
			generate_codes(root->right_, ((code << 1) | 1), len + 1);
		}
	}

	std::vector<std::tuple<uint64_t, int, uint64_t>>canonical_map; // {len, symbol, code}

	
	auto canoic() {
		for (const auto& [sym, x] : code_map) {
			auto [code, len] = x;
			canonical_map.push_back({ len, sym, code });
		}

		std::sort(canonical_map.begin(), canonical_map.end());

		uint64_t curlen = 0, curcode = 0;

		for (auto& [len, sym, code] : canonical_map) {
			curcode <<= len - curlen;
			curlen = len;
			code_map[sym].first = curcode;
			code = curcode;
			++curcode;
		}
	}

	auto canoic(std::unordered_map<int, std::pair<uint64_t, uint64_t>>& map) {
		canonical_map.erase(canonical_map.begin(), canonical_map.end());
		for (const auto& [sym, x] : map) {
			auto [code, len] = x;
			canonical_map.push_back({ len, sym, code });
		}

		std::sort(canonical_map.begin(), canonical_map.end());

		uint64_t curlen = 0, curcode = 0;

		for (auto& [len, sym, code] : canonical_map) {
			curcode <<= len - curlen;
			curlen = len;
			map[sym].first = curcode;
			code = curcode;
			++curcode;
		}
	}

};



template <typename T>
struct mat
{
	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	mat(const mat<T>& m) : r_(m.r_), c_(m.c_), data_(m.data_) {}

	size_t rows() {
		return r_;
	}

	size_t cols() {
		return c_;
	}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}
	const T& operator()(size_t row, size_t col) const {
		return data_[row * c_ + col];
	}

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};


auto loadpam(const char* input) {
	std::ifstream is(input, std::ios::binary);
	if (!is)
	{
		error("Error while opening pam input image");
		throw std::runtime_error("Error while opening pam input image");
	}

	std::string magicnumber;
	is >> magicnumber;
	if (magicnumber != "P7")
	{
		error("Magic number should be P7");
		throw std::runtime_error("Magic number should be P7");
	}

	std::string field;
	std::string tupltype;
	size_t width = 0, height = 0, maxval = 0;
	uint8_t depth = 0;
	is >> field;
	while (field != "ENDHDR")
	{
		if (field == "WIDTH")
		{
			is >> width;
		}
		else if (field == "HEIGHT") {
			is >> height;
		}
		else if (field == "DEPTH") {
			is >> depth;
		}
		else if (field == "MAXVAL") {
			is >> maxval;
		}
		else if (field == "TUPLTYPE") {
			is >> tupltype;
		}
		is >> field;
	}
	is.get(); // removing last \n
	
	mat<uint8_t> img(height, width);
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = is.get();
		}
	}
	return img;
}


auto loadhuffdiff(const char* input) {
	std::ifstream is(input, std::ios::binary);
	if (!is)
	{
		error("Error while opening huffdiff input image");
		throw std::runtime_error("Error while opening huffdiff input image");
	}

	std::string magicnumber;
	is >> magicnumber;
	if (magicnumber != "HUFFDIFF")
	{
		error("Magic number should be HUFFDIFF");
		throw std::runtime_error("Magic number should be HUFFDIFF");
	}

	bitreader br(is);
	uint32_t tmp_width = 0, tmp_height = 0,  width = 0, height = 0, numelement = 0;

	std::string field(8, ' ');

	is >> field; //width
	is.get(); //removing space
	is.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));

	is >> field; //height
	is.get(); //removing space
	is.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));

	is.get(); //removing newline

	numelement = br(9);

	std::vector<std::pair<int, uint8_t>> mapping;

	std::unordered_map<int, std::pair<uint64_t, uint64_t>> code_map;

	for (size_t i = 0; i < numelement; i++)
	{
		int symbol = br(9);
		if (symbol > 255)
		{
			symbol -= 512;
		}
		uint8_t len = br(5);
		mapping.push_back({symbol, len});
		code_map[symbol] = { 0, len };
	}

	huffman<int> h;
	h.canoic(code_map);

	// k: code	v: (len, sym)
	std::unordered_map<uint64_t, std::pair<size_t, int>> reconstructing_table;
	for (auto& [len, sym, code] : h.canonical_map) {
		reconstructing_table[code] = {len, sym};
	}

	uint64_t buf = 0;
	size_t len = 0;
	std::vector<int> values;
	while (true)
	{
		if (!is)
		{
			break;
		}
		if (reconstructing_table.contains(buf))
		{
			if (reconstructing_table[buf].first == len)
			{
				values.push_back(buf);
				buf = 0;
				len = 0;
			}
		}
		buf = (buf << 1) | br(1);
		++len;
	}

	mat<int> img(height, width);
	std::reverse(values.begin(), values.end());
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c) = values.back();
			values.pop_back();
		}
	}

	mat<uint8_t> original_img(img.rows(), img.cols());

	int prec = 0;
	for (size_t r = 0; r < original_img.rows(); r++)
	{
		for (size_t c = 0; c < original_img.cols(); c++)
		{
			auto tmp = reconstructing_table[img(r, c)].second;
			original_img(r, c) = tmp + prec;
			prec = tmp + prec;
		}
		auto tmp = reconstructing_table[img(r, 0)].second;
		prec = original_img(r, 0);
	}

	return original_img;
}


template <typename T>
auto savepam(mat<T>& img, const char* out = "diff.pam") {
	std::ofstream os(out, std::ios::binary);
	if (!os)
	{
		error("Error while opening difference file");
		return EXIT_FAILURE;
	}

	std::string head = std::format(
		"P7\n"
		"WIDTH {}\n"
		"HEIGHT {}\n"
		"DEPTH 1\n"
		"MAXVAL 255\n"
		"TUPLTYPE GRAYSCALE\n"
		"ENDHDR\n"
	, img.cols(), img.rows());

	os << head;
	os.write(img.rawdata(), img.dim());
	return EXIT_SUCCESS;
}

auto savepam_compressed(mat<int>& img, std::vector<std::tuple<uint64_t, int, uint64_t>>&canonical_map, std::unordered_map<int, std::pair<uint64_t, uint64_t>>& code_map, const char* out) {
	std::ofstream os(out, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}

	bitwriter bw(os);

	os << "HUFFDIFF\n";
	os << "WIDTH ";
	int c = img.cols();
	os.write(reinterpret_cast<char*>(&c), 4);
	os << '\n';
	os << "HEIGHT ";
	int r = img.rows();
	os.write(reinterpret_cast<char*>(&r), 4);
	os << '\n';
	bw(canonical_map.size(), 9);

	// code_map [symbol, (coding, lenght)]
	std::sort(canonical_map.begin(), canonical_map.end());
	for (const auto& [len, sym, code] : canonical_map) {
		bw(sym, 9);
		bw(len, 5);
	}

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			auto& [code, len] = code_map[img(r, c)];
			bw(code, len);

		}
	}

	return EXIT_SUCCESS;
}



auto compress(const char* inputfile, const char* outputfile) {
	mat<uint8_t> img = loadpam(inputfile);

	freq f;

	mat<int> diffimg(img.rows(), img.cols());

	uint8_t prec = 0;
	for (size_t r = 0; r < diffimg.rows(); r++)
	{
		for (size_t c = 0; c < diffimg.cols(); c++)
		{
			diffimg(r, c) = img(r, c) - prec;
			prec = img(r, c);
		}
		prec = img(r, 0);
	}

	mat<uint8_t> diffdisplay(img.rows(), img.cols());
	for (size_t r = 0; r < diffdisplay.rows(); r++)
	{
		for (size_t c = 0; c < diffdisplay.cols(); c++)
		{
			diffdisplay(r, c) = ((diffimg(r, c) + 255) / 2) + 1;
		}
	}

	savepam(diffdisplay);

	for (const auto& elem : diffimg.data_) {
		f(elem);
	}

	huffman<int> h;

	h.build(f);
	h.canoic();
	

	savepam_compressed(diffimg, h.canonical_map, h.code_map, outputfile);
	return EXIT_SUCCESS;
}


auto decompress(const char* inputfile, const char* outputfile) {
	mat<uint8_t> img = loadhuffdiff(inputfile);

	savepam(img, outputfile);

	std::cout << "Ciao";
}



int main(int argc, char* argv[]) {
	
	if (argc != 4)
	{
		error("Usage is huffdiff [c|d] <input file> <output file>");
		return EXIT_FAILURE;
	}

	std::string mode(argv[1]);
	if (mode == "c")
	{
		compress(argv[2], argv[3]);
	}
	else if(mode == "d")
	{
		decompress(argv[2], argv[3]);
	}

	return EXIT_SUCCESS;

}