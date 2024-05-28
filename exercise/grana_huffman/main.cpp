#include <algorithm>
#include <array>
#include <bit>
#include <format>
#include <fstream>
#include <iterator>
#include <iostream>
//#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <random>

#define print(...) cout << std::format(__VA_ARGS__)
#define println(...) cout << std::format(__VA_ARGS__) << "\n"

void syntax() {
	std::print("SYNTAX:\n"
		"huffman1 [c|d] <filein> <fileout>\n");
	exit(EXIT_FAILURE);
}

void error(std::string_view s) {
	std::println("ERROR: {}", s);
	exit(EXIT_FAILURE);
}

class bitwriter {
	uint8_t buffer_;
	size_t n_ = 0;
	std::ostream& os_;

	void writebit(uint64_t curbit) {
		buffer_ = (buffer_ << 1) | (curbit & 1);
		++n_;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
	}
public:
	bitwriter(std::ostream& os) : os_(os) {}
	~bitwriter() {
		flush();
	}
	std::ostream& operator()(uint64_t x, uint64_t numbits) {
		while (numbits-- > 0) {
			writebit(x >> numbits);
		}
		return os_;
	}
	std::ostream& flush(int padbit = 0) {
		while (n_ > 0) {
			writebit(padbit);
		}
		return os_;
	}
};

class bitreader {
	uint8_t buffer_;
	size_t n_ = 0;
	std::istream& is_;

	uint64_t readbit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
public:
	bitreader(std::istream& is) : is_(is) {}
	uint64_t operator()(uint64_t numbits) {
		uint64_t u = 0;
		while (numbits-- > 0) {
			u = (u << 1) | readbit();
		}
		return u;
	}
	bool fail() const {
		return is_.fail();
	}
	operator bool() const {
		return !fail();
	}
};

template<typename T, typename CT = uint64_t>
struct frequency {
	std::unordered_map<T, CT> counters_;
	void operator()(const T& val) {
		++counters_[val];
	}
	auto begin() const { return counters_.begin(); }
	auto end() const { return counters_.end(); }
	auto size() const { return counters_.size(); }
};

template<typename T>
struct huffman {
	struct node {
		T sym_;
		uint64_t freq_;
		node* left_;
		node* right_;
	};


	//ridefinizione del confronto
	struct nodeptr_less {
		bool operator()(const node* a, const node* b) {
			return a->freq_ > b->freq_;
		}
	};

	// con unique_ptr non devo occuparmi di liberare la memoria
	std::vector<std::unique_ptr<node>> nodes_;

	// ripeti un certo numero di volte typename Ts
	template<typename... Ts>
	node* make_node(Ts... args) { //accetto un numero arbitrario di parametri
		nodes_.emplace_back(std::make_unique<node>(std::forward<Ts>(args)...)); //(args)... vuol dire fai unpack di args
		return nodes_.back().get();
	}

	std::unordered_map<T, std::pair<uint64_t, uint64_t>> code_map; // sym -> {code, len}
	std::vector<std::tuple<uint64_t, T, uint64_t>> canonical; // {len, sym, code}

	void generate_codes(const node* n, uint64_t code = 0, uint64_t len = 0) {
		// mi basta controllare se uno dei due figli esiste perchè o ci sono entrambi o non ci sono entrambi
		if (n->left_ == 0) {
			code_map[n->sym_] = { code, len };
		}
		else {
			generate_codes(n->left_, (code << 1) | 0, len + 1);
			generate_codes(n->right_, (code << 1) | 1, len + 1);
		}
	}

	void make_canonical() {
		canonical.clear();
		for (const auto& [sym, x] : code_map) {
			auto&& [code, len] = x;
			canonical.emplace_back(len, sym, code);
		}
		std::ranges::sort(canonical);
		uint64_t curcode = 0, curlen = 0;
		for (auto& [len, sym, code] : canonical) {
			uint64_t addbits = len - curlen;
			curcode <<= addbits;
			code_map[sym].first = curcode;
			code = curcode;
			curlen = len;
			++curcode;
		}
	}

