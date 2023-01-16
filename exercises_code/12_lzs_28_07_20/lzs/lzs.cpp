
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

struct dizionario {
	std::vector<char> diz_;
	int limite_;
	dizionario(int limite = 2047): diz_(0), limite_(limite){}
	
	int addElemento(char c) {
		int n = 0;
		if (diz_.size() == limite_) {
			diz_.erase(diz_.begin());
			n = 1;
		}
		diz_.push_back(c);
		return n;
	}
	std::string getOffLen(int offset, int len){
		if (offset > limite_ || offset < 1) {
			throw std::logic_error("Non puoi passare valori oltre al limite");
		}
		
		std::string s;
		int original_size = diz_.size();
		int eliminati = 0;
		for (int i = 0; i < len; i++) {
			int pos = (original_size - eliminati - offset) + i;
			
			if (pos > (diz_.size() - 1)) {
				pos = (diz_.size() - 1);
			}
			char c = diz_[pos];

			s += c;
			eliminati += addElemento(c);
		}

		return s;
	}

};

struct bitreader {
	std::istream& is_;
	uint8_t buffer_;
	int count;

	bitreader(std::istream& is) : is_(is), buffer_(0), count(0){}
	uint32_t leggi_bit() {
		
		uint32_t res = 0;
		if (count == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), 1);
			count = 8;
		}
		res = (res << 1) | ((buffer_ >> (count - 1)) & 1);
		count--;
		return res;
	}
	std::istream& leggi_bit(uint32_t& u, int n_bit) {

		while (n_bit--> 0 )
		{
			u = (u << 1) | leggi_bit();

		}
		return is_;
	}
};

/*
Fai lz77
2kb (str/vettore da 2000 elementi)
se il char non esiste => encodato
come 0 + 8b(il char)

Altrimenti ho
1 + encoded offset ([1,2047] max i.e. 11 bit) + length
Encoded offset
< 128 => 1 + 7b
>= 128 => 0 + 11b

Formato length
//nella slide


Esiste un end-marker
110000000
(seguito da 0-7b in modo da essere byte preciso)
*/

struct mappa_valori {
	std::unordered_map<int, int> vals_;
	mappa_valori() {
		vals_[0] = 2;
		vals_[1] = 3;
		vals_[2] = 4;
		vals_[12] = 5;
		vals_[13] = 6;
		vals_[14] = 7;
	}

	int getLength(int key) {
		return vals_[key]; //se = 0 => so come leggerlo
	}

	int read_len_gt_7(bitreader &br) {
		uint32_t tmp = 0;
		int N = 0;
		do{
			++N;
			tmp = 0;
			br.leggi_bit(tmp, 4);
		} while (tmp == 15);
		
		int length = (N * 15) - 7 + tmp;
		return length;

	}


};



int read_offset(bitreader& br) {
	int res = 0;
	uint32_t bit = 0;
	br.leggi_bit(bit, 1);
	int to_read = 11;
	if (bit) {
		//leggi 7
		bit = 0;
		to_read = 7;
	}
	br.leggi_bit(bit, to_read);
	res += bit;
	return res;
}
int read_length(bitreader& br){
	mappa_valori map;
	bool found = false;
	int res = 0;
	uint32_t key = 0;
	br.leggi_bit(key, 2);
	if (map.getLength(key) > 0) {
		return map.getLength(key);
	}
	else {
		br.leggi_bit(key, 2);
		if (key == 15) {
			return map.read_len_gt_7(br);
		}
		else {
			return map.getLength(key);
		}
	}
}

void lzs_decompress(std::istream& is, std::ostream& os) {
	if (!is || !os)
		return;

	bitreader br(is);
	dizionario diz;
	uint32_t tmp = 0;
	while (br.leggi_bit(tmp,1))
	{
		if (tmp) {
			int offset = read_offset(br);
			if (!offset) {
				break;
			}
			int len = read_length(br);
			std::string s = diz.getOffLen(offset, len);
			os << s;
			
		}
		else {
			//literal
			tmp = 0;
			uint8_t res = 0;
			br.leggi_bit(tmp, 8);
			res += (uint8_t)tmp;
			diz.addElemento(res);
			//std::cout << res;
			os.put(res);
		}
		tmp = 0;
	}
	
}
