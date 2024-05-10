#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <array>

void error(const char* errmsg) {
	std::cerr << errmsg << '\n';
}

using vec3b = std::array<uint8_t, 3>;

template <typename T>
struct mat
{
	size_t r_;
	size_t c_;
	std::vector<T> data_;

	mat(size_t r, size_t c) : r_(r), c_(c), data_(r* c) {}

	T& operator()(size_t row, size_t col) {
		return data_[row * c_ + col];
	}
	const T& operator()(size_t row, size_t col) const {
		return data_[row * c_ + col];
	}

	size_t cols() {
		return c_;
	}

	size_t rows() {
		return r_;
	}

	size_t dim() {
		return data_.size() * sizeof(T);
	}

	auto rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
};

struct head
{
	size_t width = 0;
	size_t height = 0;
};

struct box
{
	std::vector<vec3b> boxes;

	box() {}

	box(std::vector<vec3b> b) : boxes(b) {}

	auto begin() {
		return boxes.begin();
	}

	auto end() {
		return boxes.end();
	}

	bool operator==(const box& rhs) {
		bool eq = true;
		for (size_t i = 0; i < rhs.boxes.size(); i++)
		{
			if (rhs.boxes[i] != boxes[i])
			{
				eq = false;
				break;
			}
		}
		return eq;
	}

};


int read_head(head& h, std::ifstream& is) {
	std::string magicnumber(2, ' ');
	is >> magicnumber;
	if (magicnumber != "P6")
	{
		error("Magicnumber should be P6");
		return EXIT_FAILURE;
	}
	is >> h.width;
	is >> h.height;

	std::string trash(5, ' ');
	is >> trash;
	is.get(); // remove newline

	return EXIT_SUCCESS;
}

int loadppm(mat<vec3b>& img, std::ifstream& is) {
	mat<vec3b> display(img.rows(), img.cols());
	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			img(r, c)[0] = is.get();
			img(r, c)[1] = is.get();
			img(r, c)[2] = is.get();

			display(r, c)[0] = img(r, c)[2];
			display(r, c)[1] = img(r, c)[1];
			display(r, c)[2] = img(r, c)[0];

		}
	}

	return EXIT_SUCCESS;
}

auto compute_range(box& bb) {
	
	std::vector<uint8_t> r, g, b;

	for (auto& elem : bb) {
		r.push_back(elem[0]);
		g.push_back(elem[1]);
		b.push_back(elem[2]);
	}

	auto max_r = std::max_element(r.begin(), r.end());
	auto min_r = std::min_element(r.begin(), r.end());

	auto max_g = std::max_element(g.begin(), g.end());
	auto min_g = std::min_element(g.begin(), g.end());

	auto max_b = std::max_element(b.begin(), b.end());
	auto min_b = std::min_element(b.begin(), b.end());

	uint8_t diff_r = *max_r - *min_r;

	uint8_t diff_g = *max_g - *min_g;

	uint8_t diff_b = *max_b - *min_b;

	if ((diff_r > diff_g) && (diff_r > diff_b))
	{
		std::pair<int, uint8_t> pp ({0, diff_r});
		return pp;
	}
	else if ((diff_g >= diff_r) && (diff_g >= diff_b))
	{
		std::pair<int, uint8_t> pp({1, diff_g });
		return pp;
	}
	else if ((diff_b > diff_r) && (diff_b > diff_g))
	{
		std::pair<int, uint8_t> pp({2, diff_b });
		return pp;
	}
}

std::pair<box, box> split_on_median(int channel, const box& b) {
	
	auto half = b.boxes.size() / 2;

	box copy = b;

	std::nth_element(copy.begin(), (copy.begin() + half + 1), copy.end(), [channel](vec3b& lhs, vec3b& rhs) {return lhs[channel] > rhs[channel]; });

	auto half_it = std::next(copy.begin() + half);

	box left(std::vector<vec3b>(copy.begin(), half_it));
	box right(std::vector<vec3b>(half_it, copy.end()));

	return { left, right };
}

