#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "pgm.h"
#include "ppm.h"
#include "math.h"

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
    std::ifstream is(filename, std::ios::binary);
    if(!is) { return false; }

    std::string steam_header;
    std::getline(is, steam_header);
    
    // Retrieving H and W
    char next_char;
    std::string::size_type pos = steam_header.find('W');
    std::string width;
    
    while(true) {
        next_char = steam_header.at(++pos);
        if(next_char == ' ') { break; }
        width += next_char;
    }
    int W = std::stoi(width);
    pos = steam_header.find('H');
    std::string height;
    while(true) {
        next_char = steam_header.at(++pos);
        if(next_char == ' ') { break; }
        height += next_char;
    }
    int H = std::stoi(height);

    // Computing dimensions
    int Y_size = W * H;
    int CBCR_size = Y_size / 4;

    while(true) {
        std::string frame_header;
        std::getline(is, frame_header);
        mat<uint8_t> picture(H, W);

        for(auto& x : picture) { 
            is >> std::noskipws >> x;
        }
        frames.push_back(picture);
        is.ignore(CBCR_size * 2);
        if(is.peek() != 'F') { break; }

    }

    return true;
}

//int main(int argc, char *argv[]) {
//    std::string filename = argv[1];
//    std::vector<mat<uint8_t>> frames;
//    y4m_extract_gray(filename, frames);
//}

