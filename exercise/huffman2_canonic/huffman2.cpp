#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <string>


struct bitwriter
{
	uint8_t buffer_;
	size_t n_ = 0;
	std::ostream& os_;

	void writebit(uint64_t bit) {
		buffer_ <<= 1;
		buffer_ |= (bit & 1);
		++n_;
		if (n_ == 8)
		{
			os_.put(buffer_);
			n_ = 0;
		}
	}

	bitwriter(std::ostream& os) : os_(os) {}

	~bitwriter() {
		flush();
	}

	void operator()(uint64_t val, int nbits) {
		for (int i = nbits - 1 ; i >= 0; --i)
		{
			writebit(val >> i);
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
	std::istream& is_;

	uint8_t readbit() {
		if (n_ == 0)
		{
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return ((buffer_ >> n_) & 1);
	}
	

	bitreader(std::istream& is) : is_(is) {}

	uint64_t operator()(int numbits) {
		uint64_t valbuf = 0;
		for (int i = numbits - 1; i >= 0; --i)
		{
			valbuf <<= 1;
			valbuf |= readbit();
		}
		return valbuf;
	}
};

void error(const char* errormsg) {
	std::cout << errormsg << '\n';
}
template <typename T>
struct table
{
	T symbol;
	uint32_t len;

	table(T s, uint32_t l) : symbol(s), len(l) {}

	bool operator<(const table& rhs) {
		if (len != rhs.len)
		{
			return len < rhs.len;
		}
		return symbol < rhs.symbol;
	}
};


template <typename T>
struct huffmanTree
{
	struct node
	{
		T symb_ = 0;
		node* left_ = nullptr;
		node* right_ = nullptr;
		double prob_;

		//base constructor
		node() {}

		//leaf node constructor
		node(T s, double p) : symb_(s), prob_(p) {}

		//parent node constructor
		node(node *l, node *r) : left_(l), right_(r), prob_(l->prob_ + r->prob_) {}

		bool operator<(const node& rhs) {
			return prob_ > rhs.prob_;
		}

	};

	void explore(const node* n, uint32_t len, std::vector<table<uint8_t>>& hufftable) {
		
		if (n->left_ == nullptr && n->right_ == nullptr)
		{
			hufftable.push_back({ n->symb_, len });
			return;
		}
	
		if (n->left_ != nullptr)
		{
			explore(n->left_, len + 1, hufftable);
		}
		if (n->right_ != nullptr)
		{
			explore(n->right_, len + 1, hufftable);
		}
	
	}

	auto construct_table(std::unordered_map<T, uint32_t>& map) {
		std::vector<node> v;
		for (const auto& elem : map) {
			node n(elem.first, elem.second);
			v.push_back(n);
		}
		std::sort(v.begin(), v.end());

		while (v.size() > 1)
		{
			node *left = new node();
			*left = v.back();
			v.pop_back();
			node* right = new node();
			*right = v.back();
			v.pop_back();
			node *newnode = new node(left, right);
			v.push_back(*newnode);

			std::sort(v.begin(), v.end());

		}
		node root = v.back();
		std::vector<table<uint8_t>>lenghts;
		explore(&root, 0, lenghts);

		std::sort(lenghts.begin(), lenghts.end());
		return lenghts;
	}
	
};

template <typename T>
struct freq
{
	uint32_t totcount = 0;
	std::unordered_map<T, uint32_t> f;

	void operator()(const T& occ) {
		++f[occ];
		++totcount;
	}

	auto get_freq() {
		return f;
	}
};


int encode(const char* inputfile, const char* outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		error("Error during the opening of input file");
		return EXIT_FAILURE;
	}

	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<uint8_t> caratteri(size);
	is.read(reinterpret_cast<char*>(caratteri.data()), size);

	freq<uint8_t> f;

	for (const auto& elem : caratteri) {
		f(elem);
	}

	auto frequenze = f.get_freq();

	huffmanTree<uint8_t> h;

	auto len = h.construct_table(frequenze);

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}

	os.write("HUFFMAN2", 8);

	bitwriter bw(os);

	uint8_t tablesize = len.size();
	bw(tablesize, 8);

	for (const auto& elem : len) {
		bw(elem.symbol, 8);
		bw(elem.len, 5);
	}

	uint32_t numSymbols = caratteri.size();
	bw(numSymbols, 32);
	
	std::map<uint8_t, uint32_t> symbmp;
	std::map<int, std::pair<uint8_t, uint32_t>> search_map;
	for (size_t i = 0; i < len.size(); i++)
	{
		symbmp[len[i].symbol] = len[i].len;
		search_map[i] = std::pair(len[i].symbol, len[i].len);
	}

	uint64_t base = 0;
	size_t current_len = 0;
	std::map<uint8_t, uint32_t> canonical_map;
	for (const auto& elem : search_map) {
		uint8_t sym = elem.second.first;
		auto symlen = elem.second.second;
		for (size_t ii = 0; ii < symlen - current_len; ii++)
		{
			base <<= 1;
		}
		current_len = symlen;
		canonical_map[sym] = base;
		++base;
	}

	for (size_t i = 0; i < numSymbols; i++)
	{
		auto c = caratteri[i];
		auto l = symbmp[c];
		bw(canonical_map[c], l);
	}
	return EXIT_SUCCESS;
}

int decode(const char* inputfile, const char* outputfile) {
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return EXIT_FAILURE;
	}

