#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>
#include <iterator>

void error(const char *msg)
{
	std::cout << msg;
	exit(EXIT_FAILURE);
}

void syntax() {
	error("SYNTAX:\n"
		"frequencies <input file> <output file>\n"
		"The program takes a binary file as input and for each byte "
		"(interpreted as an unsigned 8-bit integer) it "
		"counts its occurrences.\nThe output is a text file consisting "
		"of one line for each different byte found in the input file");
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
	is.unsetf(std::ios::skipws);

	/*
	typedef char* charptr;
	uint8_t my_byte;
	// the type of &my_byte is uint8_t* meaning a pointer to uint8_t
	is.read((char*)&my_byte, 1); // C-style cast
	//is.read(char*(&my_byte), 1); // function-style cast <-- doesn't work with the pointer modifier :-(
	is.read(charptr(&my_byte), 1); // function-style cast 

	(double)5; // You change and int to a double === static_cast 
	const int x = 5;
	int *p = (int*)&x; // removing constness === const_cast
	float f = 5.25;
	int *q = (int*)&f; // changing the interpretation of memory address === reinterpret_cast

	is.read(reinterpret_cast<char*>(&my_byte), 1); // C++ reinterpret_cast 
	*/

	frequency_counter f = for_each(istream_iterator<uint8_t>{is}, istream_iterator<uint8_t>{}, frequency_counter{});

	ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file.\n");
	}

	for (size_t i = 0; i < 256; ++i) {
		if (f[i] > 0) {
			// Notice how we need to set back decimal mode (I forgot to do that in class),
			// because std::hex sticks to the ostream and also the frequency gets formatted
			// as a hexadecimal value.
			os << setfill('0') << setw(2) << hex << i << dec; 
			os << '\t' << f[i] << '\n';
		}
	}

	cout << "Entropy: " << f.entropy() << "\n";

	return EXIT_SUCCESS;
}