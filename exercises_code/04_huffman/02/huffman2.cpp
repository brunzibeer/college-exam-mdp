#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <array>
#include <iterator>
#include <vector>
#include <numeric>

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&num), size);
}

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&num), size);
}

class bitwriter {
    std::ostream& os_;
    uint8_t buffer_;
    size_t nbits_;

    std::ostream& write_bit(uint32_t u){
        buffer_ = (buffer_ << 1) | (u & 1);
        ++nbits_;
        if (nbits_ == 8) {
            raw_write(os_, buffer_);
            nbits_ = 0;
        }
        return os_;
    }

public:
    bitwriter(std::ostream& os) : os_(os), nbits_(0) {}

    ~bitwriter() {
        flush();
    }

    std::ostream& write(uint32_t u, size_t n) {
        while(n --> 0) {
            write_bit(u >> n);
        }
        return os_;
    }

    std::ostream& operator()(uint32_t u, size_t n) {
        return write(u, n);
    }

    void flush(uint32_t u = 0) {
        while(nbits_ > 0) {
            write_bit(u);
        }
    }
};

void setup(std::ifstream& is, std::ofstream& os, const std::string& filein, const std::string& fileout) {
    is = std::ifstream(filein, std::ios::binary);
    os = std::ofstream(fileout, std::ios::binary);
}

struct frequencies {
    std::array<size_t, 256> occurencies_;

    frequencies() : occurencies_{ 0 } {}

    void operator()(uint8_t val) {
        ++occurencies_[val];
    }

    size_t& operator[](size_t i) {
        return occurencies_[i];
    }

    const size_t& operator[](size_t i) const {
        return occurencies_[i];
    }
};

struct node {
    uint8_t sym_;
    size_t prob_;
    node* left_ = nullptr;
    node* right_ = nullptr;

    node(uint8_t sym, size_t prob) : sym_(sym), prob_(prob) {}
    node(node* a, node* b) : prob_(a->prob_ + b->prob_), left_(a), right_(b) {}
};

struct code {
    uint32_t len_, val_;
};

struct codes_table {
    std::unordered_map<uint8_t, code> table_;

    codes_table(const node* root) {
        compute_codes(root, 0, 0);
    }

    void compute_codes(const node* n, uint32_t len, uint32_t val) {
        if(n->left_ == nullptr) {
            table_[n->sym_] = { len, val };
        }
        else {
            compute_codes(n->left_, len + 1, val << 1); // Adding a 0
            compute_codes(n->right_, len + 1, val << 1 | 1); // Adding a 1
        }
    }
};

class Huffman {
    std::ifstream& is_;
    std::ofstream& os_;
    frequencies f_;
    std::vector<std::unique_ptr<node>> storage_;
    std::vector<node*> v_;
    bitwriter bw_;

    void initialize_nodes() {
        for (size_t i = 0; i < 256; ++i) {
            if(f_[i] > 0) {
                node* n = new node(static_cast<uint8_t>(i), f_[i]);
                storage_.emplace_back(n);
                v_.push_back(n);
            }
        }
        std::sort(
            v_.begin(), 
            v_.end(), 
            [](const node* a, const node* b){
                return a->prob_ > b->prob_;
            });
    }

    void create_huff_tree() {
        while (v_.size() > 1) {
            // Getting the last 2 nodes
            node* n1 = v_.back();
            v_.pop_back();
            node* n2 = v_.back();
            v_.pop_back();

            // Creating the new node as a sum of the 2
            node* n = new node(n1, n2);
            storage_.emplace_back(n);

            // Retrieving where to put the new node in the list
            auto pos = std::lower_bound(
                v_.begin(), 
                v_.end(), 
                [](const node* a, const node* b){
                    return a->prob_ > b->prob_;
                });
            
            // Inserting the new node in the vector
            v_.insert(pos, n);
        }
    }

public:
    Huffman(std::ifstream& is, std::ofstream& os) : is_(is), os_(os), bw_(os) {
        f_ = std::for_each(std::istreambuf_iterator<char>(is_), std::istreambuf_iterator<char>(), f_);
    }

    void encode() {
        initialize_nodes();
        create_huff_tree();

        auto root = v_.back();
        codes_table ct_(root);

        os_ << "HUFFMAN1";
        
        os_.put(static_cast<char>(ct_.table_.size()));
        for(const auto& x : ct_.table_) {
            bw_(x.first, 8);
            bw_(x.second.len_, 5);
            bw_(x.second.val_, x.second.len_);
        }

        uint32_t num_symbols_ = std::accumulate(
            f_.occurencies_.begin(), 
            f_.occurencies_.end(), 
            0);
        bw_(num_symbols_, 32);

        is_.clear();
        is_.seekg(0);

        while(true) {
            int c = is_.get();
            if(c == EOF) {
                break;
            }
            auto hc = ct_.table_[c];
            bw_(hc.val_, hc.len_);
        }
    }

    void decode() {}
};

int main(int argc, char *argv[]) {
    std::ifstream is;
    std::ofstream os;
    setup(is, os, argv[2], argv[3]);
    std::string operation = argv[1];

    Huffman H(is, os);

    if(operation == "c") {
        H.encode();
    }
    else if(operation == "d") {
        H.decode();
    }
    else { std::cout << "Wrong Syntax! First param should be 'c' or 'd'."; }
}