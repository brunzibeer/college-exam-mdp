#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <iomanip>
#include <iterator>

void error(std::string err) {
    std::cout << err;
}

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& to_write, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&to_write), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& read_into, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&read_into), size);
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
        while(n --> 0) {
            u = (u << 1) | read_bit();
        }
        return is_;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        error(
            "Wrong Argument Number:\n"
            "frequencies <filein> <fileout>");
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[1]);
    if (!is) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    std::ofstream os(argv[2], std::ios::binary);
    if (!os) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    bitreader br(is);

    uint32_t unsigned_num;
    while(br.read(unsigned_num, 11)) {
        // Positivity / Negativity Check
        int32_t num = unsigned_num;
        if (num & 0x400) {
            num -= 0x800;
        }
        os << num << '\n';
    }

    return EXIT_SUCCESS;
}