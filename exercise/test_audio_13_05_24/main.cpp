#include <vector>
#include <cstdint>
#include <cmath>
#include <fstream>


int main(void) {
	

	// y = A*sin(2*pi*f*t)

	int duration = 3000; //ms
	int A = 32767;
	int f = 440; //Hz
	const double pi = 3.1415926535;
	const int sample_freq = 44100;

	std::vector<int16_t> samples(sample_freq * (duration / 1000));

	for (int i = 0; i < samples.size(); i++)
	{
		double t = (double)i / sample_freq;
		double x = A * sin(2 * pi * f * t);
		int16_t s = static_cast<int16_t>(round(x));
		samples[i] = s;
	}

	std::ofstream os("audio.raw", std::ios::binary);
	if (!os)
	{
		return EXIT_FAILURE;
	}

	os.write(reinterpret_cast<char*>(samples.data()), samples.size()  * 2);

	os.close();


	// caricare un file audio

	std::ifstream is("audio.raw", std::ios::binary);
	if (!is)
	{
		return EXIT_FAILURE;
	}

	is.seekg(0, std::ios::end);
	auto size = is.tellg();
	is.seekg(0, std::ios::beg);

	std::vector<int16_t> audio(size/2);

	is.read(reinterpret_cast<char*>(audio.data()), size);

	for (auto& x : audio) {
		x /= 2;
	}



	return EXIT_SUCCESS;

}