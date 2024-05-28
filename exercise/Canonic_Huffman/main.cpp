#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <map>
#include <memory>
#include <bitset>

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
	std::map<uint8_t, uint64_t> f_; // mappa <carattere, frequenza>

	void compute(std::vector<uint8_t>& data) {
		for (const auto& elem : data) {
			++f_[elem];
		}
	}

	const auto& get_freq() const  {
		return f_;
	}
};	

struct node
{
	uint8_t sym_;
	uint64_t freq_;
	uint8_t len_ = 0;

	node* left_ = nullptr;
	node* right_ = nullptr;

	std::map<uint8_t, uint32_t>sym_code_map;
	std::vector<std::pair<uint8_t, uint8_t>> lenmap;
	std::map<uint8_t, uint8_t> len_mapping;


	node() {}

	node(uint8_t sym, uint64_t freq, node* left = nullptr, node* right = nullptr) : sym_(sym), freq_(freq), left_(left), right_(right) {}

	bool operator()(const node* a, const node* b) {
			return a->freq_ > b->freq_;
		}

	void compute_lenght(node* root, uint8_t lvl, std::vector<std::pair<uint8_t, uint8_t>>&lenmap) {
		if (root->left_ == nullptr)
		{
			lenmap.emplace_back(root->sym_, lvl);
			len_mapping[root->sym_] = lvl;
			return;
		}

		if (root->left_ != nullptr)
		{
			compute_lenght(root->left_, lvl + 1, lenmap);
		}
		if (root->right_ != nullptr)
		{
			compute_lenght(root->right_, lvl + 1, lenmap);
		}
		return;
	}

	void compute_canonical_code(std::map<uint8_t, uint32_t>& sym_code_map, std::vector<std::pair<uint8_t, uint8_t>>& sym_len_map) {
		int curlen = 0;
		uint64_t curval = 0;
		for(const auto& [sym, len] : sym_len_map) {
			curval <<= len - curlen;
			curlen = len;
			sym_code_map[sym] = curval;
			++curval;
		}
	}

	
	auto buildCode(freq& f) {
		
		std::vector<std::unique_ptr<node>> nodeManager(0);

		for (const auto& [sym, len] : f.get_freq()) {
			nodeManager.emplace_back(std::make_unique<node>(sym, len, nullptr, nullptr));
		}

		std::vector<node*> nodes;
		
		for (size_t i = 0; i < nodeManager.size(); i++)
		{
			auto nodeptr = nodeManager[i].get();
			nodes.push_back(nodeptr);
		}

		std::sort(nodes.begin(), nodes.end(), [](const node* lhs, const node* rhs) {return lhs->freq_ > rhs->freq_; });

		while (nodes.size() > 1)
		{
			node* l = nodes.back();
			nodes.pop_back();

			node* r = nodes.back();
			nodes.pop_back();

			node n(uint8_t{}, l->freq_ + r->freq_, l, r);

			nodeManager.push_back(std::make_unique<node>(n));
			auto n_node = nodeManager.back().get();
			auto it = std::lower_bound(nodes.begin(), nodes.end(), n_node, [](const node* lhs, const node* rhs) {return lhs->freq_ > rhs->freq_; });
			nodes.insert(it, n_node);
		}

		node* root = nodes.back();


		compute_lenght(root, 0, lenmap);
		
		//std::cout << "ciao";
		std::sort(lenmap.begin(), lenmap.end(), [](const std::pair<uint8_t, uint8_t> lhs, const std::pair<uint8_t, uint8_t> rhs)
			{ 
				if (lhs.second != rhs.second)
				{
					return lhs.second < rhs.second;
				}
				else
				{
					return lhs.first < rhs.first;
				}
				 
			});

		compute_canonical_code(sym_code_map, lenmap);

	}

	

};

int error(const char* errmgs) {
	std::cerr << errmgs << std::endl;
	return EXIT_FAILURE;
}

auto encode(const char* inputfile, const char* outputfile){
	
	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while opening inputfile for encoding");
	}

	is.seekg(0, std::ios::end);
	auto dim = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<uint8_t> data(dim);

	is.read(reinterpret_cast<char*>(data.data()), dim);

	freq f;
	
	f.compute(data); // calculate frequency 

	node n;

	n.buildCode(f);

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file for decoding");
	}

	bitwriter br(os);

	std::string magicnumber("HUFFMAN2");
	os.write(reinterpret_cast<char*>(magicnumber.data()), 8);

	uint8_t tablesize = n.lenmap.size();
	os.put(tablesize);

	for (const auto& [sym, len] : n.lenmap) {
		br(sym, 8);
		br(len, 5);
		//std::cout << +elem.second;
		std::cout << "Symbol: " << sym << "\tLen: " << +len << std::endl;
	}

	uint32_t n_coded = data.size();
	br(n_coded, 32);

	for (const auto& elem : data) {
		br(n.sym_code_map[elem], n.len_mapping[elem]);
	}
		
	return EXIT_SUCCESS;


}

auto decode(const char* inputfile, const char* outputfile) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return error("Error while openign input file for decoding");
	}

	bitreader br(is);

	std::string magicnumber(8, ' ');
	is.read(reinterpret_cast<char*>(magicnumber.data()), 8);
	
	if (magicnumber != "HUFFMAN2")
	{
		return error("Magicnumber should be \"HUFFMAN2\"");
	}

	uint8_t tablesize = is.get();
	std::map<uint8_t, uint8_t> len_map;
	std::vector<std::pair<uint8_t, uint8_t>> len_vec;

	for (size_t i = 0; i < tablesize; i++)
	{
		uint8_t sym = br(8);
		uint8_t len = br(5);
		len_map[sym] = len;
		len_vec.emplace_back(sym, len);
	}

	std::sort(len_vec.begin(), len_vec.end(), [](const std::pair<uint8_t, uint8_t> lhs, const std::pair<uint8_t, uint8_t> rhs)
		{
			if (lhs.second != rhs.second)
			{
				return lhs.second < rhs.second;
			}
			else
			{
				return lhs.first < rhs.first;
			}

		});

	uint32_t nsym = 0;
	nsym = br(32);

	std::map<uint64_t, uint8_t> code_sym_map;

	int curlen = 0;
	uint32_t val = 0;
	for (const auto& [sym, len] : len_vec) {
		val <<= len - curlen;
		curlen = len;
		code_sym_map[val] = sym;
		++val;
	}

	std::ofstream os(outputfile, std::ios::binary);
	if (!os)
	{
		return error("Error while opening output file for decoding");
	}

	int decoded = 0;
	uint64_t cur_len = 0;
	uint64_t curbuf = 0;
	while (decoded < nsym)
	{
		curbuf <<= 1;
		curbuf |= br(1);
		++cur_len;
		if (code_sym_map.contains(curbuf))
		{
			if (cur_len == len_map[code_sym_map[curbuf]])
			{
				++decoded;
				os.put(code_sym_map[curbuf]);
				curbuf = 0;
				cur_len = 0;
			}
		}
	}


}


int main(int argc, char* argv[]) {
	
	if (argc != 4)
	{
		return error("Usage is: huffman2 [c|d] <input file> <output file>");
	}

	std::string mode(argv[1]);

	if (mode == "c")
	{
		encode(argv[2], argv[3]);
	}
	else
	{
		decode(argv[2], argv[3]);
	}

}