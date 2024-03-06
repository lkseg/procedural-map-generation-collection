#pragma once
#include "basic.h"
#include <type_traits>


template<typename T>
struct Slice {
    T *data = nullptr;
    isize count = 0;

    inline T &operator[](isize idx) {
        assert(0 <= idx && idx < count, "Out of bounds for " + to_string(idx));
        return data[idx];
    }  
    inline T *begin() {
        return data;
    }
    inline T *end() {
        return data + count;
    }
    inline isize size() const {
        return count;
    }
};

// invoke_result, result_of
// A must define T &operator[]
// python: a[first:last]
template<typename A, typename T = std::remove_reference_t<decltype(std::declval<Array<i32>>()[0])>>
Slice<T> slice(A &a, isize first, isize last) {
    isize count = last-first;
    if(count <= 0) return {};    
    T *p = &a[0];
    return Slice<T>{p+first, count};
}
