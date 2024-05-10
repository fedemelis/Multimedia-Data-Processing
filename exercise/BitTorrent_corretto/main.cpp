#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <format>


void error(const char* errmsg) {
	std::cerr << errmsg << '\n';
}


struct element
{
	char type_; // type could be: i -> integer, s -> string, l -> list, d -> dictionary
	int64_t i_;
	std::string s_;
	std::vector<element> l_;
	std::map<std::string, element> d_;

	void read_rec(std::ifstream& is) {
		type_ = is.peek(); // checking what is next byte, but not popping it from stream
		if (type_ == EOF)
		{
			return;
		}
		else if (type_ == 'd')
		{
			is.get(); // removing initial 'd'
			while (is.peek() != 'e')
			{
				element keystring; // creating new element that will must be a string
				keystring.read_rec(is); // reading the string
				element val;
				val.read_rec(is); // val can be i, s, l, d
				d_.insert({ keystring.s_, val }); // inserting the new dictionary element in d_
			}
			is.get(); // remoing final 'e'
		}
		else if (type_ == 'l') {
			is.get(); // remove initial 'l'
			while (is.peek() != 'e')
			{
				element val;
				val.read_rec(is); // i will read an element
				l_.push_back(val); // adding the value readed in l_
			}
			is.get(); // removing final 'e'
		}
		else if (type_ == 'i') {
			is.get(); // removing initial 'i'
			is >> i_;
			is.get(); // removing final 'e'
		}
		else { // string case
			int dim = 0;
			is >> dim;
			is.get(); // removing ':', that separate lenght's string and string itself
			s_.resize(dim); // preparing the space for upcoming string
			is.read(s_.data(), dim);
		}
	}

	void tab(int tabs) {
		std::cout << std::string(tabs, '\t');
	}


	void print_rec(int tabs = 0, const std::string& key = "") {
		if (key == "pieces")
		{
			for (size_t i = 0; i < s_.size(); i++)
			{
				if ((i % 20) == 0)
				{
					std::cout << '\n';
					tab(tabs + 1);
				}
				std::cout << std::format("{:02x}", (uint8_t)s_[i]);
			}
			std::cout << '\n';
		}
		else if (type_ == 'd')
		{
			//tab(tabs);
			std::cout << "{\n";
			for (auto& [keystring, value] : d_) {
				tab(tabs + 1);
				std::cout << '"' << keystring << "\" => ";
				value.print_rec(tabs + 1, keystring);
			}
			tab(tabs);
			std::cout << "}\n";
		}
		else if (type_ == 'l') {
			//tab(tabs);
			std::cout << "[\n";
			for (auto& value : l_) {
				tab(tabs + 1);
				value.print_rec(tabs + 1);
			}
			tab(tabs);
			std::cout << "]\n";
		}
		else if (type_ == 'i') {
			//tab(tabs + 1);
			std::cout << i_ << '\n';
		}
		else {
			//tab(tabs + 1);
			std::cout << '"';
			for (const auto& c : s_) {
				if (c < 32 || c > 126) {
					std::cout << '.';
				}
				else
				{
					std::cout << c;
				}
			}
			std::cout << "\"\n";
		}
	}

};


int main(int argc, char* argv[]) {

	if (argc != 2)
	{
		error("Usage is torrent_dump <file.torrent>");
		return -1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return -1;
	}

	element e;
	e.read_rec(is);

	e.print_rec();


}