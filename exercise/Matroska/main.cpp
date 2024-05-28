#include <cstdint>
#include <vector>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>

void error(const char* msg){
	std::cerr << msg << std::endl;
}

struct reader
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

	reader(std::ifstream& is) : is_(is) {}

	uint64_t operator()(int nbits) {
		uint64_t buf = 0;
		for (int i = nbits - 1; i >= 0; i--)
		{
			buf <<= 1;
			buf |= readbit();
		}
		return buf;
	}
};

struct element
{
	uint32_t id_;
	uint64_t size_;
	uint32_t uint;
	int i;
	float f;
	std::string str;
	std::string nome;
	std::vector<element> components;
};

auto get_id_len(uint8_t first) {
	if ((first >> 7) == 1)
	{
		return 1;
	}
	if ((first >> 6) == 1)
	{
		return 2;
	}
	if ((first >> 5) == 1)
	{
		return 3;
	}
	if ((first >> 4) == 1)
	{
		return 4;
	}
}

static const std::map<uint32_t, std::string> masterElements{
		{0x1a45dfa3, "EBML"},
		{0x18538067, "Segment"},
		{0x114d9b74, "SeekHead"},
		{0x1549a966, "Info"},
		{0x6924, "ChapterTranslate"},
		{0x1f43b675, "Cluster"},
		{0xa0, "BlockGroup"},
		{0x75a1, "BlockAdditions"},
		{0xaf, "ReferenceFrame"},
		{0x1654ae6b, "Tracks"},
		{0x1c53bb6b, "Cues"},
		{0x1941a469, "Attachments"},
		{0x1043a770, "Chapters"},
		{0x1254c367, "Tags"},
		{0x4DBB, "SeekEntry" },
		{0xA6, "BlockMore"},
		{0xAE, "TrackEntry"},
		{0xE0, "Video"},
		{0x55B0, "Colour"},
		{0xE1, "Audio"},
		{0x7373, "Tag"},
		{0x63C0, "Targets"},
		{0x67C8, "SimpleTag"},
		{0xBB, "CuePoint"},
		{0xB7, "CueTrackPositions"},
		{0xDB, "CueReference"},






};

bool isMasterElement(uint32_t id) {
	return masterElements.find(id) != masterElements.end();
}

static const std::map<uint32_t, std::string> stringElements{
	{0x4282, "DocType"},
	{0x4D80, "MuxingApp"},
	{0x4D82, "WritingApp"},
	{0x47E7, "ChapterStringUID"},
	{0x466E, "FileName"},
	{0x4486, "Name"},
	{0x4487, "Language"},
	{0x445A, "LanguageIETF"},
	{0x444D, "ChapterLanguageIETF"},
	{0x437E, "ClusterCodecName"},
	{ 0x7384, "SegmentFilename" },
	{0x3C83AB, "PrevFilename"},
	{0x3E83BB, "NextFilename"},
	{0x7BA9, "Title"},
	{0x5741, "WritingApp"},
	{0x22B59C, "Language"},
	{0x86, "CodecID"},
	{0x45A3, "TagName"},
	{0x447A, "TagLanguage"},


};
bool isString(uint32_t id) {
	return stringElements.find(id) != stringElements.end();
}

static const std::map<uint32_t, std::string> uintElements{
{0x4286, "EBMLVersion"},
{0x42F7, "EBMLReadVersion"},
{0x42F2, "EBMLMaxIDLength"},
{0x42F3, "EBMLMaxSizeLength"},
{0x4287, "DocTypeVersion"},
{0x4285, "DocTypeReadVersion"},
{0x2AD7B1, "TimestampScale"},
{0x4484, "TagDefault"},
{0x4DA3, "NextUID"},
{0x4EBF, "MaxCache"},
{0x4E61, "MinCache"},
{0xB3, "CueTime"},
{0x9F, "Channels"},
{0xF7, "CueTrack"},
{0xF1, "CueClusterPosition"},
{0xEA, "CueCodecState"},
{0x96, "CueRefTime"},
{0xF0, "CueRelativePosition"},
{0x68CA, "TargetTypeValue"},
{0x23E383, "DefaultDuration"},
{0x4DA0, "PrevFilename"},
{0x53B3, "TrackTimecodeScale"},
{0x55AE, "CodecDelay"},
{0x56BF, "EffectiveDuration"},
{0x5854, "SilentTrackNumber"},
{0x61A7, "Position"},
{0x6264, "CueClusterPosition"},
{0x63C5, "CueTrackPositions"},
{0x63A2, "CueTime"},
{0x55B1, "MatrixCoefficients"},
{0x55B2, "BitsPerChannel"},
{0x55B7, "ChromaSitingHorz"},
{0x55B8, "ChromaSitingVert"},
{0x55B9, "Range"},
{0x55BA, "TransferCharacteristics"},
{0x55BB, "Primaries"},
{0x6DE7, "ReferenceBlock"},
{0x77A5, "ChapterTranslateID"},
{0x7A84, "SegmentFilename"},
{0x7DBB, "SeekID"},
{0x7E7B, "CueRelativePosition"},
{0x7E84, "TrackTranslateTrackID"},
{0x53AC, "SeekPosition"},
{0x69BF, "ChapterTranslateCodec"},
{0x69FC, "ChapterTranslateEditionUID"},
{0xE7, "Timestamp"},
{0xA7, "Position" },
{0xAB, "PrevSize"},
{0xEE, "BlockAddID"},
{0x9B, "BlockDuration"},
{0xFA, "ReferencePriority"},
{0xD7, "TrackNumber"},
{0x73C5, "TrackUID"},
{0x83, "TrackType"},
{0xB9, "FlagEnabled	"},
{0x88, "FlagDefault"},
{0x55AA, "FlagForced"},
{0x9C, "FlagLacing"},
{0x55EE, "MaxBlockAdditionID"},
{0x41E7, "BlockAddIDType"},
{0x56AA, "CodecDelay"},
{0x56BB, "SeekPreRoll"},
{0x9A, "FlagInterlaced"},
{0x9D, "FieldOrder"},
{0x53B8, "StereoMode"},
{0x53C0, "AlphaMode"},
{0xB0, "PixelWidth"},
{0xBA, "PixelHeight"},
};
bool isUint(uint32_t id) {
	return uintElements.find(id) != uintElements.end();
}