	std::string magicnumber(8, ' ');
	is.read(magicnumber.data(), 8);

	if (magicnumber != "HUFFMAN2")
	{
		error("Incorrect format. It should be HUFFMAN2");
		return EXIT_FAILURE;
	}

	uint8_t entriesnumb = 0;
	

	std::vector<table<uint8_t>> t;
	bitreader br(is);

	entriesnumb = br(8);

	for (size_t i = 0; i < entriesnumb; i++)
	{
		uint8_t symbol = br(8);
		uint32_t len = br(5);
		t.push_back({ symbol, len });
	}
	 
	std::map<std::pair<uint8_t, uint32_t>, uint32_t>decoding;
	for (const auto& elem : t) {
		std::pair<uint8_t, uint32_t> tmp(elem.symbol, elem.len);
		decoding[tmp] = 0;
	}

	uint32_t traduction = 0;
	size_t curlen = 0;
	for (const auto& elem : t) {
		auto len = elem.len;
		traduction <<= (len - curlen);
		std::pair<uint8_t, uint32_t>tmp(elem.symbol, elem.len);
		decoding[tmp] = traduction;
		++traduction;
		curlen = len;
	}

	// mappa -> key = (encoding, lenght) : value = (char)
	std::map<std::pair<uint32_t, uint32_t>, uint8_t> traduction_map;
	
	for (const auto& elem : decoding) {
		std::pair<uint32_t, uint32_t> tmp(elem.second, elem.first.second);
		traduction_map[tmp] = elem.first.first;
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		error("Error while opening output file");
		return EXIT_FAILURE;
	}
	uint32_t ndec = br(32);
	uint32_t buf = 0;
	size_t dim = 0;
	bool done = false;
	
	for (size_t i = 0; i < ndec; i++)
	{
		while (!done)
		{
			buf <<= 1;
			buf |= (br(1) & 1);
			++dim;
			std::pair<uint32_t, uint32_t> candidate(buf, dim);
			auto res = traduction_map.find(candidate);
			if (res != traduction_map.end())
			{
				auto s = (*res).second;
				os.put(s);
				buf = 0;
				dim = 0;
				done = true;
			}
		}
		done = false;
	}

	return EXIT_SUCCESS;

}


int main(int argc, char* argv[]) {
	
	if (argc != 4)
	{
		error("Usage is: [c|d] input output");
		return EXIT_FAILURE;
	}

	std::string action = argv[1];

	if (action == "c")
	{
		encode(argv[2], argv[3]);
	}
	else if(action == "d")
	{
		decode(argv[2], argv[3]);
	}
	else
	{
		error("The first parameter must be: [c | d]");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}