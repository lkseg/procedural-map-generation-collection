#include "util.h"
// Default is Max Heap
template<typename T, typename Comp = std::less<T>>
struct Heap {
    Array<T> data;
    void push(const T &val) {
        data.push_back(val);
        // [](T &l, T &r) -> bool {return l < r}
        std::push_heap(data.begin(), data.end(), Comp{});
    }
    T pop() {
        assert(!empty());
        std::pop_heap(data.begin(), data.end(), Comp{});
        T val = data.back();
        data.pop_back();
        return val;
    }
	const T &top() {
		assert(!empty());
		return data[0];
	}
	T &top_mut() {
		assert(!empty());
		return data[0];
	}
    void heapify() {
        std::make_heap(data.begin(), data.end(), Comp{});
    }
	// @return True if the element got removed
	bool erase(const T &value) {
		auto it = std::find(data.begin(), data.end(), value);
		if(it != data.end()) {
            data.erase(it);
            std::make_heap(data.begin(), data.end(), Comp{});
            return true;
        }
		return false;
	}
    void update(const T &old, const T &_new) {
        auto it = std::find(data.begin(), data.end(), old);
        if(it != data.end()) {
            *it = _new;
            std::make_heap(data.begin(), data.end(), Comp{});
            return;
        }
        assert(false);
    }
    bool empty() {
        return data.size() == 0;
    }
};