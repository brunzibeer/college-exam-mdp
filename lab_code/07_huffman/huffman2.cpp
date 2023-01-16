#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>
#include <iterator>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <numeric>

void error(const char *msg)
{
	std::cout << msg;
	exit(EXIT_FAILURE);
}

void syntax() {
	error("SYNTAX:\n"
		"huffman1 [c|d] <input file> <output file>\n");
}

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T))
{
	return is.read(reinterpret_cast<char*>(&num), size);
}
template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&num), size);
}

class bitreader {
	std::istream& is_;
	uint8_t buffer_;
	size_t nbits_;

	uint32_t read_bit() {
		if (nbits_ == 0) {
			raw_read(is_, buffer_);
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is), nbits_(0) {}

	std::istream& read(uint32_t& u, size_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	std::istream& read(int32_t& i, size_t n) {
		uint32_t u;
		read(u, n);
		i = static_cast<int32_t>(u);
		return is_;
	}
};

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_;
	size_t nbits_;

	std::ostream& write_bit(uint32_t u) {
		// buffer_ = buffer_ * 2 + u % 2;
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
		while (n --> 0) {
			write_bit(u >> n);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, size_t n) {
		return write(u, n);
	}

	void flush(uint32_t u = 0) {
		while (nbits_ > 0) {
			write_bit(u);
		}
	}
};

struct frequency_counter {
	std::array<size_t, 256> occurrencies;

	frequency_counter() : occurrencies{ 0 } {}

	void operator()(uint8_t val) {
		++occurrencies[val];
	}

	const size_t& operator[](size_t pos) const {
		return occurrencies[pos];
	}
	size_t& operator[](size_t pos) {
		return occurrencies[pos];
	}

	double entropy() {
		double tot = 0.0;
		for (const auto& x : occurrencies) {
			tot += x;
		}

		double H = 0.0;
		for (const auto& x : occurrencies) {
			if (x > 0) {
				double px = x / tot;
				H += px * log2(px);
			}
		}

		return -H;
	}
};

void encode(const std::string& input, const std::string& output)
{
	using namespace std;
	ifstream is(input, ios::binary);
	if (!is) {
		error("Cannot open input file\n");
	}

	frequency_counter f;
	/*while (true) {
		int c = is.get();
		if (c == EOF) {
			break;
		}
		f(c);
	}*/
	/*char c;
	while (is.get(c)) {
		f(c);
	}*/
	f = for_each(istreambuf_iterator<char>(is), istreambuf_iterator<char>(), f);
	cout << f.entropy();

	struct node {
		uint8_t sym_;
		size_t prob_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(uint8_t sym, size_t prob) : // Initial nodes
			sym_(sym), prob_(prob) {}
		node(node* a, node *b) : // Combines nodes
			prob_(a->prob_ + b->prob_),
			left_(a), right_(b) {}
	};

	vector<unique_ptr<node>> storage;
	vector<node*> vec;
	for (size_t i = 0; i < 256; ++i) {
		if (f[i] > 0) { // Create a node only if its symbol exists
			node* n = new node(static_cast<uint8_t>(i), f[i]);
			storage.emplace_back(n);
			vec.push_back(n);
		}
	}
	std::sort(begin(vec), end(vec), 
		[](const node* a, const node* b) {
			return a->prob_ > b->prob_;
		}
	);

	while (vec.size() > 1) {
		// Take the two least probable nodes and remove them from the vec
		node* n1 = vec.back();
		vec.pop_back();
		node* n2 = vec.back();
		vec.pop_back();
		// Combine them in a new node
		node* n = new node(n1, n2);
		storage.emplace_back(n);
		// Add the new node back into the vector in the correct position
		auto it = lower_bound(begin(vec), end(vec), n, 
			[](const node* a, const node* b) {
				return a->prob_ > b->prob_;
			}
		);
		vec.insert(it, n);
	}
	node* root = vec.back();

	struct huffman {
		struct code {
			uint32_t len_, val_;
		};
		unordered_map<uint8_t, code> codes_table_;

		huffman(const node* root) {
			compute_codes(root, 0, 0);
		}

		void compute_codes(const node* p, uint32_t len, uint32_t val) {
			if (p->left_ == nullptr) {
				codes_table_[p->sym_] = { len, val };
			}
			else {
				compute_codes(p->left_, len + 1, val << 1);
				compute_codes(p->right_, len + 1, val << 1 | 1);
			}
		}
	};

	huffman h(root);

	ofstream os(output, ios::binary);
	if (!os) {
		error("Cannot open output file\n");
	}

	os << "HUFFMAN1";
	os.put(static_cast<char>(h.codes_table_.size()));
	bitwriter bw(os);
	for (const auto& x : h.codes_table_) {
		bw(x.first, 8);
		bw(x.second.len_, 5);
		bw(x.second.val_, x.second.len_);
	}
	uint32_t num_symbols = accumulate(begin(f.occurrencies),
		end(f.occurrencies), 0);
	bw(num_symbols, 32);

	is.clear();
	is.seekg(0);

	while (true) {
		int c = is.get();
		if (c == EOF) {
			break;
		}
		auto hc = h.codes_table_[c];
		bw(hc.val_, hc.len_);
	}
}

void decode(const std::string& input, const std::string& output)
{
	using namespace std;
	ifstream is(input, ios::binary);
	if (!is) {
		error("Cannot open input file\n");
	}

	string MagicNumber(8,' ');
	raw_read(is, MagicNumber[0], 8);
	if (MagicNumber != "HUFFMAN1") {
		error("Wrong input format\n");
	}

	size_t TableEntries = is.get();
	if (TableEntries == 0) {
		TableEntries = 256;
	}

	bitreader br(is);
	struct triplet {
		uint32_t sym_;
		uint32_t len_;
		uint32_t code_;

		bool operator<(const triplet& rhs) const {
			return len_ < rhs.len_;
		}
	};
	vector<triplet> table;
	for (size_t i = 0; i < TableEntries; ++i) {
		triplet t;
		br.read(t.sym_, 8);
		br.read(t.len_, 5);
		br.read(t.code_, t.len_);
		table.push_back(t);
	}
	sort(begin(table), end(table));

	uint32_t NumSymbols;
	br.read(NumSymbols, 32);

	ofstream os(output, ios::binary);
	if (!os) {
		error("Cannot open output file\n");
	}

	for (size_t i = 0; i < NumSymbols; ++i) {
		uint32_t len = 0, code = 0;
		size_t pos = 0;
		do {
			while (table[pos].len_ > len) {
				uint32_t bit;
				br.read(bit, 1);
				code = (code << 1) | bit;
				++len;
			}
			if (code == table[pos].code_) {
				break;
			}
			++pos;
		} while (pos < table.size());
		if (pos == table.size()) {
			error("This shouldn't happen!\n");
		}
		os.put(table[pos].sym_);
	}
}

int main(int argc, char *argv[])
{
	using namespace std;
	{
		if (argc != 4) {
			syntax();
		}

		string operation = argv[1];
		if (operation == "c") {
			encode(argv[2], argv[3]);
		}
		else if (operation == "d") {
			decode(argv[2], argv[3]);
		}
		else {
			error("First parameter is wrong\n");
		}
	}
	return EXIT_SUCCESS;
}