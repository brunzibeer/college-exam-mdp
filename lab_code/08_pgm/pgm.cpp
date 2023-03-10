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
