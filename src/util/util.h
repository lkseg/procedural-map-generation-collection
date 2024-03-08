#ifndef UTIL_H
#define UTIL_H


#include <stdio.h>

#include <iostream>
#include "basic.h"
#include "chrono"
#include "type_traits"
#include "initializer_list"
#include <limits>
#include <tuple>
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>


inline void print() { std::cout<<'\n'<<std::flush; }
inline void printerr() { std::cout<<std::flush; }

template<class T, typename ...Args>
void print(const T &t, Args... args) {
	if constexpr(std::is_same<T, bool>::value) {
		if(t) std::cout<<"true";
		else std::cout<<"false";		
	} else {
		std::cout<<t;
	}
	std::cout << " ";
	print(args...);
}
template<typename T, typename ...Args>
void printerr(const T &t, Args... args) {
	std::cout<<t;
	print(args...);
}


/*
template<>
void print<bool>(const bool &v) {
	// << std::boolalpha
	 if(v) std::cout<<"true"; else std::cout<<"false";
}
*/
// Can be compiler dependent
#define xprintln(...) xprint(__VA_ARGS__)
#define println(...) print(__VA_ARGS__)




// actual modulo; '%' is division remainder in c++
template<typename T>
T mod(T a, T b) {
	static_assert(std::is_integral_v<T>);
	T rem = a % b;
	return (rem>=0)? rem : rem + b;
}

// template<>
// inline f64 mod(f64 a, f64 b) {
// 	f64 rem = std::fmod(a,b);
// 	return (rem>=0)? rem : rem + b;
// }
inline f64 fmod(f64 a, f64 b) {
	f64 rem = std::fmod(a,b);
	return (rem>=0)? rem : rem + b;
}

template<typename K>
using Maybe = std::optional<K>;

typedef std::string String;
using std::to_string;
using std::find;
using std::find_if;
using std::swap;
using std::move;

template<typename T>
inline String str(const T &v) {
	// std::ostringstream stream;
	// stream << v;
	// return stream.str();
	return to_string(v);
}

// Wrapper around std::vector with some debug utilities and *signed* sizes.
template<typename T>
struct Array {
	std::vector<T> data;
	typedef std::vector<T>::iterator Iterator;
	Array() {}
	explicit Array(isize _size): data(_size) {}		
	explicit Array(isize _size, T _value): data(_size, _value) {}	

	// Array(const std::vector<T> &vec): data(vec) {}	
	// Array(std::vector<T> &&vec): data(std::move(vec)) {}	
	// explicit Array(const Array<T> &other): data(other.data) {}	
	// explicit Array(Array<T> &&other): data(move(other.data)) {}	

	Array(std::initializer_list<T> l): data(l) {}
	
	inline T &operator[](isize i) {
		assert(0 <= i && i < (isize)data.size(), "array out of bounds for index " + std::to_string(i));
		return data[i];
	}
	inline const T &operator[](isize i) const {
		assert(0 <= i && i < (isize)data.size(), "array out of bounds for index " + std::to_string(i));
		return data[i];
	}
	inline isize size() const {
		return static_cast<isize>(data.size());
	}
	inline void resize(isize count) {
		assert(count >= 0);
		data.resize(count);
	}
	inline void reserve(isize count) {
		assert(count >= 0);
		data.reserve(count);
	}
	inline void clear() {
		data.clear();
	}
	inline Iterator begin() {
		return data.begin();
	}
	inline Iterator end() {
		return data.end();
	}
	// Not part of default std::vector.
	inline Iterator erase(isize idx) {
		return data.erase(data.begin()+idx);
	}
	inline Iterator erase(const Iterator &it) {
		return data.erase(it);
	}
	inline bool contains(const T &v) {
		return this->find(v) != data.end();
	}
	inline Iterator find(const T &v) {
		return std::find(data.begin(), data.end(), v);
	}
	// @return True if found and erased
	bool find_and_erase(const T &v) {
		auto it = std::find(data.begin(), data.end(), v);
		if(it != data.end()) {
			data.erase(it);
			return true;
		}
		return false;
	}
	inline void push_back(const T &v) {
		data.push_back(v);
	}
	inline void pop_back() {
		assert(this->size() > 0, "can't call pop_back on an empty array");
		data.pop_back();
	}
	inline T &back() {
		assert(this->size() > 0, "can't call back on an empty array");
		return data.back();
	}
	inline usize bytes_size() {
		return data.size() * usize(sizeof(T));
	}
};

