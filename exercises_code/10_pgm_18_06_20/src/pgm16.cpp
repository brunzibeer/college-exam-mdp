#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include "pgm16.h"

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cout << "Problem occured while opening the file!\n";
        return false;
    }
    std::string magicnumber;
    std::getline(is, magicnumber);

    if(is.peek() == '#') {
        std::string comment;
        std::getline(is, comment);
    }

    int W, H;
    is >> W >> H;

    char newline;
    is.get(newline);
    std::string max;
    std::getline(is, max);
    maxvalue = static_cast<uint16_t>(std::stoi(max));
    img.resize(H, W);

    if(maxvalue < 256) {
        uint8_t val;
        for(auto& x : img) {
            is >> std::noskipws >> val;
            x = val;
        }
    }
    else {
        uint8_t val1, val2;
        for(auto& x : img) {
            is >> std::noskipws >> val1 >> val2;
            uint16_t big_endian = (val1 << 8) + val2;
            x = big_endian;
        }
    }

    return true;
}

// int main(int argc, char *argv[]) {
//     std::string filename = argv[1];
//     mat<uint16_t> img;
//     uint16_t maxvalue;
// 
//     load(filename, img, maxvalue);
// }