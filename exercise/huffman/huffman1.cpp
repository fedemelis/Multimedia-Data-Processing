#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>

class bitwriter {

	uint8_t buffer = 0;
	size_t n = 0;
	std::ostream& os_;

	void writebit(uint64_t bit) {
		buffer <<= 1;
		buffer |= (bit & 1);
		++n;
		if (n == 8)
		{
			os_.put(buffer);
			n = 0;
		}
	}

public:
	bitwriter (std::ostream& os) : os_(os) {}

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
		while (n > 0)
		{
			writebit(0);
		}
	}

	std::ostream& get_stream() {
		return os_;
	}

};

class bitreader
{
	uint8_t buffer = 0;
	size_t n = 0;
	std::istream& is_;

	uint8_t readbit() {
		if (n == 0)
		{
			buffer = is_.get();
			n = 8;
		}
		--n;
		return (buffer >> n) & 1;
	}
	

public:
	bitreader(std::istream& is) : is_(is) {}

	uint64_t operator()(int numbits) {
		uint64_t valbuf = 0;
		for (int i = numbits -1; i >= 0 ; i--)
		{
			valbuf <<= 1;
			uint8_t bit = readbit();
			valbuf |= bit;
		}
		return valbuf;
	}

	std::istream& get_stream() {
		return is_;
	}

};

class frequencies {
public:
	uint64_t total = 0;
	uint32_t count[256] = { 0 };
	std::vector<double> prob;
	void increment(const char& c) {
		++count[c];
		++total;
	}

};

struct table {
	uint8_t carattere[256] = { 0 };
	uint64_t encoding[256] = { 0 };
	int dim[256] = { 0 };

	int get_width(char c) {
		return std::bit_width(encoding[c]);
	}
};

struct occurrence
{
	char carattere = NULL;
	double prob = 0;
};

class Node
{
public:
	occurrence data;
	Node* left = nullptr;
	Node* right = nullptr;

	bool operator<(const Node& other) const {
		return data.prob < other.data.prob;
	}
};

void printTree(const std::string& prefix, const Node* node, bool isLeft) {
	if (node != nullptr) {
		std::cout << prefix;

		std::cout << (isLeft ? "|--" : "|--");

		// Stampare il valore del nodo
		std::cout << node->data.prob << std::endl;

		// Entrare nel livello successivo dell'albero - ramo sinistro e destro
		printTree(prefix + (isLeft ? "|   " : "    "), node->left, true);
		printTree(prefix + (isLeft ? "|   " : "    "), node->right, false);
	}
}

void findBinaryRepresentation(Node* root, char target, std::vector<int>& path, std::vector<int>& winpath) {

	// Aggiungi il bit 0 al percorso e visita il sottoalbero sinistro
	if (root->left != nullptr)
	{
		path.push_back(0);
		findBinaryRepresentation(root->left, target, path, winpath);
		path.pop_back();
	}

	// Se il valore del nodo corrente è uguale al target, interrompi la ricerca
	if ((root->data.carattere == target) && root->left == nullptr && root->right == nullptr) {
		winpath.resize(path.size());
		std::copy(path.begin(), path.end(), winpath.begin());
		return;
	} 
	
	//se ho un nodo a dx
	if (root->right != nullptr) {
		path.push_back(1);
		findBinaryRepresentation(root->right, target, path, winpath);
		path.pop_back();
	}

	if (root->data.carattere == target && root->left == nullptr && root->right == nullptr) {
		winpath.resize(path.size());
		std::copy(path.begin(), path.end(), winpath.begin());
		return;
	}
}

void deleteTree(Node* root) {
	if (root) {
		deleteTree(root->left);
		deleteTree(root->right);
		delete root;
		root = nullptr; // Imposta il puntatore radice a null dopo la deallocazione
	}

}

