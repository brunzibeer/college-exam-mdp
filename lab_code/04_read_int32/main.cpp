#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>
#include <iterator>
#include <cstdint>

void error(const char *msg)
{
	std::cout << msg;
	exit(EXIT_FAILURE);
}

void syntax() {
	error("SYNTAX:\n"
		"read_int32 <filein.bin> <fileout.txt>\n"
		"The first parameter is the name of a binary file containing 32-bit numbers 2’s complement, in little\n"
		"endian. The program must create a new file, with the name passed as the second parameter, with the same\n"
		"numbers saved in decimal text format separated by a new line.\n");
}

struct frequency_counter {
	std::array<size_t, 256> occurrencies;
	
	frequency_counter() : occurrencies{ 0 } {}

	void operator()(uint8_t val) {
		++occurrencies[val];
	}

	const size_t& operator[](size_t pos) const {
		return occurrencies[pos];
	}
	size_t& operator[](size_t pos) {
		return occurrencies[pos];
	}

	double entropy() {
		double tot = 0.0;
		for (const auto& x : occurrencies) {
			tot += x;
		}

		double H = 0.0;
		for (const auto& x : occurrencies) {
			if (x > 0) {
				double px = x / tot;
				H += px * log2(px);
			}
		}

		return -H;
	}
};

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&num), size);
}
template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T))
{
	return is.read(reinterpret_cast<char*>(&num), size);
}

int main(int argc, char *argv[])
{
	using namespace std;

	if (argc != 3) {
		syntax();
	}

	std::ifstream is(argv[1], ios::binary);
	if (!is) {
		error("Cannot open input file\n");
	}
	ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file.\n");
	}

	int32_t num;
	while (raw_read(is, num)) {
		os << num << '\n';
	}

	return EXIT_SUCCESS;
}