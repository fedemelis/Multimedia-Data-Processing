#include "pgm16.h"

int main(int argc, char** argv) {

	mat<uint16_t> img;
	std::string filename("frog_bin.pgm");
	uint16_t maxvalue = 0;

	load(filename, img, maxvalue);

	return 0;

}