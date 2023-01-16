#pragma once

#include <vector>
#include <string>
#include <cassert>

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

    uint8_t& operator()(int r, int c) { 
        assert(r >= 0 && c >= 0 && r < H && c < W);
        return ImageData[r*W + c];
    }
    const uint8_t& operator()(int r, int c) const { 
        assert(r >= 0 && c >= 0 && r < H && c < W);
        return ImageData[r*W + c];
    }

    int rows() const { return H; }
    int cols() const { return W; }
};

struct Image{
    int W;
    int H;
    std::vector<uint8_t> ImageData;

    int& rows() { return H; }
    int& cols() { return W; }
    int rows() const { return H; }
    int cols() const { return W; }

    uint8_t& operator()(int r, int c) { 
        assert(r >= 0 && c >= 0 && r < H && c < W);
        return ImageData[r*W + c];
    }
    const uint8_t& operator()(int r, int c) const { 
        assert(r >= 0 && c >= 0 && r < H && c < W);
        return ImageData[r*W + c];
    }

    void resize(int rows, int cols) { ImageData.resize(rows * cols); }

    void push_back(uint8_t val) { ImageData.push_back(val); }

    size_t size() { return ImageData.size(); }
};

Image BinaryImageToImage(const BinaryImage& bimg);