auto compute_color(std::vector<box> boxlist) {
	std::vector<vec3b> palette;
	int tot_r = 0, tot_g = 0, tot_b = 0;
	for (auto& elem : boxlist) {
		for (const auto& pix : elem) {
			tot_r += pix[0];
			tot_g += pix[1];
			tot_b += pix[2];
		}

		uint8_t r = tot_r / elem.boxes.size();
		uint8_t g = tot_g / elem.boxes.size();
		uint8_t b = tot_b / elem.boxes.size();
		tot_r = 0, tot_g = 0, tot_b = 0;

		palette.push_back({ r, g, b });
	}

	return palette;
}

std::vector<vec3b> split_space_in_box(mat<vec3b>& img, size_t N) {

	std::vector<box> boxlist({ { img.data_ } });
	uint8_t maxdiff = 0;
	int channel = 0;
	box winbox;
	while (boxlist.size() < N)
	{
		for (auto& elem : boxlist) {
			auto [ch, diff] =  compute_range(elem);
			if (diff >= maxdiff)
			{
				maxdiff = diff;
				channel = ch;
				winbox = elem;
			}
		}	
		auto [left, right] = split_on_median(channel, winbox);
		//box bbb = winbox;
		auto it = std::find(boxlist.begin(), boxlist.end(), winbox);
		if (it != boxlist.end())
		{
			boxlist.erase(it);
		}

		boxlist.push_back(left);
		boxlist.push_back(right);

		maxdiff = 0;
	}

	return compute_color(boxlist);
}

auto euclidean_distance(const vec3b lhs, const vec3b rhs) {
	uint8_t l1 = 0, l2 = 0, l3 = 0, r1 = 0, r2 = 0, r3 = 0;

	l1 = lhs[0];
	l2 = lhs[1];
	l3 = lhs[2];
	r1 = rhs[0];
	r2 = rhs[1];
	r3 = rhs[2];

	auto d1 = std::abs(l1 - r1);
	auto d2 = std::abs(l2 - r2);
	auto d3 = std::abs(l3 - r3);

	return d1 + d2 + d3;


}

auto apply_palette(std::vector<vec3b>& palette, mat<vec3b>& img) {

	for (size_t r = 0; r < img.rows(); r++)
	{
		for (size_t c = 0; c < img.cols(); c++)
		{
			std::vector<double> distances;
			for (size_t i = 0; i < palette.size(); i++)
			{
				distances.push_back(euclidean_distance(palette[i], img(r, c)));
			}
			auto it = std::min_element(distances.begin(), distances.end());
			if (it != distances.end())
			{
				auto dst = std::distance(distances.begin(), it);
				img(r, c) = palette[dst];
			}
			
		}
	}

}


auto save_rgb(std::ofstream& os, mat<vec3b>& rgb_img) {
	os << "P6" << '\n';
	os << rgb_img.cols() << '\n';
	os << rgb_img.rows() << '\n';
	os << "255" << '\n';

	os.write(rgb_img.rawdata(), rgb_img.dim());

	return EXIT_SUCCESS;
}





int main(int argc, char* argv[]) {
	
	if (argc != 2)
	{
		error("Usage is: median cut <file.ppm>");
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		error("Error while opening input ppm file");
		return EXIT_FAILURE;
	}

	head h;
	read_head(h, is);

	mat<vec3b> img(h.height, h.width);

	loadppm(img, is);

	//std::vector<vec3b> palette;

	size_t N = 16;
	//split_space_in_box(img, N);

	std::vector<vec3b> palette = split_space_in_box(img, N);

	/*mat<vec3b> display(2, 8);
	int index = 0;
	for (size_t r = 0;  r < display.rows();  r++)
	{
		for (size_t c = 0; c < display.cols(); c++)
		{
			display(r, c)[0] = palette[index][2];
			display(r, c)[1] = palette[index][1];
			display(r, c)[2] = palette[index][0];
			++index;
		}
	}*/

	mat<vec3b> display(img.rows(), img.cols());
	apply_palette(palette, img);
	
	//int index = 0;
	for (size_t r = 0; r < display.rows(); r++)
	{
		for (size_t c = 0; c < display.cols(); c++)
		{
			display(r, c)[0] = img(r, c)[2];
			display(r, c)[1] = img(r, c)[1];
			display(r, c)[2] = img(r, c)[0];
			//++index;
		}
	}

	std::ofstream os("zanettiout.ppm", std::ios::binary);
	if(!os)
	{
		return EXIT_FAILURE;
	}

	save_rgb(os, img);

	std::cout << "Ciao";
}
