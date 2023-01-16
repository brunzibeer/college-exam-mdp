#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <cmath>

void error(const char* err) {
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

template<typename T>
class bitwriter {
    std::ostream& os_;
    uint8_t buffer_;
    size_t nbits_;

    std::ostream& write_bit(T u) {
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

    std::ostream& write(T u, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            write_bit(u >> (n - 1 - i));
        }
        return os_;
    }

    std::ostream& operator()(T u, size_t n) {
        return write(u, n);
    }

    void flush(T u = 0) {
        while(nbits_ > 0) {
            write_bit(u);
        }
    }
};

template<typename T>
class bitreader {
    std::istream& is_;
    uint8_t buffer_;
    size_t nbits_;

    T read_bit() {
        if (nbits_ == 0) {
            raw_read(is_, buffer_);
            nbits_ = 8;
        }
        --nbits_;

        return (buffer_ >> nbits_) & 1; // Return the most significant bit
    }

public:
    bitreader(std::istream& is) : is_(is), nbits_(0) {}

    std::istream& read(T& u, size_t n) {
        u = 0;
        while (n --> 0) {
            u = (u << 1) | read_bit(); // Save the most significant bit
        }
        return is_;
    }

    std::istream& operator()(T& u, size_t n) {
        return read(u, n);
    }
};

class Encoder {
    std::istream& is_;
    std::ostream& os_;
    bitwriter<int32_t> bw;
    std::vector<int32_t> v;
    int32_t val;

    void fill_vector() {
        while(is_ >> val) {
            v.push_back(val);
        }
    }

public:
    Encoder(std::istream& is, std::ostream& os) : is_(is), os_(os), bw(os) {
        fill_vector();
    }

    void encode() {
        os_ << "BIN!";

        auto max = *std::max_element(v.begin(), v.end());
        auto min = *std::min_element(v.begin(), v.end());
        auto size = v.size();
        bw(min, 32);
        bw(max, 32);
        bw(size, 32);

        size_t range = max - min + 1;
        size_t nbits = std::ceil(std::log2(range));

        for (auto& it : v) {
            bw(it, nbits);
        }
    }
    
};

class Decoder {
    std::istream& is_;
    std::ostream& os_;
    bitreader<uint32_t> br;
    std::vector<int32_t> v;

public:
    Decoder(std::istream& is, std::ostream& os) : is_(is), os_(os), br(is) {}

    void decode() {
        char c;
        for (size_t i = 0; i < 4; ++i) {
            is_ >> c;
            os_ << c;
        }
        uint32_t val;
        br(val, 32);
        int32_t min = static_cast<int32_t>(val);
        os_ << "\n" << min;
        br(val, 32);
        int32_t max = static_cast<int32_t>(val);
        os_ << "\n" << max;
        br(val, 32);
        int32_t size = static_cast<int32_t>(val);
        os_ << "\n" << size;

        size_t range = max - min +1;
        size_t nbits = std::ceil(std::log2(range));

        while(br(val, nbits)) {
            int32_t num = val;
            if (num & 0x400) {
                num -= 0x800;
            }
            os_ << "\n" << num;
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        error(
            "Wrong Argument Number:\n"
            "binary [c|d] <input file> <output file>");
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[2]);
    if (!is) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    std::ofstream os(argv[3], std::ios::binary);
    if (!os) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    if (!std::strcmp("c", argv[1])) {
        Encoder enc(is, os);
        enc.encode();
    }
    else if (!std::strcmp("d", argv[1])) {
        Decoder dec(is, os);
        dec.decode();
    }
    else {
        error(
            "Wrong Argument Format:\n"
            "binary [c|d] <input file> <output file>");
    }
    

    return EXIT_SUCCESS;
}