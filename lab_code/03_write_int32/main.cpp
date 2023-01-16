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
		"write_int32 <filein.txt> <fileout.bin>\n"
		"The first parameter is the name of a text file that contains signed\n"
		"base 10 integers from -1000 to 1000\n"
		"separated by whitespace. The program must create a new file,\n"
		"with the name passed as the second parameter, with the same\n"
		"numbers saved as 32-bit binary little-endian numbers in 2's complement.");
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

int main(int argc, char *argv[])
{
	using namespace std;

	if (argc != 3) {
		syntax();
	}

	std::ifstream is(argv[1]);
	if (!is) {
		error("Cannot open input file\n");
	}
	ofstream os(argv[2], ios::binary);
	if (!os) {
		error("Cannot open output file.\n");
	}

	int32_t num;
	while (is >> num) {
		raw_write(os, num);
	}

	return EXIT_SUCCESS;
}