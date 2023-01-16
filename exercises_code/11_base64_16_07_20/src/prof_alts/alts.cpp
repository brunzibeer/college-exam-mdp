#include <string>
#include <vector>

std::string base64_decode_1(const std::string& input) {
    std::string alphabet_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    std::vector<uint8_t> tmp_;
    std::string output_ = "";

    for (size_t i = 0; i < input.size(); ++i) {
        std::string::size_type check_ = alphabet_.find(input[i]);
        if (check_ == std::string::npos) { continue; }
        tmp_.push_back(check_);
        if (tmp_.size() == 4) {
            output_ += (tmp_[0] << 2) + ((tmp_[1] & 0x30) >> 4);
            if (tmp_[2] != 64)
                output_ += ((tmp_[1] & 0xf) << 4) + ((tmp_[2] & 0x3c) >> 2);
            if (tmp_[3] != 64)
                output_ += ((tmp_[2] & 0x3) << 6) + tmp_[3];
            tmp_.clear();
        }
    }

    return output_;
}

std::string base64_decode_2(const std::string& in) 
{
	static int table[256] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	};

	std::string out;
	int val[4];
	int n = 0;
	for (const auto &c : in) {
		if (c == '=') 
			break;
		if ((val[n] = table[c]) < 0) 
			continue;
		if (++n == 4) {
			out += (val[0] << 2) | (val[1] >> 4);
			out += ((val[1] & 0xf) << 4) | (val[2] >> 2);
			out += ((val[2] & 0x3) << 6) | val[3];
			n = 0;
		}
	}
	if (n > 1) {
		out += (val[0] << 2) | (val[1] >> 4);
		if (n > 2) {
			out += ((val[1] & 0xf) << 4) | (val[2] >> 2);
		}
	}

	return out;
}

