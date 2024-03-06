#pragma once
#include "basic.h"

template<typename T, usize S>
struct Buffer {
	// std::array<T, S> data;	
	T data[S];
	constexpr Buffer() {}

	constexpr Buffer(const T &val) {
		for(int i = 0; i < S; ++i) data[i] = val;
	}
	// explicit Buffer(T _value): data(_value) {}	
	
	constexpr Buffer(std::initializer_list<T> l) {
		assert(l.size() == S, "initializer_list size != buffer size");
		for(int i = 0; i < S; ++i) data[i] = *(l.begin()+i);
	}
	// See std::to_array
	// constexpr Buffer(std::array<T, S> &&arr): data(arr) {}
	
	inline constexpr T &operator[](isize i) {
		assert(0 <= i && i < S, "Array out of bounds for index " + std::to_string(i));
		return data[i];
	}
	inline constexpr const T &operator[](isize i) const {
		assert(0 <= i && i < S, "Array out of bounds for index " + std::to_string(i));
		return data[i];
	}
	inline constexpr isize size() const {
		return static_cast<isize>(S);
	}
	inline constexpr T *begin() { return &data[0];}
	inline constexpr T *end() { return &data[S];}
	
	inline constexpr const T *begin() const { return &data[0];}
	inline constexpr const T *end() const { return &data[S];}


	void constexpr fill(const T &val) {
		for(isize i = 0; i < S; ++i) {
			this->data[i] = val;
		}
	}
	void constexpr operator=(const Buffer<T, S> &other) {
		for(isize i = 0; i < S; ++i) {
			this->data[i] = other[i];
		}
	}
};
template<typename T, usize S>
std::ostream &operator<<(std::ostream &os, const Buffer<T, S> &arr) {
	os << "[";
	
	if(arr.size() > 0) {
		for(auto i = 0; i < arr.size() - 1; i+=1) {
			os << arr[i] << ", ";
		}
		os << arr[arr.size()-1];
	}
	
	os << "]";
	return os;
}
// template<typename T, usize S>
// //                            what is this syntax?
// constexpr Buffer<T, S> to_buffer(T (&&arr)[S]) {
// 	return std::to_array<T, S>(arr);
// }