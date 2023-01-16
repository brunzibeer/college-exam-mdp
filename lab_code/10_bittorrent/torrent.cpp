#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <list>
#include <regex>
#include <sstream>
#include <memory>
#include <iomanip>

class Item {
protected:
    static bool ReadUpToX(std::istream &is, std::string &data, char x) {
        while (is.peek() != std::char_traits<char>::eof() && is.peek() != x) {
            data += is.get();
        }
        return is && is.peek() == x;
    }
    static void ReadExpectedCharacter(std::istream &is, char expected) {
        using namespace std::literals;
        int c = is.get();
        if (c != expected) {
            throw std::logic_error("Expected character was "s + expected + " but got " + static_cast<char>(c));
        }
    }

public:
    static std::unique_ptr<Item> Decode(std::istream &input, const std::string &key = "");
    virtual std::ostream &Print(std::ostream &os, int tab) const = 0;
    virtual ~Item() = default;
};

class StringItem : public Item {
protected:
    std::string utf8_value_;
public:
    StringItem(std::istream &input) {
        std::string asccii_string_len;
        if (!ReadUpToX(input, asccii_string_len, ':')) {
            throw std::logic_error("Unable to read string size.");
        }

        std::string::size_type string_len;
        std::stringstream ss(asccii_string_len);
        ss >> std::dec >> string_len;

        ReadExpectedCharacter(input, ':');

        // Let's read the string (still utf8, no decoding here)
        utf8_value_.resize(string_len);
        input.read(&utf8_value_[0], string_len);
        if (input.gcount() != string_len) {
            throw std::logic_error("Unable to read the entire string.");
        }
    }

    bool operator<(const StringItem &rhs) const {
        return utf8_value_ < rhs.utf8_value_;
    }

    std::ostream &Print(std::ostream &os, int tab) const {
        os << '"';
        for (const auto &c : utf8_value_) {
            os << (c >= 32 && c <= 126 ? c : '.');
        }
        os << '"';
        return os;
    }

    operator std::string() const { return utf8_value_; }
};

class PiecesItem : public StringItem {
public:
    PiecesItem(std::istream &input) : StringItem(input) {}

    std::ostream &Print(std::ostream &os, int tab) const {
        using namespace std;
        for (size_t i = 0; i < utf8_value_.size(); i += 20) {
            os << '\n' << std::string(tab + 1, '\t');
            for (size_t j = 0; j < 20; ++j) {
                os << hex << setw(2) << setfill('0') << + unsigned(char(utf8_value_[i + j]));
            }
        }
        return os;
    }
};
 
class IntegerItem : public Item {
    int64_t value_;
public:
    IntegerItem(std::istream &input) {
        std::string encoded_integer;
        if (!ReadUpToX(input, encoded_integer, 'e')) {
            throw std::logic_error("Unable to read integer item.");
        }

        std::regex r("i([-+]?(0|[1-9][0-9]*))");
        std::smatch m;
        if (!std::regex_match(encoded_integer, m, r)) {
            throw std::logic_error("Wrong integer format.");
        }
        ReadExpectedCharacter(input, 'e');

        std::stringstream ss(m[1].str());
        ss >> std::dec >> value_;
    }

    std::ostream &Print(std::ostream &os, int tab) const {
        os << value_;
        return os;
    }
};

class ListItem : public Item {
    std::list<std::unique_ptr<Item>> values_;
public:
    ListItem(std::istream &input) {
        ReadExpectedCharacter(input, 'l');
        while (input && input.peek() != 'e') {
            values_.push_back(Decode(input));
        }
        ReadExpectedCharacter(input, 'e');
    }

    std::ostream &Print(std::ostream &os, int tab) const {
        os << "[\n";
        tab++;
        for (const auto &e : values_) {
            os << std::string(tab, '\t');
            e->Print(os, tab);
            os << '\n';
        }
        os << std::string(tab - 1, '\t') << ']';
        return os;
    }
};

class DictItem : public Item {
    std::map<StringItem, std::unique_ptr<Item>> values_;
public:
    DictItem(std::istream &input) {
        ReadExpectedCharacter(input, 'd');
        while (input && input.peek() != 'e') {
            StringItem key(input);
            values_[key] = Decode(input, std::string(key));
        }
        ReadExpectedCharacter(input, 'e');
    }

    std::ostream &Print(std::ostream &os, int tab) const {
        os << "{\n";
        tab++;
        for (const auto &e : values_) {
            os << std::string(tab, '\t');
            e.first.Print(os, 0);
            os << " => ";
            e.second->Print(os, tab);
            os << '\n';
        }
        os << std::string(tab - 1, '\t') << '}';
        return os;
    }
};

std::unique_ptr<Item> Item::Decode(std::istream &input, const std::string& key) {
    switch (input.peek()) {
    case 'd':
        return std::make_unique<DictItem>(input);
    case 'i':
        return std::make_unique<IntegerItem>(input);
    case 'l':
        return std::make_unique<ListItem>(input);
    default:
        if (key=="pieces") {
            return std::make_unique<PiecesItem>(input);
        }
        else {
            return std::make_unique<StringItem>(input);
        }
    }

    // This should never happen
    return std::unique_ptr<Item>();
}

int main(int argc, char **argv) {
    {
        if (argc != 2) {
            std::cerr << "Wrong params: usage 'torrent_dump <file .torrent>'";
            return EXIT_FAILURE;
        }

        std::ifstream is(argv[1], std::ios::binary);
        if (!is) {
            std::cerr << "I'm not able to open the input file!";
            return EXIT_FAILURE;
        }

        auto data = Item::Decode(is);
        data->Print(std::cout, 0);
    }
    return EXIT_SUCCESS;
}