static const std::map<uint32_t, std::string> intElements{
	{0x4DA3, "NextUID"},
	{0x55AE, "CodecDelay"},
	{0x56BF, "EffectiveDuration"},
	{0x5854, "SilentTrackNumber"},
	{0xFB, "ReferenceBlock"}
};
bool isInt(uint32_t id) {
	return intElements.find(id) != intElements.end();
}

static const std::map<uint32_t, std::string> binElements{
		{0x53AB, "SeekID"},
		{0x73A4, "SegmentUUID"},
		{0x3CB923, "PreviusUUID"},
		{0x3EB923, "NextUUID"},
		{0x4444, "SegmentFamily"},
		{0x69A5, "ChapterTranslateID"},
		{0xA3, "SimpleBlock"},
		{0xA1, "Block"},
		{0xA5, "BlockAdditional"},
		{0xBF, "CRC32"},
		{0xEC, "EBML Void"},
};
bool isBin(uint32_t id) {
	return binElements.find(id) != binElements.end();
}

static const std::map<uint32_t, std::string> floatElements{
		{0x4489, "Duration"},
		{0xB5, "SamplingFrequency"},

};
bool isFloat(uint32_t id) {
	return floatElements.find(id) != floatElements.end();
}




uint64_t read_rec(std::ifstream& is) {

	std::map<int, std::string> table(masterElements.begin(), masterElements.end());
	table.insert(stringElements.begin(), stringElements.end());
	table.insert(intElements.begin(), intElements.end());
	table.insert(uintElements.begin(), uintElements.end());
	table.insert(floatElements.begin(), floatElements.end());
	table.insert(binElements.begin(), binElements.end());

	reader br(is);
	while (is.peek() != EOF)
	{
		uint8_t firstByte = is.get();
		uint8_t id_len = get_id_len(firstByte);
		uint32_t id = firstByte;
		id <<= (id_len - 1) * 8;
		if (id_len != 1)
		{
			uint32_t val = 0;
			val = br((id_len - 1) * 8);
			id |= val;
		}


		firstByte = is.get();
		uint8_t size_len = std::countl_zero(firstByte) + 1;
		uint64_t size = firstByte & (int)(std::pow(2, (8 - size_len)) - 1);
		size <<= 8 * (size_len - 1);
		size |= br((size_len - 1) * 8);
		size += id_len + 1;
		//size += size_len - 1;
		
		//std::cout << "DEBUG:" << is.peek() << std::endl;

		element e;
		e.id_ = id;
		e.size_ = size + size_len - 1;

		if (isMasterElement(e.id_))
		{
			e.nome = table[e.id_];
			std::cout << e.nome << " dim: " << e.size_ << std::endl;
			uint64_t cur_size = 0;
			while (size > cur_size + id_len + 1)
			{
				//element ee = read_rec(is);
				uint64_t size_ = read_rec(is);
				cur_size += size_;
				if (is.peek() == EOF)
				{
					break;
				}
				//e.components.push_back(ee);
			}
		}
		else
		{
			if (isString(e.id_))
			{
				e.str.resize(e.size_ - id_len - size_len);
				is.read(reinterpret_cast<char*>(e.str.data()), e.size_ - id_len - size_len);
				e.nome = table[e.id_];
				std::cout << e.nome << " dim: " << e.size_ << std::endl;
			}
			else if(isUint(e.id_))
			{
				uint32_t v = br((e.size_ - id_len - size_len) * 8);
				e.uint = v;
				e.nome = table[e.id_];
				std::cout << e.nome << " dim: " << e.size_ << std::endl;
			}
			else if (isInt(e.id_))
			{
				int v = br((e.size_ - id_len - size_len) * 8);
				e.i = v;
				e.nome = table[e.id_];
				std::cout << e.nome << " dim: " << e.size_ << std::endl;
			}
			else if (isFloat(e.id_))
			{
				float ff = br((e.size_ - id_len - size_len) * 8);
				e.f = ff;
				e.nome = table[e.id_];
				std::cout << e.nome << " dim: " << e.size_ << std::endl;
			}
			else
			{
				br((e.size_ - id_len - size_len) * 8);
				e.nome = table[e.id_];
				if (e.nome == "SimpleBlock")
				{
					std::cout << "SimpleBlock di dimensione: " << e.size_ << " (" << e.nome << ')' << " id: " << e.id_ << std::endl;
				}
				else
				{
					std::cout << "roba binaria di dimensione: " << size - id_len - 1 << " (" << e.nome << ')' <<" id: " << e.id_ << std::endl;
				}
				
			}
			return e.size_;
		}
	}
}

auto decode(const char* input) {
	std::ifstream is(input, std::ios::binary);
	if (!is)
	{
		error("Error while opening input file");
		return EXIT_FAILURE;
	}

	read_rec(is);

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
	
	decode(argv[1]);

	return EXIT_SUCCESS;
}