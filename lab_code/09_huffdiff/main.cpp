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
#include <set>

#include "bits.h"
#include "freq.h"
#include "pgm.h"
#include "huffman.h"

void error(const char *msg)
{
	std::cout << msg;
	exit(EXIT_FAILURE);
}

void syntax() {
	error("SYNTAX:\n"
		"huffdiff [c|d] <input file> <output file>\n");
}

void encode(const std::string& input, const std::string& output)
{
	auto img = read(input);
	if (img.empty()) {
		error("Failed to open input image.\n");
	}

	mat<int> diff(img.rows(), img.cols());
	int prev = 0;
	for (int r = 0; r < diff.rows(); ++r) {
		for (int c = 0; c < diff.cols(); ++c) {
			diff(r, c) = img(r, c) - prev;
			prev = img(r, c);
		}
		prev = img(r, 0);
	}

	frequency_counter<int> f;
	f = for_each(diff.begin(), diff.end(), f);
	std::cout << f.entropy();

	huffman<int> h;
	h.create_table(f);
	h.compute_canonical_codes();

	std::ofstream os(output, std::ios::binary);
	if (!os) {
		error("Cannot open output file\n");
	}

	os << "HUFFDIFF";
	int Width = img.cols();
	raw_write(os, Width);
	int Height = img.rows();
	raw_write(os, Height);
	bitwriter bw(os);
	bw(h.codes_table_.size(), 9);
	for (const auto& x : h.codes_table_) {
		bw(x.sym_, 9);
		bw(x.len_, 5);
	}
	std::unordered_map<int, huffman<int>::code> search_map;
	for (const auto& x : h.codes_table_) {
		search_map[x.sym_] = x;
	}
	for (const auto& x : diff) {
		auto hc = search_map[x];
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
	if (MagicNumber != "HUFFDIFF") {
		error("Wrong input format\n");
	}

	int Width, Height;
	raw_read(is, Width);
	raw_read(is, Height);
	
	bitreader br(is);
	uint32_t tmp;
	br.read(tmp, 9);
	size_t TableEntries = tmp;

	huffman<int> h;
	for (size_t i = 0; i < TableEntries; ++i) {
		huffman<int>::code t;
		br.read(t.sym_, 9);
		br.read(t.len_, 5);
		h.codes_table_.push_back(t);
	}
	h.compute_canonical_codes();

	mat<int> diff(Height, Width);

	for (int r = 0; r < diff.rows(); ++r) {
		for (int c = 0; c < diff.cols(); ++c) {
			uint32_t len = 0, code = 0;
			size_t pos = 0;
			do {
				while (h.codes_table_[pos].len_ > len) {
					uint32_t bit;
					br.read(bit, 1);
					code = (code << 1) | bit;
					++len;
				}
				if (code == h.codes_table_[pos].val_) {
					break;
				}
				++pos;
			} while (pos < h.codes_table_.size());
			if (pos == h.codes_table_.size()) {
				error("This shouldn't happen!\n");
			}
			diff(r, c) = h.codes_table_[pos].sym_;
		}
	}

	mat<uint8_t> img(diff.rows(), diff.cols());
	int prev = 0;
	for (int r = 0; r < diff.rows(); ++r) {
		for (int c = 0; c < diff.cols(); ++c) {
			img(r, c) = diff(r, c) + prev;
			prev = img(r, c);
		}
		prev = img(r, 0);
	}

	write(output, img, pgm_mode::binary);
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