int main(int argc, char* argv[]) {

	using namespace std::chrono;

	auto start = steady_clock::now();

	if (argv[1] == std::string("c"))
	{
		std::ifstream is(argv[2]);
		if (!is)
		{
			return EXIT_FAILURE;
		}

		is.seekg(0, std::ios::end);
		auto input_size = is.tellg();
		is.seekg(std::ios::beg);

		std::vector<char> caratteri(input_size);
		is.read(caratteri.data(), input_size);

		frequencies s;

		for (auto& c : caratteri) {
			if (c >= 0 && c <= 255)
			{
				s.increment(c);
			}
		}

		s.prob.resize(256);
		for (size_t i = 0; i < 256; i++)
		{
			s.prob[i] = static_cast<double>(s.count[i]) / static_cast<double>(s.total);
		}

		std::vector<Node> leaf(s.prob.size());

		for (size_t i = 0; i < s.prob.size(); i++)
		{
			leaf[i].data.carattere = static_cast<char>(i);
			leaf[i].data.prob = s.prob[i];
			leaf[i].left = nullptr;
			leaf[i].right = nullptr;
		}
		int n_iter = 1;

		std::sort(leaf.begin(), leaf.end());
		leaf.erase(std::remove_if(leaf.begin(), leaf.end(), [](const Node& n) {return n.data.prob == 0; }), leaf.end());
		std::vector<Node> tree(leaf.size());

		std::copy(leaf.begin(), leaf.end(), tree.begin());
		std::sort(tree.begin(), tree.end());

		Node* newnode = nullptr;
		auto sizetree = tree.size();
		while (true)
		{
			//first min element
			auto minleft = std::min_element(tree.begin(), tree.end());
			if (minleft == tree.end())
			{
				break;
			}
			n_iter += 1;
			Node* nmin1 = new Node(*minleft);

			//second min element
			auto minright = std::min_element(tree.begin() + 1, tree.end());
			if (minright == tree.end())
			{
				break;
			}
			Node* nmin2 = new Node(*minright);

			//adjusting node vector
			std::vector<Node> tmp(tree);
			tree.clear();
			tree.resize(sizetree - n_iter);
			std::copy(tmp.begin() + 2, tmp.end(), tree.begin());

			//new top node
			newnode = new Node();
			newnode->data.prob = (nmin1->data.prob + nmin2->data.prob);
			newnode->left = nmin1;
			newnode->right = nmin2;
			tree.push_back(*newnode);

			//sorting just in case
			std::sort(tree.begin(), tree.end());
		}
		//printTree("", newnode, false);
		std::cout << '\n';

		std::vector<int> binaryRepresentation;
		std::vector<int> win;
		table encoding;

		for (int i = 0; i < sizetree; i++)
		{
			auto c = leaf[i].data.carattere;
			std::cout << "Cerco " << c << ": ";
			findBinaryRepresentation(newnode, c, binaryRepresentation, win);
			for (auto& elem : win) {
				std::cout << elem;
			}
			encoding.carattere[c] = c;
			uint64_t buffer = 0;
			for (int ii = 0; ii < win.size(); ii++) {
				buffer <<= 1;
				buffer |= win[ii];
			}
			encoding.encoding[c] = buffer;
			encoding.dim[c] = static_cast<int>(win.size());
			buffer = 0;
			std::cout << '\n';
			binaryRepresentation.erase(binaryRepresentation.begin(), binaryRepresentation.end());
			win.erase(win.begin(), win.end());
		}

		std::ofstream os(argv[3], std::ios::binary);
		if (!os)
		{
			return EXIT_FAILURE;
		}

		bitwriter writer(os);
		os.write("HUFFMAN1", 8);
		uint8_t numbers = static_cast<uint8_t>(leaf.size());
		os.put(numbers);
		//std::sort(leaf.begin(), leaf.end());

		for (int i = 0; i < sizetree; i++)
		{
			auto c = leaf[i].data.carattere;
			writer(c, 8);
			auto width = encoding.dim[c];
			writer(width, 5);
			writer(encoding.encoding[c], width);
		}

		uint32_t numencoded = static_cast<uint32_t>(caratteri.size());
		writer(numencoded, 32);
		//os.put(std::byteswap(numencoded));

		for (size_t i = 0; i < caratteri.size(); i++)
		{
			char c = caratteri[i];
			writer(encoding.encoding[c], encoding.dim[c]);
			if (!writer.get_stream())
			{
				break;
			}
		}

		deleteTree(newnode);
	}
	else
	{
		std::ifstream is(argv[2], std::ios::binary);
		if (!is)
		{
			return EXIT_FAILURE;
		}
		bitreader reader(is);
		char trash8[8];
		is.read(trash8, 8);
		uint8_t entries = 0;
		is.read(reinterpret_cast<char*>(&entries), 1);
		//std::vector<char> decoding;
		std::vector<char> decoding(256);
		std::vector<int64_t> dec(256);
		for (size_t i = 0; i < dec.size(); i++)
		{
			dec[i] = -1;
		}
		std::vector<size_t> length(256);
		char c;
		for (size_t i = 0; i < entries; i++)
		{
			c = 0;
			c = static_cast<char>(reader(8));
			if (c >= 0 && c <= 255)
			{
				decoding[c] = c;
				uint8_t len = static_cast<uint8_t>(reader(5));
				length[c] = len;
				dec[c] = reader(len);
			}
		}

		uint32_t numsymbol = 0;
		numsymbol = static_cast<uint32_t>(reader(32));
		std::ofstream os(argv[3]);
		char cc = 0;
		for (size_t i = 0; i < numsymbol; i++)
		{
			cc =static_cast<char>(reader(1));
			auto res = std::find(dec.begin(), dec.end(), cc);
			while (res == dec.end())
			{
				cc <<= 1;
				cc |= reader(1);
				res = std::find(dec.begin(), dec.end(), cc);
			}
			res = std::find(dec.begin(), dec.end(), cc);
			char pos = static_cast<char>(std::distance(dec.begin(), res));
			os << pos;
		}
	}

	

	auto end = steady_clock::now();
	auto diff = end - start;

	auto duration_ms = duration<double, std::milli>(diff);

	std::cout << "Tempo di esecuzione: " << duration_ms << '\n';

	return 0;
}