	huffman() {}

	template<std::ranges::range R >
	huffman(R&& r) {
		std::vector<node*> v;
		for (auto it = std::begin(r); it != std::end(r); ++it) {
			v.emplace_back(make_node(it->first, it->second));
		}
		std::ranges::sort(v, nodeptr_less{});

		while (v.size() > 1) {
			node* n1 = v.back();
			v.pop_back();
			node* n2 = v.back();
			v.pop_back();


			//costruisco un nodo con 5 parametri, il primo parametro non ha significato poiché non siamo in una foglia
			node* n = make_node(T{}, n1->freq_ + n2->freq_, n1, n2);

			auto pos = std::ranges::lower_bound(v, n, nodeptr_less{});
			v.insert(pos, n);
		}
		node* root = v.back();
		v.pop_back();

		generate_codes(root);
	}
};

void compress(const std::string& input_filename, const std::string& output_filename) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is) {
		error("Cannot open file " + input_filename + " for reading");
	}

	std::vector<uint8_t> v{ std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>() };

	auto f = std::ranges::for_each(v, frequency<uint8_t, uint64_t>{}).fun;

	huffman<uint8_t> h(f);

	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		error("Cannot open file " + output_filename + " for writing");
	}
	bitwriter bw(os);

	os << "HUFFMAN1";
	os.put(static_cast<char>(f.size()));

	h.make_canonical();
	// const auto& [x, y] scompatta una coppia

	for (const auto& [sym, x] : h.code_map) {
		auto&& [code, len] = x;
		bw(sym, 8);
		bw(len, 5);
		//bw(code, len);
		std::cout << "Symbol: " << sym << "\tLen: " << +len << std::endl;
	}
	bw(v.size(), 32);
	for (const auto& x : v) {
		auto&& [code, len] = h.code_map[x];
		bw(code, len);
	}
}

void decompress(const std::string& input_filename,
	const std::string& output_filename) {

	std::ifstream is(input_filename, std::ios::binary);
	if (!is) {
		error("Cannot open file " + input_filename + " for reading");
	}
	bitreader br(is);



	/*std::string test;
	is.read(const_cast<char*>(test.c_str()), 8);*/

	std::string magic(8, ' ');
	is.read(magic.data(), 8);
	if (magic != "HUFFMAN1") {
		return;
	}
	size_t tblsize = is.get();
	if (tblsize == 0) {
		tblsize = 256;
	}
	std::vector<std::tuple<uint64_t, uint8_t, uint64_t>> tbl; // len,sym,code
	for (size_t i = 0; i < tblsize; ++i) {
		uint8_t sym = static_cast<uint8_t>(br(8));
		uint64_t len = br(5);
		uint64_t code = br(len);
		tbl.emplace_back(len, sym, code); //emplace back prende gli argomenti e li passa al costruttore del tipo di dati contenuto nel vettore
	}
	std::ranges::sort(tbl);
	size_t numsym = br(32);

	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		error("Cannot open file " + output_filename + " for writing");
	}
	for (size_t i = 0; i < numsym; ++i) {
		uint64_t curlen = 0;
		uint64_t curcode = 0;
		size_t tblpos;
		for (tblpos = 0; tblpos < tbl.size(); ++tblpos) {
			auto& [len, sym, code] = tbl[tblpos];
			auto readbits = len - curlen;
			curcode = (curcode << readbits) | br(readbits);
			curlen = len;
			if (curcode == code) {
				os.put(sym);
				break;
			}
		}
		if (tblpos == tbl.size()) {
			error("Cannot find Huffman code!");
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		syntax();
	}

	std::string command = argv[1];
	if (command == "c") {
		compress(argv[2], argv[3]);
	}
	else if (command == "d") {
		decompress(argv[2], argv[3]);
	}
	else {
		syntax();
	}

	return EXIT_SUCCESS;
}