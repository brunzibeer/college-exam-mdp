#pragma once

#include <istream>
#include <ostream>

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
		while (n-- > 0) {
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
