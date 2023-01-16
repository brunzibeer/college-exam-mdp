#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>

#include "lzs.h"

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&num), size);
}

class bitreader {
    std::istream& is_;
    uint8_t buffer_;
    size_t nbits_;

    uint32_t read_bit() {
        if(nbits_ == 0) {
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

    std::istream& operator()(uint32_t& u, size_t n) {
        return read(u, n);
    }
};

void lzs_decompress(std::istream& is, std::ostream& os) {
    std::deque<uint8_t> dict_;
    bitreader br_(is);
    bool end_ = false;

    uint32_t val_;
    while(br_(val_, 1) && !end_) {
        switch(val_) {
            case 0:
                uint32_t lit_;
                br_(lit_, 8);

                dict_.push_back((uint8_t)(lit_));

                if(dict_.size() > 2048) { dict_.pop_front(); }

                os << (char)(lit_);
                break;
            case 1:
                uint32_t marker_;
                br_(marker_, 1);

                uint32_t offset_;
                if(marker_ == 1 ) { 
                    br_(offset_, 7);
                    if(offset_ == 0) { end_ = true; break; }
                }
                else { br_(offset_, 11); }
                
                int len_;
                uint32_t bits2_;
                br_(bits2_, 2);

                if(bits2_ < 3) { len_ = bits2_ + 2; }
                else {
                    uint32_t morebits_;
                    br_(morebits_, 2);
                    if(morebits_ < 3) { len_ = bits2_ + morebits_ + 2; }
                    else {
                        uint32_t bits4_;
                        while(true) {
                            br_(bits4_, 4);
                            if(bits4_ != 15) { break; }
                        }
                        len_ = bits4_ + 8;
                    }
                }
                for(int i = 0; i < len_; ++i) {
                    uint8_t val = dict_[dict_.size() - offset_];
                    dict_.push_back(val);
                    if(dict_.size() > 2048) { dict_.pop_front(); }
                    os << val;
                }
                break;

        }
    }
    std::cout << "Done!";
}

int main(int argc, char* argv[]) {
    std::ifstream is(argv[1], std::ios::binary);
    std::ofstream os("out.txt");
    lzs_decompress(is, os);
    return EXIT_SUCCESS;
}