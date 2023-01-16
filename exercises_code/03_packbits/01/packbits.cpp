#include <iostream>
#include <fstream>
#include <vector>

template <typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T))
{
	return is.read(reinterpret_cast<char*>(&val), size);
}
/*
    FSM:
    - State 1: Only 1 char read
    - State 2: Run state
    - State 3: Copy state
*/

class Encoder {
    std::ifstream& is_;
    std::ofstream& os_;
    std::vector<uint8_t> buf_;
    int state_ = 0;
    uint8_t count_, val_;
    bool new_line_issue = false;

public:
    Encoder(std::ifstream& is, std::ofstream& os) : is_(is), os_(os) {}

    void encode() {
        while(raw_read(is_, val_)) {
            switch(state_) {
                case 0:
                    if(!buf_.empty()) {
                        if(val_ == buf_.back()) { state_ = 1; }
                        else { state_ = 2; }
                    }
                    buf_.push_back(val_);
                    break;
                case 1:
                    if(val_ != buf_.back()) {
                        raw_write(os_, (257 - buf_.size()));
                        raw_write(os_, buf_.back());
                        buf_.clear();
                        state_ = 0;
                    }
                    buf_.push_back(val_);
                    break;
                case 2:
                    if(val_ == buf_.back()) {
                        buf_.pop_back();
                        raw_write(os_, (buf_.size() - 1));
                        for(const auto& y : buf_) { raw_write(os_, y); }
                        auto temp = buf_.back();
                        buf_.clear();
                        buf_.push_back(temp);
                        state_ = 1;
                    }
                    buf_.push_back(val_);
                    break;
            }
        }
        if(state_ == 0) {
            raw_write(os_, 0);
            raw_write(os_, buf_.back());
        }
        else if(state_ == 1) {
            raw_write(os_, (257 - buf_.size()));
            raw_write(os_, buf_.back());
        }
        else {
            raw_write(os_, (buf_.size() - 1));
            for(const auto& y : buf_) { raw_write(os_, y); }
        }
        raw_write(os_, 128);
    }
};

int main(int argc, char *argv[]) {
    if(argc != 4) {
        std::cout << "Syntax Error!";
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[2]);
    if(!is) {
        return EXIT_FAILURE;
    }
    std::ofstream os(argv[3]);
    if(!os) {
        return EXIT_FAILURE;
    }

    Encoder enc(is, os);
    enc.encode();
}