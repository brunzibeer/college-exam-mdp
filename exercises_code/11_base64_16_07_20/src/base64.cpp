#include <string>
#include <iostream>
#include <vector>

#include "base64.h"

std::string base64_decode(const std::string& input) {
    std::string alphabet_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    std::vector<uint8_t> tmp_;
    std::vector<uint8_t> buf_{0, 0, 0};
    std::string output_ = "";

    for(size_t i = 0; i < input.size(); ++i) {
        std::string::size_type check_ = alphabet_.find(input[i]);
        if(check_ == std::string::npos) { continue; }
        tmp_.push_back(input[i]);
        if(tmp_.size() == 4) {
            int count_ = std::count(std::begin(tmp_), std::end(tmp_), '=');
            if(count_ == 0) {
                for(int j = 0; j < 4; ++j){
                    for(size_t k = 0; k < alphabet_.size(); ++k) {
                        if(tmp_[j] == alphabet_[k]) {
                            tmp_[j] = k;
                            break;
                        }
                    }
                }
                buf_[0] = (tmp_[0] << 2) + ((tmp_[1] & 0x30) >> 4);
                buf_[1] = ((tmp_[1] & 0xf) << 4) + ((tmp_[2] & 0x3c) >> 2);
                buf_[2] = ((tmp_[2] & 0x3) << 6) + tmp_[3];
                output_ += buf_[0];
                output_ += buf_[1];
                output_ += buf_[2];
                tmp_.clear();
            }
            else if(count_ == 2) {
                for(int j = 0; j < 2; ++j) {
                    for(size_t k = 0; k < alphabet_.size(); ++k) {
                        if(tmp_[j] == alphabet_[k]) {
                            tmp_[j] = k;
                            break;
                        }
                    }
                }
                buf_[0] = (tmp_[0] << 2) + ((tmp_[1] & 0x30) >> 4);
                output_ += buf_[0];
                tmp_.clear();
            }
            else if(count_ == 1) {
                for(int j = 0; j < 3; ++j) {
                    for(size_t k = 0; k < alphabet_.size(); ++k) {
                        if(tmp_[j] == alphabet_[k]) {
                            tmp_[j] = k;
                            break;
                        }
                    }
                }
                buf_[0] = (tmp_[0] << 2) + ((tmp_[1] & 0x30) >> 4);
                buf_[1] = ((tmp_[1] & 0xf) << 4) + ((tmp_[2] & 0x3c) >> 2);
                output_ += buf_[0];
                output_ += buf_[1];
                tmp_.clear();
            }
        }
    }

    return output_;
}

//int main() {
//    std::string input = "UHJldHR5IGxvbmcgdGV4dCB3aGljaCByZXF1aXJlcyBtb3JlIHRoYW4gNzYgY2hhcmFjdGVycyB0\nbyBlbmNvZGUgaXQgY29tcGxldGVseS4=";
//    std::string out = base64_decode(input);
//    std::cout << out;
//
//    return EXIT_SUCCESS;
//}