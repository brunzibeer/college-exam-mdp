#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <iomanip>
#include <iterator>

void error(std::string err) {
    std::cout << err;
}

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&num), size); // Reinterpret cast because os.write() wants a char*
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
    while (is >> num) {
        raw_write(os, num);
    }

    return EXIT_SUCCESS;
}