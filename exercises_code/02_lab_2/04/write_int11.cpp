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

class bitwriter {
    std::ostream& os_;
    uint8_t buffer_;
    size_t nbits_;

    std::ostream& write_bit(uint32_t u) {
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

    std::ostream& write(uint32_t u, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            write_bit(u >> (n - 1 - i));
        }
        return os_;
    }

    std::ostream& operator()(uint32_t u, size_t n) {
        return write(u, n);
    }

    void flush(uint32_t u = 0) {
        while(nbits_ > 0) {
            write_bit(u);
        }
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

    bitwriter bw(os);

    int32_t num;
    while(is >> num) {
        bw(num, 11);
    }

    return EXIT_SUCCESS;
}