#include <iostream>
#include <fstream>

#include "pbm.h"

//bool BinaryImage::ReadFromPBM(const std::string& filename) {
//    std::ifstream is(filename, std::ios::binary);
//    if(!is) {
//        return false;
//    }
//    std::string MagicNumber;
//    std::getline(is, MagicNumber);
//    if(is.peek() == '#') {
//        std::string comment;
//        std::getline(is, comment);
//    }
//    is >> W >> H;
//    char newline;
//    is.get(newline);
//    if(newline != '\n') {
//        return false;
//    }
//    int h_bits_ = W / 8 + 1;
//    ImageData.resize(H * h_bits_);
//    for(auto& x : ImageData) {
//        is >> std::noskipws >> x;
//    }
//    return true;
//}

Image BinaryImageToImage(const BinaryImage& bimg) {
    Image img;
    img.cols() = bimg.cols();
    img.rows() = bimg.rows();

    int octects_ = bimg.cols() / 8;
    int spare_ = bimg.cols() % 8;

    uint8_t val;
    int count_ = 0;

    for(const auto& x : bimg.ImageData) {
        ++count_;
        if(count_ <= octects_) {
            for(int bit = 0; bit < 8; ++bit) {
                val = x >> (8 - bit - 1) & 1;
                if(val == 0) { img.push_back(0xff); }
                else { img.push_back(0x00); }
            }
        }
        else {
            for(int bit = 0; bit < spare_; ++bit) {
                val = x >> (8 - bit - 1) & 1;
                if(val == 0) { img.push_back(0xff); }
                else { img.push_back(0x00); } 
            }
            count_ = 0;
        }
    }
    

    return img;
}

//int main(int argc, char* argv[]) {
//    BinaryImage BI;
//    BI.ReadFromPBM(argv[1]);
//    BinaryImageToImage(BI);
//}