template<typename T>
struct Array2D {
	std::vector<T> data = {};
	struct {
		i32 x = 0;
		i32 y = 0;		
	} size;
	Array2D() {}
	Array2D(i32 x, i32 y) {
		data.resize(x*y);
		size.x = x;
		size.y = y;
	}
	inline isize index(isize x, isize y) const {
		return y * size.x + x;
	}
	inline T &operator()(i32 x, i32 y) {
		assert(0 <= x && x < size.x && 0 <= y && y < size.y, "("+str(x) + ", " + str(y)+")");
		return data[index(x,y)];
	}
	inline const T &operator()(i32 x, i32 y) const {
		assert(0 <= x && x < size.x && 0 <= y && y < size.y, "("+str(x) + ", " + str(y)+")");
		return data[index(x,y)];
	}
	inline T &get(i32 x, i32 y) {
		assert(0 <= x && x < size.x && 0 <= y && y < size.y, "("+str(x) + ", " + str(y)+")");
		return data[index(x,y)];
	}
	
	inline Maybe<T> &get_maybe(i32 x, i32 y) {
		if(0 <= x && x < size.x && 0 <= y && y < size.y) {
			return data[index(x,y)];
		}
		return {};
	}

	T &operator[](isize idx) {
		assert(0 <= idx && idx < data.size());
		return data[idx];
	}
	void resize(i32 x, i32 y) {
		data.resize(x*y);
		size = {x, y};
	}
	void resize(i32 x, i32 y, const T &val) {
		data.resize(x*y);
		size = {x, y};
		fill(val);
	}
	inline isize count() {
		return data.size();
	}
	void fill(const T &val) {
		for(auto &it: data) it = val;
	}
	bool is_valid(i32 x, i32 y) {
		return 0 <= x && x < size.x && 0 <= y && y < size.y;
	}
	
};







template<typename T>
struct Flags {
	typedef std::underlying_type<T>::type Type;
	typename std::underlying_type<T>::type flags = 0;
	force_inline void set(const T &flag)   {this->flags |= Type(flag);}
	force_inline void unset(const T &flag) {this->flags &= ~Type(flag);}
	force_inline bool has(const T &flag)   {return this->flags & Type(flag);}	
};


template<typename T>
T clamp(const T &x, const T &a, const T &b) {
	return (x < a)? a : (x > b)? b : x;
}


// If windows.h gets ever included.
#undef max
#undef min
template<typename T>
inline T max(const T &a, const T &b) {
	return (a > b)? a : b;
}
template<typename T>
inline T max(const T &a, const T &b, const T &c) {
	return max(max(a, b), c);
}
template<typename T>
inline T max(const T &a, const T &b, const T &c, const T &d) {
	return max(max(a, b, c), d);
}
template<typename T>
usize argmax(T a, T b) {
	return (a > b)? 0 : 1;
}
template<typename T>
usize argmax(T a, T b, T c) {
	isize idx = argmax(a,b);
	if(idx == 0 && a > c) {
		return 0;
	}
	if(b > c) {
		return 1;
	}
	return 2;
}
template<typename T>
inline T min(const T &a, const T &b) {
	return (a < b)? a : b;
}
template<typename T>
inline T min(const T &a, const T &b, const T &c) {
	return min(min(a, b), c);
}
template<typename T>
inline T min(const T &a, const T &b, const T &c, const T &d) {
	return min(min(a, b, c), d);
}

#define swap_values(A, B) auto _D = A; A = B; B = _D
bool approx(f32, f32, f32 delta = 0.001);
bool approx(f64, f64, f64 delta = 0.001);

template<typename D, typename B> 
void assign_base(D &derived, B &base) {
	static_assert(std::is_base_of<B, D>::value);
	*((B *)&derived) = base;
}

extern u64 default_seed;
typedef std::chrono::time_point<std::chrono::system_clock> Chrono_Clock;
Chrono_Clock get_time();
f64 time_diff(Chrono_Clock start, Chrono_Clock end);



