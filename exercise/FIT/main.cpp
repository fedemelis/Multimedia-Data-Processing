#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <tuple>
#include <iomanip>
#include <cmath>

using namespace std;

void FitCRC_Get16(uint16_t& crc, uint8_t byte)
{
	//std::cout << +byte << std::endl;
	static const uint16_t crc_table[16] =
	{
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
	};
	uint16_t tmp;
	// compute checksum of lower four bits of byte
	//std::cout << +byte << std::endl;
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[byte & 0xF];
	// now compute checksum of upper four bits of byte
	tmp = crc_table[crc & 0xF];
	crc = (crc >> 4) & 0x0FFF;
	crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
}

auto readHead(std::ifstream& is) {
	
	uint8_t headerSize = 0, proto_v = 0;
	uint16_t profile_v = 0, CRC_head = 0;
	uint32_t dataSize = 0;
	std::string type_string(4, ' ');

	uint16_t actual_CRC = 0;

	headerSize = is.get();
	FitCRC_Get16(actual_CRC, headerSize);

	proto_v = is.get();
	FitCRC_Get16(actual_CRC, proto_v);

	is.read(reinterpret_cast<char*>(&profile_v), 2);
	FitCRC_Get16(actual_CRC, (profile_v & 0x000000FF));
	FitCRC_Get16(actual_CRC, (profile_v >> 8) & 0xFF);

	is.read(reinterpret_cast<char*>(&dataSize), 4);
	FitCRC_Get16(actual_CRC, static_cast<uint8_t>(dataSize & 0x000000FF));
	FitCRC_Get16(actual_CRC, static_cast<uint8_t>((dataSize >> 8) & 0x000000FF));
	FitCRC_Get16(actual_CRC, static_cast<uint8_t>((dataSize >> 16) & 0x000000FF));
	FitCRC_Get16(actual_CRC, static_cast<uint8_t>((dataSize >> 24) & 0x000000FF));

	is.read(reinterpret_cast<char*>(type_string.data()), 4);
	FitCRC_Get16(actual_CRC, type_string.data()[0]);
	FitCRC_Get16(actual_CRC, type_string.data()[1]);
	FitCRC_Get16(actual_CRC, type_string.data()[2]);
	FitCRC_Get16(actual_CRC, type_string.data()[3]);

	is.read(reinterpret_cast<char*>(&CRC_head), 2);

	if (CRC_head == actual_CRC)
	{
		std::cout << std::endl << "Header CRC ok" << std::endl;
	}
	else
	{
		return (uint32_t)1;
	}

	return dataSize;

}

auto readBody(std::ifstream& is, uint32_t dataSize) {

	std::map<uint8_t, std::vector<std::tuple<uint8_t, uint8_t, uint16_t>>> definition_map;
	bool first = true;

	while (is.tellg() < (dataSize + 14))
	{
		uint8_t infobyte = is.get();
		if ((infobyte >> 4) == 4)
		{
			uint8_t local_message_type = infobyte & 0b1111;
			is.ignore(2); // skipping reserved + architecture + global message number
			uint16_t glob = 0;
			is.read(reinterpret_cast<char*>(&glob), 2);
			uint8_t num_field = is.get();
			std::vector<std::tuple<uint8_t, uint8_t, uint16_t>> entry;
			for (size_t i = 0; i < num_field; i++)
			{
				uint8_t number = is.get();
				uint8_t size = is.get();
				is.ignore(1); // skipping base
				entry.emplace_back(number, size, glob);
			}
			definition_map[local_message_type] = entry;
			
			//definition message
		}
		else if ((infobyte >> 4) == 0) 
		{
			uint8_t local_message_type = infobyte & 0b1111;
			for (const auto& [numb, size, glob] : definition_map[local_message_type]) {
				uint64_t buffer = 0;
				if ((numb == 4) && first)
				{
					first = false;
					uint32_t buffer = 0;
					is.read(reinterpret_cast<char*>(&buffer), size);
					std::cout << "time_created = " << +buffer << std::endl;
				}
				else if ((glob == 19) && (numb == 13))
				{
					uint16_t buffer = 0;
					is.read(reinterpret_cast<char*>(&buffer), size);
					double kmps = ((double)buffer * 3600) / std::pow(10, 6);
					std::cout << "avg_speed = " <<  std::fixed << std::setprecision(3) << kmps << std::endl;
				}
				else
				{
					is.ignore(size);
				}
			}
		}
		else
		{
			return 1;
		}
	}
	uint16_t CRC_body = 0;
	is.read(reinterpret_cast<char*>(&CRC_body), 2);
	is.seekg(14, std::ios::beg);
	uint16_t crc = 0;
	for (size_t i = 0; i < dataSize; i++)
	{
		FitCRC_Get16(crc, is.get());
	}
	if (crc == CRC_body)
	{
		std::cout << "File CRC ok" << std::endl;
	}
	else
	{
		return 1;
	}

	return 0;
}

auto readfit(const char* inputfile) {

	std::ifstream is(inputfile, std::ios::binary);
	if (!is)
	{
		return 1;
	}

	uint32_t dataSize = readHead(is);
	if (dataSize == 1)
	{
		return 1;
	}

	return readBody(is, dataSize);
	
}

int main(int argc, char **argv){

	if (argc != 2)
	{
		return 1;
	}

	return readfit(argv[1]);

}