#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <iomanip>
#include <iterator>

void error(std::string err) {
    std::cout << err;
}

struct frequency_counter {
    std::array<uint8_t, 256> occurancies;

    frequency_counter() : occurancies { 0 } {}

    void operator()(uint8_t val) {
        ++occurancies[val];
    }

    const uint8_t& operator[](size_t pos) const {
        return occurancies[pos];
    }

    uint8_t& operator[](size_t pos) {
        return occurancies[pos];
    }

    double entropy() {
        double tot =  0.0;
        for (const auto& x : occurancies) {
            tot += x;
        }

        double H = 0.0;
        for (const auto& x : occurancies) {
            if (x > 0) {
                double px = x / tot;
                H += px * log2(px);
            }
        }

        return -H;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        error(
            "Wrong Argument Number:\n"
            "frequencies <filein> <fileout>");
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[1], std::ios::binary);
    if (!is) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    std::ofstream os(argv[2]);
    if (!os) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    frequency_counter f = std::for_each(
        std::istream_iterator<uint8_t>{is}, 
        std::istream_iterator<uint8_t>{}, 
            frequency_counter{}); // Unary function, it must implement operator()

    for (size_t i = 0; i < 256; ++i) {
        if (f[i] > 0) {
            os << std::setfill('0') << std::setw(2) << std::hex << i << std::dec; // Printing the byte as 2 digit hex
            os << "\t" << f[i] << "\n";
        }
    }

    std::cout << "Entropy: " << f.entropy() << "\n";

    return EXIT_SUCCESS;
}