constexpr u8  U8_MAX  = std::numeric_limits<u8 >::max();
constexpr u16 U16_MAX = std::numeric_limits<u16>::max();
constexpr u32 U32_MAX = std::numeric_limits<u32>::max();
constexpr u64 U64_MAX = std::numeric_limits<u64>::max();

constexpr i8  I8_MAX  = std::numeric_limits<i8 >::max();
constexpr i16 I16_MAX = std::numeric_limits<i16>::max();
constexpr i32 I32_MAX = std::numeric_limits<i32>::max();
constexpr i64 I64_MAX = std::numeric_limits<i64>::max();

constexpr f32 F32_MAX = std::numeric_limits<f32>::max();
constexpr f64 F64_MAX = std::numeric_limits<f64>::max();

constexpr isize ISIZE_MAX = std::numeric_limits<isize>::max();
constexpr usize USIZE_MAX = std::numeric_limits<usize>::max();

constexpr u8  U8_MIN  = std::numeric_limits<u8 >::min();
constexpr u16 U16_MIN = std::numeric_limits<u16>::min();
constexpr u32 U32_MIN = std::numeric_limits<u32>::min();
constexpr u64 U64_MIN = std::numeric_limits<u64>::min();

constexpr i8  I8_MIN  = std::numeric_limits<i8 >::min();
constexpr i16 I16_MIN = std::numeric_limits<i16>::min();
constexpr i32 I32_MIN = std::numeric_limits<i32>::min();
constexpr i64 I64_MIN = std::numeric_limits<i64>::min();

// !!! lowest !!! instead of min for floating point numbers
constexpr f32 F32_MIN = std::numeric_limits<f32>::lowest();
constexpr f64 F64_MIN = std::numeric_limits<f64>::lowest();

constexpr isize ISIZE_MIN = std::numeric_limits<isize>::min();
constexpr usize USIZE_MIN = std::numeric_limits<usize>::min();


constexpr f32 F32_EPS = std::numeric_limits<f32>::epsilon();
constexpr f64 F64_EPS = std::numeric_limits<f64>::epsilon();

template<typename T>
constexpr T num_max() {
	return std::numeric_limits<T>::max();
}
template<typename T>
constexpr T num_min() {
	return std::numeric_limits<T>::max();
}

template<typename T>
std::ostream &operator<<(std::ostream &os, const Array<T> &arr) {
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
template<typename T>
std::ostream &operator<<(std::ostream &os, const Array2D<T> &arr) {
	os << '|' << arr.size.x << ", " << arr.size.y << "| [\n";	
	for(auto y = 0; y < arr.size.y; ++y) {
		for(auto x = 0; x < arr.size.x; ++x) {
			os << arr(x, y) << ", ";
		}
		os << '\n';
	}	
	os << "]";
	return os;
}


#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <tuple>

#include <queue>

template<typename K>
using Set = std::unordered_set<K>;

template<typename K>
using Ordered_Set = std::set<K>;

template<typename K, typename V>
using Map = std::unordered_map<K, V>;

template<typename K, typename V>
using Ordered_Map = std::map<K, V>;

template<typename K, typename V>
using Ordered_Multi_Map = std::multimap<K, V>;

template<typename K>
using Tuple = std::tuple<K>;

template<typename K, typename V>
using Pair = std::pair<K, V>;



template<typename K, typename Cont = std::vector<K>, typename Comp = std::less<typename Cont::value_type>>
using Priority_Queue = std::priority_queue<K, Cont, Comp>;



template<typename K, typename V>
Array<V> to_array(const Map<K, V> &map) {
	Array<V> arr(map.size());
	isize i = 0;
	for(auto &[key, val]: map) {
		arr[i++] = val;
	}
	return arr;
}

#include <sstream>
template<typename T>
inline String ostr(const T &v) {
	std::ostringstream stream;
	stream << v;
	return stream.str();
}

template<typename K, typename V>
struct std::hash<Pair<K, V>> {
	// @todo Use primes or something
	std::size_t operator()(const Pair<K, V> &pair) const {
		return std::size_t(pair.first << 1) | std::size_t(pair.second);
	}
};

template<typename T>
std::ostream &operator<<(std::ostream &os, const Set<T> &arr) {
	os << "Set {";
	
	for(auto &v: arr) {
		os << v << ", ";
	}
		
	os << "}";
	return os;
}

// typedef std::string StdString;
#endif // UTIL_H
