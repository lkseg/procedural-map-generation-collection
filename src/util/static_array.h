#include "basic.h"

template<typename T, isize capacity>
struct Static_Array {
	T data[capacity];
	isize count = 0;
	Static_Array() {}
	Static_Array(std::initializer_list<T> list) {
		assert(list.size() <= capacity);
		this->count = list.size();
		auto begin = list.begin();
		ForRange(i, 0, count) {
			data[i] = *(begin+i);
		}
	}
	inline T &operator[](isize index) {
		assert(0<=index && index<count, "bad index");
		return data[index];		
	}
	inline void add(const T &val) {
		if(capacity > count) {
			data[count] = val;
			count += 1;
		} else {
			assert(false, "overflowing static array");
		}
	}
	inline void add(const T &&val) {
		add(val);
	}	
	void resize(isize a_size) {
		if (a_size < 0) {
			count = 0;
			return;
		}
		if(a_size > capacity) {
			count = capacity;
			return;
		}
		count = a_size;
		return;
	}
	Static_Array &operator=(const Static_Array<T,capacity> &from) = delete;

	// Iterator
	struct _Value_Index {
		T &v;
		isize i;
	};
	struct Iterator {
		T *data;
		isize index = 0;
		force_inline Iterator(T *_data, isize _index): data(_data), index(_index){}
		force_inline _Value_Index operator*() {return {*data, index};}
		force_inline Iterator &operator++() {
			index += 1;
			this->data += 1;
			return *this;
		}
		force_inline bool operator!=(const Iterator &iter) {return this->index != iter.index;}
	};
	force_inline Iterator begin() {return Iterator(&this->data[0], 0);}
	force_inline Iterator end()   {return Iterator(&this->data[this->count], this->count);}
};
template<typename T, isize capacity>
void copy(Static_Array<T,capacity> &to, Static_Array<T,capacity> &from) {
	to.count = from.count;
	memcpy(to.data, from.data, capacity);
}	