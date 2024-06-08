#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>
#include "pcx.h"
#include "mat.h"

int main(int argc, char** argv) {

	mat<uint8_t> img(1,1);

	load_pcx("hose.pcx", img);

}