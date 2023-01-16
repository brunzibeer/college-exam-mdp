#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <utility>
#include <algorithm>

namespace mdp {

	template<typename T>
	class vector {
		size_t size_;
		size_t capacity_;
		T* data_;
	public:
		// Constructor with initial size (also default constructor)
		vector(size_t starting_size = 0) : size_(starting_size), capacity_(starting_size) {
			data_ = new T[capacity_];
		}
		// Copy constructor 
		vector(const vector& other) : size_(other.size_), capacity_(other.capacity_) {
			data_ = new T[capacity_];
			std::copy(other.data_, other.data_ + size_, data_);
		}
		// Move constructor
		vector(vector&& other) { // r-value reference
			size_ = other.size_;
			capacity_ = other.capacity_;
			data_ = other.data_;
			other.data_ = nullptr; // I steal other.data_
		}
		vector& operator=(vector rhs) { // Copy and swap idiom
			swap(*this, rhs);
			return *this;
		}
		// Destructor
		~vector() {
			delete[] data_;
		}
		void resize(size_t new_size, size_t new_capacity = 0, bool force_capacity = false) {
			if (new_capacity == 0 || new_capacity < new_size) {
				new_capacity = new_size;
			}
			if (new_size > capacity_ || force_capacity) {
				capacity_ = new_capacity;
				auto* tmp = new T[capacity_];
				std::copy(data_, data_ + size_, tmp);
				delete[] data_;
				data_ = tmp;
			}
			size_ = new_size;
		}
		void push_back(const T& val) {
			resize(size_ + 1, capacity_ * 2 + 1);
			data_[size_ - 1] = val;
		}
		void shrink_to_fit() {
			resize(size_, size_, true);
		}
		bool empty() const {
			return size_ == 0;
		}
		size_t size() const {
			return size_;
		}
		auto* data() {
			return data_;
		}

		const auto& at(size_t pos) const {
			assert(pos < size_);
			return data_[pos];
		}
		auto& at(size_t pos) {
			assert(pos < size_);
			return data_[pos];
		}

		const auto& operator[](size_t pos) const {
			return data_[pos];
		}
		auto& operator[](size_t pos) {
			return data_[pos];
		}

		friend void swap(vector& a, vector& b) {
			using std::swap;
			swap(a.size_, b.size_);
			swap(a.capacity_, b.capacity_);
			swap(a.data_, b.data_);
		}
	};

	template<typename T>
	int comp(const void *va, const void *vb)
	{
		const auto* a = (const T *)va;
		const auto* b = (const T *)vb;

		if (*a < *b) {
			return -1;
		}
		else if (*b < *a) {
			return 1;
		}
		else {
			return 0;
		}
	}
	template<typename T>
	void sort(vector<T>& vec) {
		qsort(vec.data(), vec.size(), sizeof(T), comp<T>);
	}

	// Save the vector of int output to file
	bool write(const char *filename, const vector<int>& v);

	// Read a vector of int from file
	vector<int> read(const char *filename);
}
#endif VECTOR_H

