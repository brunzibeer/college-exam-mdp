#pragma once

#include <vector>
#include <string>

struct BinaryImage {
    int W;
    int H;
    std::vector<uint8_t> ImageData;

    bool ReadFromPBM(const std::string& filename);

    int size() const { return W*H; }

    template<typename T>
    std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
	    return is.read(reinterpret_cast<char*>(&num), size);
    }
};