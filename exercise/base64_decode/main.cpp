#include "base64.h"
#include <iostream>

int main(int argc, char** argv) {

	auto res = base64_decode("MQ==");

	std::cout << res;
	return 0;
}