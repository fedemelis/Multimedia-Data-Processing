#include <iostream>
#include <fstream>
#include <variant>
#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <exception>

namespace bencode {

	struct parse_error : public std::runtime_error {
		parse_error() : std::runtime_error("Parse error") {}
	};

	struct elem {
		char type_;
		std::string s_;
		int64_t i_;
		std::vector<elem> l_;
		std::map<std::string, elem> d_;

		elem(std::istream& is) {
			type_ = is.peek();
			if (type_ == EOF) {
				throw parse_error();
			}
			if (type_ == 'i') {
				is.get(); // remove 'i'
				is >> i_;
				if (is.get() != 'e') {
					throw parse_error();
				}
			}
			else if (type_ == 'l') {
				is.get(); // remove 'l' 
				while (is.peek() != 'e') {
					l_.emplace_back(is);
				}
				is.get(); // remove 'e'
			}
			else if (type_ == 'd') {
				is.get(); // remove 'd'
				while (is.peek() != 'e') {
					elem key(is);
					if (key.type_ != 's') {
						throw parse_error();
					}
					d_.emplace(key.s_, is);
				}
				is.get(); // remove 'e'
			}
			else {
				type_ = 's';
				size_t len;
				is >> len;
				if (is.get() != ':') {
					throw parse_error();
				}
				s_.resize(len);
				is.read(s_.data(), len);
			}
		}

		void print(int tabs = 0, const std::string& key = "") const {
			if (key == "pieces") {
				for (size_t i = 0; i < s_.size(); ++i) {
					if (i % 20 == 0) {
						std::cout << '\n' << std::string(tabs + 1, '\t');
					}
					std::cout << std::format("{:02x}",
						static_cast<unsigned char>(s_[i]));
				}
				std::cout << '\n';
			}
			if (type_ == 'i') {
				std::cout << i_ << '\n';
			}
			else if (type_ == 'l') {
				std::cout << "[\n";
				for (const auto& x : l_) {
					std::cout << std::string(tabs + 1, '\t');
					x.print(tabs + 1);
				}
				std::cout << std::string(tabs, '\t') << "]\n";
			}
			else if (type_ == 'd') {
				std::cout << "{\n";
				for (const auto& [key, value] : d_) {
					std::cout << std::string(tabs + 1, '\t');
					std::cout << '"' << key << "\" => ";
					value.print(tabs + 1, key);
				}
				std::cout << std::string(tabs, '\t') << "}\n";
			}
			else {
				std::cout << '"';
				for (auto ch : s_) {
					if (ch < 32 || ch > 126) {
						std::cout << '.';
					}
					else {
						std::cout << ch;
					}
				}
				std::cout << "\"\n";
			}
		}
	};

}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cout << "SYNTAX\ntorrent_dump <file.torrent>";
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		std::cout << std::format("Cannot open {} for reading", argv[1]);
		return EXIT_FAILURE;
	}

	bencode::elem torrent(is);
	torrent.print();

	return EXIT_SUCCESS;
}