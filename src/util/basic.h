#ifndef BASIC_H
#define BASIC_H
#include <cstddef> // ptrdiff_t
#include <stdint.h>
#include <stdlib.h>

#include <cstring>
#include <string>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;


typedef uint8_t  byte;

typedef size_t    usize;
typedef ptrdiff_t isize;

// --------------------------------------------------------------------------------------------------------------------------------------------------------
// https://github.com/gingerBill/gb/blob/master/gb.h
// Public Domain
template <typename T> struct _RemoveReference       { typedef T Type; };
template <typename T> struct _RemoveReference<T &>  { typedef T Type; };
template <typename T> struct _RemoveReference<T &&> { typedef T Type; };

template <typename T> inline T &&__forward(typename _RemoveReference<T>::Type &t)  { return static_cast<T &&>(t); }
template <typename T> inline T &&__forward(typename _RemoveReference<T>::Type &&t) { return static_cast<T &&>(t); }
template <typename T> inline T &&__move   (T &&t)                                   { return static_cast<typename _RemoveReference<T>::Type &&>(t); }
template <typename F>
struct _privDefer {
	F f;
	_privDefer(F &&f) : f(__forward<F>(f)) {}
	~_privDefer() { f(); }
};
template <typename F> _privDefer<F> _a_defer_func(F &&f) { return _privDefer<F>(__forward<F>(f)); }

#define __DEFER_1(x, y) x##y
#define __DEFER_2(x, y) __DEFER_1(x, y)
#define __DEFER_3(x)    __DEFER_2(x, __COUNTER__)
#define defer(code)      auto __DEFER_3(_defer_) = _a_defer_func([&]()->void{code;})
// --------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
	#define MSVC
	#define force_inline __forceinline
	#define __FUNCTION_SIG__ __FUNCSIG__
#elif __GNUC__
	#define GCC
	#define force_inline __attribute__((always_inline))
	#define __FUNCTION_SIG__ __PRETTY_FUNCTION__
#else
	#define force_inline inline
#endif

// This is fine for both gcc and msvc.
// It forces the struct to be packed tightly which makes it easy to serialize and hash
// sadly this style is forced by msvc.
#define force_packed(STRUCT) _Pragma("pack(push, 1)") STRUCT _Pragma("pack(pop)")

#define force_packed_start _Pragma("pack(push, 1)")
#define force_packed_end   _Pragma("pack(pop)")

// gcc specific
// __attribute__ ((packed))
#if !defined GCC && !defined MSVC
	static_assert(false);
#endif


void _internal_crash(const std::string &file_name, int line, const std::string &msg = "");

#define release_assert(COND, ...) if(!(COND)) {_internal_crash(__FILE__, __LINE__, ##__VA_ARGS__); }
// Debug macros
// Maybe tie it to NDebug?
#ifdef XDEBUG
#define assert(COND, ...) if(!(COND)) {_internal_crash(__FILE__, __LINE__, ##__VA_ARGS__); }
#define xassert assert
#define if_debug if constexpr(true)
#else
#define assert(A, ...)   ((void)0)
#define if_debug if constexpr(false)
#endif

#define panic(...) {_internal_crash(__FILE__, __LINE__, ##__VA_ARGS__); }


// python-like: for i in range(0, n); should be used when the user doesn't care for the types (wants default types)

#define ForRange(I, S, N) for(typename std::remove_const<decltype(N)>::type I = S; I < N; ++I)
#define For(IT, IDX, A) for(auto [IT, IDX] = std::tuple{A.begin(), 0}; IDX < A.size(); ++IDX, ++IT)




// #include <new>
/* 

inline void *mem_alloc(isize size) {
	assert(size >= 0);
	return malloc(size);
}
inline void *mem_realloc(void *ptr, isize size) {
	assert(size >= 0);
	return realloc(ptr, size);
}
inline void mem_free(void *ptr) {
	free(ptr);
}


template<typename T>
inline T *mem_alloc(isize count = 1) {
	T *ptr = (T *)mem_alloc(count * sizeof(T));
	return ptr;
}

// allocates a single object and uses its default constructor
template<typename T>
inline T* mem_new() {
	void *ptr = mem_alloc(1 * sizeof(T));
	// placement new
	T *obj = new(ptr) T();
	return obj;
} */

#endif // BASIC_H