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

class bitreader {
	std::istream& is_;
	uint8_t buffer_;
	size_t nbits_;

	uint32_t read_bit() {
		if (nbits_ == 0) {
			raw_read(is_, buffer_);
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is), nbits_(0) {}

	std::istream& read(uint32_t& u, size_t n) {
		u = 0;
		while (n --> 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

int main(int argc, char *argv[])
{
	using namespace std;

	if (argc != 3) {
		error("Ciao");
	}

	std::ifstream is(argv[1], ios::binary);
	if (!is) {
		error("Cannot open input file\n");
	}
	bitreader br(is);

	ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file.\n");
	}

	uint32_t unum;
	while (br.read(unum, 11)) {
		int32_t num = unum;
		if (num & 0x400) {
			num -= 0x800;
		}
		os << num << '\n';
	}

	return EXIT_SUCCESS;
}