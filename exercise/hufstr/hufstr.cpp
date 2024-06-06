#include "hufstr.h"
#include <cstdint>
#include <fstream>
#include <unordered_map>

struct bitreader
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

    bitreader(std::ifstream& is) : is_(is) {}

    uint64_t operator()(int numbits) {
        uint64_t val = 0;
        for (int i = numbits - 1; i >= 0; i--)
        {
            val <<= 1;
            val |= readbit();
        }
        return val;
    }

    auto ended() {
        return (!is_);
    }
};

hufstr::hufstr() {
    std::ifstream is("table.bin", std::ios::binary);
    if (!is)
    {
        throw 1;
    }

    bitreader br(is);
    for (size_t i = 0; i < 256; i++)
    {
        uint8_t sym = 0, len = 0;
        sym = br(8);
        len = br(5);
        uint32_t code = br(len);
        compress_map[sym] = { len, code };
        decompress_map[code] = { len, sym };
    }
   /* for (size_t i = 0; i < 256; i++)
    {
        ++freq[i];
    }
    for (size_t i = 0; i < 7; i++)
    {
        ++freq['a'];
        ++freq['e'];
        ++freq['i'];
        ++freq['o'];
    }
    for (size_t i = 0; i < 3; i++)
    {
        ++freq['c'];
        ++freq['f'];
        ++freq['l'];
        ++freq['m'];
    }*/
    
}

std::vector<uint8_t> hufstr::compress(const std::string& s) const {
    std::ofstream os("tmpout.bin", std::ios::binary);
    auto localmap = compress_map;
    bitwriter bw(os);
    for (const auto& elem : s) {
        std::pair<uint8_t, uint32_t> entry = localmap[elem];
        bw(entry.second, entry.first);
    }
    bw.flush();
    os.close();

    std::ifstream is("tmpout.bin", std::ios::binary);
    if (!is)
    {
        return std::vector<uint8_t>{};
    }
    is.seekg(0, std::ios::end);
    auto size = is.tellg();
    is.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);

    is.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

std::string hufstr::decompress(const std::vector<uint8_t>& v) const {
    std::ofstream os("tmpout_dec.bin", std::ios::binary); 
    for (const auto& elem : v) {
        os.put(elem);
    }
    os.close();

    uint32_t buffer = 0;
    std::ifstream is("tmpout_dec.bin", std::ios::binary);
    bitreader br(is);
    auto localmap = decompress_map;
    uint8_t len = 0;
    uint32_t totlen = 0;
    std::string output;
    while ((totlen < (v.size() * 8)))
    {
        buffer <<= 1;
        buffer |= br(1);
        ++len;
        ++totlen;
        if (decompress_map.find(buffer) != decompress_map.end())
        {
            if (localmap[buffer].first == len)
            {
                output.push_back(localmap[buffer].second);
                len = 0;
                buffer = 0;
            }
        }
    }
    return output;

    }