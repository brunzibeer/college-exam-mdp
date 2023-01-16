#include "pgm.h"

#include <fstream>
#include <iterator>

bool write(const std::string& filename, const mat<uint8_t>& img, pgm_mode mode)
{
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}
	os << "P" << int(mode) << "\n# MDP 2021\n";
	os << img.cols() << " " << img.rows() << "\n255\n";

	if (mode == pgm_mode::plain) {
		/*for (int r = 0; r < img.rows(); ++r) {
			for (int c = 0; c < img.cols(); ++c) {
				os << int(img(r, c)) << ' ';
			}
			os << '\n';
		}*/
		//for (int i = 0; i < img.size(); ++i) {
		//	os << int(img[i]) << ' ';
		//}
		//for (auto it = img.begin(); it != img.end(); ++it) {
		//	os << int(*it) << ' ';
		//}
		//for (const auto& x : img) {
		//	os << int(x) << ' ';
		//}
		copy(img.begin(), img.end(), std::ostream_iterator<int>(os, " "));
	}
	else {
		os.write(img.rawdata(), img.rawsize());
	}

	return true;
}

mat<uint8_t> read(const std::string& filename)
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

