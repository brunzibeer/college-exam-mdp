#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <iomanip>
#include <iterator>
#include <cstdint>

void error(std::string err) {
    std::cout << err;
}

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&num), size); // Reinterpret cast because os.write() wants a char*
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        error(
            "Wrong Argument Number:\n"
            "frequencies <filein> <fileout>");
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[1]);
    if (!is) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    std::ofstream os(argv[2], std::ios::binary);
    if (!os) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    int32_t num;
    while(raw_read(is, num)) {
        os << num << "\n";
    }

    return EXIT_SUCCESS;
}