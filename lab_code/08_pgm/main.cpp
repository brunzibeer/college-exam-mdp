#include <fstream>
#include <iostream>
#include <string>

#include "pgm.h"

void error(const std::string& message)
{
	std::cout << message;
	exit(EXIT_FAILURE);
}

auto make_test_pattern()
{
	mat<uint8_t> img(256, 256);

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img(r, c) = r; // Filling with nonsense values
		}
	}

	return img;
}

auto read(const std::string& filename) 
{
	mat<uint8_t> img;

	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return img;
	}

	std::string magic_number;
	std::getline(is, magic_number);
	pgm_mode mode;
	if (magic_number == "P2") {
		mode = pgm_mode::plain;
	}
	else if (magic_number == "P5") {
		mode = pgm_mode::binary;
	}
	else {
		return img;
	}

	if (is.peek() == '#') {
		std::string comment;
		std::getline(is, comment);
	}

	int width, height, nlevels;
	char newline;
	is >> width >> height >> nlevels;
	is.get(newline);
	if (nlevels != 255 || newline != '\n') {
		return img;
	}

	img.resize(height, width);

	if (mode == pgm_mode::plain) {
		for (auto& x : img) {
			int value;
			is >> value;
			x = value;
		}
	}
	else {
		is.read(img.rawdata(), img.rawsize());
	}

	return img;
}

void flip(mat<uint8_t>& img)
{
	for (int r = 0; r < img.rows()/2; ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			std::swap(img(r, c), img(img.rows() - 1 - r, c));
		}
	}
}

int main()
{
	mat<uint8_t> img = make_test_pattern();

	write("test_plain.pgm", img, pgm_mode::plain);
	write("test_bin.pgm", img, pgm_mode::binary);

	img = read("frog_asc.pgm");
	if (img.empty()) {
		error("Cannot read image.\n");
	}
	img = read("frog_bin.pgm");
	if (img.empty()) {
		error("Cannot read image.\n");
	}
	flip(img);
	write("flipped_frog.pgm", img, pgm_mode::binary);	

	return 0;
}