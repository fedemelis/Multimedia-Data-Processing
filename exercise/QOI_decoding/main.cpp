#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>

int error(const char* errmsg) {
    std::cerr << errmsg << std::endl;
    return EXIT_FAILURE;
}

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
};

template<typename T>
struct mat {
    size_t r_, c_;
    std::vector<T> data_;

    mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

    T& operator()(size_t row, size_t col) {
        return data_[row * c_ + col];
    }

    const T& operator()(size_t row, size_t col) const {
        return data_[row * c_ + col];
    }

    size_t rows() const { return r_; }
    size_t cols() const { return c_; }

    auto rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t dim() const { return data_.size() * sizeof(T); }
};

struct head
{
    std::string magicnumber = "    ";
    uint32_t width, height;
    uint8_t channel, colorspace;

};

struct seen_color {

    std::array<std::array<uint8_t, 4>, 64> table = { {0} };

    std::array<uint8_t, 4> prev = { 0, 0, 0, 255 };

    void operator()(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        int index = ((r * 3) + (g * 5) + (b * 7) + (a * 11)) % 64;
        table[index] = { r, g, b, a };
    }

    void updPrev_a(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        prev[0] = r;
        prev[1] = g;
        prev[2] = b;
        prev[3] = a;
    }

    void updPrev(uint8_t r, uint8_t g, uint8_t b) {
        prev[0] = r;
        prev[1] = g;
        prev[2] = b;
    }
};

auto readHead(std::ifstream& is, head& h) {

    bitreader br(is);

    is.read(reinterpret_cast<char*>(h.magicnumber.data()), 4);
    if (h.magicnumber != "qoif")
    {
        return error("Error: magicnumber should be \"qoif\"");
    }
    
    h.width = br(32);
    h.height = br(32);

    h.channel = is.get();
    h.colorspace = is.get();
    return EXIT_SUCCESS;
}

auto readQOI(std::ifstream& is, mat<std::array<uint8_t, 4>>& img, seen_color& color) {
    
    is.seekg(0, std::ios::end);
    auto size = is.tellg();
    is.seekg(14, std::ios::beg);

    bitreader br(is);
    size -= 8;
    int pos = 13;
    std::vector<std::array<uint8_t, 4>>pixels;

    while (pos < size)
    {
        auto peek = is.peek();
        if ((peek == 0b11111110) || (peek == 0b11111111))
        {
            if (peek == 0b11111110)
            {
                is.ignore(1);
                uint8_t r, g, b;
                uint8_t a = 255;

                r = is.get();
                g = is.get();
                b = is.get();

                color.updPrev(r, g, b);
                color(r, g, b, a);
                
                std::array<uint8_t, 4> pix = { r, g, b, a };
                pixels.push_back(pix);
                pos += 4;
            }
            else
            {
                is.ignore(1);
                uint8_t r, g, b, a;
                r = is.get();
                g = is.get();
                b = is.get();
                a = is.get();

                color.updPrev_a(r, g, b, a);
                color(r, g, b, a);

                std::array<uint8_t, 4> pix = { r, g, b, a };
                pixels.push_back(pix);
                pos += 5;
            }
        }
        else
        {
            uint8_t code = br(2);
            if (code == 0)
            {
                uint8_t index = br(6);
                auto& [r, g, b, a] = color.table[index];
                
                std::array<uint8_t, 4> pix = { r, g, b, a };
                pixels.push_back(pix);

                color.updPrev_a(r, g, b, a);
                
                ++pos;

            }
            else if (code == 1) {
                uint8_t dr, dg, db;
                
                dr = br(2);
                dg = br(2);
                db = br(2);

                uint8_t r, g, b;
                uint8_t a = color.prev[3];

                r = dr - 2 + color.prev[0]; // -2 bias
                g = dg - 2 + color.prev[1];
                b = db - 2 + color.prev[2];

                color.updPrev(r, g, b);
                color(r, g, b, a);

                std::array<uint8_t, 4> pix = { r, g, b, a };
                pixels.push_back(pix);

                ++pos;

            }
            else if (code == 2) {
                uint8_t dr_dg, dg, db_dg;

                dg = br(6);
                dr_dg = br(4);
                db_dg = br(4);

                uint8_t r, g, b;

                auto& [pr, pg, pb, pa] = color.prev;

                g = dg + color.prev[1] - 32; // -32 bias

                r = (g - pg) + dr_dg + pr - 8; // -8 bias

                b = (g - pg) + db_dg + pb - 8; // -8 bias

                color.updPrev(r, g, b);
                color(r, g, b, pa);

                std::array<uint8_t, 4> pix = { r, g, b, pa };
                pixels.push_back(pix);

                pos += 2;

            }
            else
            {
                uint8_t runLen = br(6);
                for (size_t i = 0; i < runLen + 1; i++)
                {
                    pixels.push_back(color.prev);
                }
                ++pos;
            }
        }

    }
    //std::cout << "ciao";
    for (size_t r = 0; r < img.rows(); r++)
    {
        for (size_t c = 0; c < img.cols(); c++)
        {
            img(r, c) = pixels[r * img.cols() + c];
        }
    }
    return EXIT_SUCCESS;
    //std::cout << "Ciao";

}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        return error("Usage is qoi_decomp <input file> <output file>");
    }

    std::ifstream is(argv[1], std::ios::binary);
    if (!is)
    {
        return error("Error while opening input file for decoding");
    }

    head h;
    readHead(is, h);

    mat<std::array<uint8_t, 4>> img(h.height, h.width);
    seen_color color;

    readQOI(is, img, color);

    // Questo è il formato di output PAM. È già pronto così, ma potete modificarlo se volete
    std::ofstream os(argv[2], std::ios::binary); // Questo utilizza il secondo parametro della linea di comando!
    os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() <<
        "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.dim());

    return EXIT_SUCCESS;
}