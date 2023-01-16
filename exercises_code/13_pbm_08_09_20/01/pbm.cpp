#include <iostream>
#include <fstream>

#include "pbm.h"

bool BinaryImage::ReadFromPBM(const std::string& filename) {
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        return false;
    }
    std::string MagicNumber;
    std::getline(is, MagicNumber);

    if(is.peek() == '#') {
        std::string comment;
        std::getline(is, comment);
    }

    is >> W >> H;
    char newline;
    is.get(newline);
    if(newline != '\n') {
        return false;
    }

    int h_bits_ = W / 8 + 1;

    ImageData.resize(H * h_bits_);

    for(auto& x : ImageData) {
        is >> std::noskipws >> x;
    }

    
    return true;
}

//int main(int argc, char* argv[]) {
//        BinaryImage BI;
//    BI.ReadFromPBM(argv[1]);
//}