#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>
#include <tuple>
#include <map>

struct bitwriter {

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

	void operator()(uint64_t elem, int numbits) {
		for (int i = numbits - 1; i >= 0; i--)
		{
			writebit(elem >> i);
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
		return (buffer_ >> n_) & 1;

	}

	bitreader(std::istream& is) : is_(is) {}

	auto operator()(int numbits) {
		uint64_t valbuf = 0;
		while (numbits --> 0)
		{
			valbuf <<= 1;
			uint8_t bit = readbit();
			valbuf |= bit;
		}
		return valbuf;
	}
};

struct encoding
{
	uint8_t symbol;
	uint32_t len, val;
};

struct HuffmanTree
{
	struct Node {
		uint8_t symbol_;
		double prob_;
		Node* left_ = nullptr;
		Node* right_ = nullptr;
		encoding encode;

		Node() {}

		Node(uint8_t s, double p) : symbol_(s), prob_(p) {}

		Node(Node* l, Node* r) : prob_(l->prob_ + r->prob_), left_(l), right_(r) {}

		bool operator <(const Node& rhs) {
			return prob_ > rhs.prob_;
		}
	};

	void explore(Node* node, std::vector<encoding>& table, uint32_t len, uint32_t val) {
		if ((node->left_ == nullptr) && (node->right_ == nullptr))
		{
			table.push_back({ node->symbol_, len, val });
			return;
		}

		if (node->left_ != nullptr)
		{
			explore(node->left_, table, len + 1, ((val << 1) | 0)); // or 0 non ha senso ma è solo per capire meglio
		}
		if (node->right_ != nullptr)
		{
			explore(node->right_, table, len + 1, ((val << 1) | 1));
		}
	}

	auto create_table(const std::unordered_map<uint8_t, uint32_t>& map) {
		std::vector<Node> vec;
		std::vector<Node> backup;
		for (const auto& val : map) {
			Node* n = new Node(val.first, val.second);
			vec.push_back(*n);
			backup.push_back(*n);
		}
		std::sort(vec.begin(), vec.end());

		while (vec.size() > 1)
		{
			Node* left = new Node();
			*left = vec.back();
			vec.pop_back();
			Node* right = new Node();
			*right = vec.back();
			vec.pop_back();

			Node* p = new Node(left, right);

			vec.push_back(*p);
			std::sort(vec.begin(), vec.end());
		}
		Node root = vec.back();

		std::vector<encoding> encodingtable;

		explore(&root, encodingtable, 0, 0);
		return encodingtable;
	}
};

auto compress(const char* outputfile, std::vector<uint8_t>& caratteri, std::vector<encoding>& table) {

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	bitwriter bw(os);

	os.write("HUFFMAN1", 8);
	uint8_t size = table.size();
	os.put(size);
	for (const auto& elem : table) {
		bw(elem.symbol, 8);
		bw(elem.len, 5);
		bw(elem.val, elem.len);
	}

	uint32_t numsymb = caratteri.size();
	bw(numsymb, 32);

	std::unordered_map<uint8_t, encoding> search_map;
	for (const auto& elem : table) {
		search_map[elem.symbol] = elem;
	}

	for (const auto& elem : caratteri) {
		auto val = search_map[elem];
		bw(val.val, val.len);
	}

}

struct freq
{
	std::unordered_map<uint8_t, uint32_t> fmap;
	uint64_t count;

	auto operator()(uint8_t pos) {
		++fmap[pos];
		++count;
	}

	auto get_freq() {
		return fmap;
	}

};

void error(const char* msg) {
	std::cout << msg;
}

auto encode(const char* inputfile, const char* outputfile) {
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(std::ios::beg);

	std::vector<uint8_t> caratteri(size);
	is.read(reinterpret_cast<char*>(caratteri.data()), size);

	freq f;
	
	for (const auto& elem : caratteri) {
		f(elem);
	}

	auto map = f.get_freq();

	HuffmanTree h;
	auto table = h.create_table(map);

	compress(outputfile, caratteri, table);


}

auto decode(const char* inputfile, const char* outputfile) {
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}
	
	std::string magicnumber(8, ' ');
	is.read(const_cast<char *>(magicnumber.c_str()), 8);

	if (magicnumber != "HUFFMAN1")
	{
		return EXIT_FAILURE;
	}

	uint8_t tabledim = is.get();

	std::vector<encoding> table;
	bitreader br(is);

	for (size_t i = 0; i < tabledim; i++)
	{
		encoding entry;
		entry.symbol = br(8);
		entry.len = br(5);
		entry.val = br(entry.len);
		table.push_back(entry);
	}

	uint32_t numsymb = 0;
	numsymb = br(32);
	std::map<std::pair<uint32_t, uint32_t>, encoding> search_map;
	for (const auto& elem : table) {
		std::pair<uint32_t, uint32_t> pair(elem.val, elem.len);
		search_map[pair] = elem;
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	bitwriter bw(os);
	bool done = false;

	for (size_t i = 0; i < numsymb; i++)
	{
		uint32_t buffered = 0;
		uint32_t len = 0;
		while (!done)
		{
			buffered <<= 1;
			buffered |= br(1);
			len += 1;
			std::pair<uint32_t, uint32_t> pair(buffered, len);
			auto candidate = search_map.find(pair);
			if (candidate != search_map.end())
			{
				bw(candidate->second.symbol, 8);
				len = 0;
				buffered = 0;
				done = true;
			}
		}
		done = false;

	}

}

int main(int argc, char* argv[]) {
	using namespace std;

	if (argc != 4)
	{
		error("Usage is: [c|d] input output\n");
		return EXIT_FAILURE;
	}

	if (string(argv[1]) == "c")
	{
		encode(argv[2], argv[3]);
	}
	else if(string(argv[1]) == "d")
	{
		decode(argv[2], argv[3]);
	}

	return 0;
}