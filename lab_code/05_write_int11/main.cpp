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
		"write_int11 <filein.txt> <fileout.bin>\n"
		"The first parameter is the name of a text file that contains base 10 integers from -1000 to 1000 separated\n"
		"by whitespace. The program must create a new file, with the name passed as the second parameter, with\n"
		"the same numbers saved as 11-bit binary in 2's complement. The bits are inserted in the file from the\n"
		"most significant to the least significant. The last byte of the file, if incomplete, is filled with bits of 0.\n");
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

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_;
	size_t nbits_;

	std::ostream& write_bit(uint32_t u) {
		// buffer_ = buffer_ * 2 + u % 2;
		buffer_ = (buffer_ << 1) | (u & 1);
		++nbits_;
		if (nbits_ == 8) {
			raw_write(os_, buffer_);
			nbits_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os), nbits_(0) {}

	~bitwriter() {
		flush();
	}

	// 43210
	// 00111

	// Writes the n least significant bits of u, from the most significant to the least significant,
	// i.e. from bit #n-1 to bit #0
	std::ostream& write(uint32_t u, size_t n) {
		// Solution 1: for (size_t i = n - 1; i < n; --i) { 
		// Solution 2: for (size_t i = n; i > 0;) { 
		//	               --i;
		// Solution 3: while (n > 0) {
		// 	               n--;
		// Solution 4: while (n --> 0) {
        //                 write_bit(u >> n); // writes bit #n
		//             }
		for (size_t i = 0; i < n; ++i) {
			write_bit(u >> (n - 1 - i));
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, size_t n) {
		return write(u, n);
	}

	void flush(uint32_t u = 0) {
		while (nbits_ > 0) {
			write_bit(u);
		}
	}
};

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
	bitwriter bw(os);

	int32_t num;
	while (is >> num) {
		bw(num, 11);
	}

	return EXIT_SUCCESS;
}