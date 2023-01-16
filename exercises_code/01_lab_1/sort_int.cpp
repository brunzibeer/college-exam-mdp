#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void error(std::string err) {
    std::cout << err;
}

std::istream& read_file(std::istream& is, std::vector<int32_t>& v) {
    int32_t val;

    while (is >> val) {
        v.push_back(val);
    }
    return is;
}

std::ostream& write_file(std::ostream& os, const std::vector<int32_t>& v) {
    for (size_t i = 0; i < v.size(); ++i) {
        os << v[i] << "\n";
    }
    return os;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        error(
            "Wrong Argument Number:\n"
            "sort_int <filein.txt> <fileout.txt>");
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[1]);
    if (!is) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    std::ofstream os(argv[2]);
    if (!os) {
        error("Cannot open file.");
        return EXIT_FAILURE;
    }

    std::vector<int32_t> v;

    read_file(is, v);
    std::sort(v.begin(), v.end());
    write_file(os, v);

    return EXIT_SUCCESS;
}