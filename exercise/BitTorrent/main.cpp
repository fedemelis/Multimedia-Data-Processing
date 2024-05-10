#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

void error(const char* errmsg) {
	std::cerr << errmsg << '\n';
}

struct elem
{
	char type;
	std::string s_;
	int64_t i_;
	std::vector<elem> l_;
	std::map<std::string, elem> d_;

	void build(std::ifstream& is) {
		type = is.peek();
		if (type == EOF)
		{
			return;
		}
		if (type == 'd')
		{
			is.get(); //remove d
			while (is.peek() != 'e')
			{
				elem key;
				key.build(is);
				elem val;
				val.build(is);
				d_.insert({ key.s_, val });
			}
			is.get(); //remove e
		}
		else if(type == 'l')
		{
			is.get(); //remove l
			while (is.peek() != 'e')
			{
				elem val;
				val.build(is);
				l_.push_back(val);
			}
			is.get(); //remove e
		}
		else if(type == 'i')
		{
			is.get(); //remove i
			is >> i_;
			is.get(); //remove e
		}
		else
		{
			type = 's';
			size_t len;
			is >> len;
			is.get(); //remove :
			s_.resize(len);
			is.read(reinterpret_cast<char*>(s_.data()), len);
		}
	}

	void print_rec(int tabs = 0, std::string k = "") {
		if (k == "pieces")
		{
			for (size_t i = 0; i < s_.length(); i++)
			{
				if ((i % 20) == 0)
				{
					std::cout << '\n' << std::string(tabs + 1, '\t');
				}
				std::cout << std::format("{:02x}", static_cast<uint8_t>(s_[i]));
			}
		}
		else if (type == 'd')
		{
			std::cout << "{\n";
			for (auto& [key, val] : d_) {
				std::cout << std::string(tabs + 1, '\t');
				std::cout << key << " => ";
				val.print_rec(tabs + 1, key);
			}
			std::cout << std::string(tabs, '\t') << "}\n";
		}
		else if (type == 'l') {

			std::cout << "[\n";
			for (auto& val : l_) {
				std::cout << std::string(tabs + 1, '\t');
				val.print_rec(tabs + 1);
			}
			std::cout << std::string(tabs, '\t');
			std::cout << "]\n";
		
		}
		else if (type == 'i') {
			std::cout << i_ << '\n';
		}
		else {
			std::cout << '"';
			for (const auto& c : s_) {
				if (c < 32 || c > 126)
				{
					//std::cout << '.';
					std::cout << c;
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

//auto readstring(std::vector<uint8_t>& buffer, uint8_t currentval) {
//	uint8_t val = currentval;
//	std::vector<uint8_t> dim_buff;
//
//	while (val != ':') {
//		dim_buff.push_back(val);
//		val = buffer.back();
//		buffer.pop_back();
//	}
//
//	std::string stringed_dim(dim_buff.begin(), dim_buff.end());
//
//	int s_dim = std::stoi(stringed_dim);
//
//	std::string s;
//	for (size_t i = 0; i < s_dim; i++)
//	{
//		s.push_back(buffer.back());
//		buffer.pop_back();
//	}
//
//	return s;
//}

//auto readint(std::vector<uint8_t>& buffer, uint8_t currentval) {
//
//	uint8_t val = currentval;
//	std::vector<uint8_t> dim_buff;
//
//	while (val != 'e')
//	{
//		dim_buff.push_back(val);
//		val = buffer.back();
//		buffer.pop_back();
//	}
//
//	std::string s(dim_buff.begin(), dim_buff.end());
//	int intero = std::atoi(s.c_str());
//
//	return intero;
//
//}

//void tabulate(size_t lvl) {
//	for (size_t i = 0; i < lvl; i++)
//	{
//		std::cout << '\t';
//	}
//}

//
//int construct_rec(std::vector<uint8_t>& buffer, uint8_t current_val, int lvl, bool iskey = false, bool indict = false) {
//
//	current_val = buffer.back();
//	buffer.pop_back();
//	if (current_val == 'e')
//	{
//		return 0;
//	}
//	if (current_val == 'd')
//	{
//		tabulate(lvl);
//		std::cout << "{\n";
//		construct_rec(buffer, current_val, ++lvl, true, true);
//		tabulate(lvl - 1);
//		std::cout << "}\n";
//		// entro dentro e leggo gli elementi del dizionario
//	}
//	else if (current_val == 'l')
//	{
//		/*auto next = buffer.back();
//		buffer.pop_back();*/
//
//		tabulate(lvl);
//		std::cout << "[\n";
//		construct_rec(buffer, current_val, ++lvl);
//		tabulate(lvl - 1);
//		std::cout << "]\n";
//		// entro dentro e leggo gli elementi della list
//	}
//	else if (current_val == 'i')
//	{
//		tabulate(lvl);
//		std::cout << readint(buffer, current_val) << '\n';
// 	}
//	else
//	{
//		if (indict)
//		{
//			if (iskey)
//			{
//
//				tabulate(lvl);
//				//std::cout << "Key: ";
//				std::cout << readstring(buffer, current_val) << "=>" ;
//				construct_rec(buffer, current_val, lvl, false, true);
//				return 0;
//			}
//			else
//			{
//				std::cout << '"';
//				std::cout << readstring(buffer, current_val);
//				std::cout << "\"\n";
//				construct_rec(buffer, current_val, lvl, true, true);
//				return 0;
//			}
//		}
//		else
//		{
//			tabulate(lvl);
//			std::cout << readstring(buffer, current_val);
//			std::cout << "\"\n";
//			
//		}
//	}
//
//	if (buffer.size() > 0)
//	{
//		construct_rec(buffer, current_val, lvl);
//	}
//
//	return 0;
//}




//auto loadfile(const char* input) {
//	
//	std::ifstream is(input, std::ios::binary);
//	if (!is)
//	{
//		error("Error while opening input file");
//		return EXIT_FAILURE;
//	}
//
//	is.seekg(0, std::ios::end);
//	auto dim = is.tellg();
//	is.seekg(0, std::ios::beg);
//
//	std::vector<uint8_t> filebuff(dim);
//	
//	is.read(reinterpret_cast<char*>(filebuff.data()), dim);
//
//	std::reverse(filebuff.begin(), filebuff.end());
//
//	construct_rec(filebuff, uint8_t{}, 0);
//
//	//std::cout << "Fine";
//
//
//
//}


int main(int argc, char* argv[]) {


	if (argc != 2)
	{
		error("Usage is bittorren <file.torrent>");
		return EXIT_FAILURE;
	}
	
	
	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return EXIT_FAILURE;
	}

	elem e;
	e.build(is);

	e.print_rec();

	std::cout << "ciao";



}