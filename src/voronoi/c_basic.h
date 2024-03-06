#ifndef C_BASIC_H
#define C_BASIC_H

#define declare_carray(T)\
typedef struct {\
    T *data;\
    int count;\
	int capacity;\
} T##_CArray;\

#define define_carray(T)\
T##_CArray new_##T##_carray(int count, int capacity) {\
    T##_CArray arr;\
    arr.count = count;\
    arr.capacity = capacity;\
    arr.data = (T *)malloc(sizeof(T) * capacity);\
    return arr;\
}\
inline bool T##_carray_reserve(T##_CArray *self, int a_capacity) {\
    if (a_capacity < 0) {\
        a_capacity = 0;\
    }\
    if(self->capacity >= a_capacity) {\
        return true;\
    }\
    void *ret = (T *)realloc(self->data, sizeof(T) * a_capacity);\
    if(ret == NULL) {\
        return false;\
    }\
    self->data = (T *)ret;\
    self->capacity = a_capacity;\
    return true;\
}\
bool T##_carray_resize(T##_CArray *self, int a_size) {\
    if (a_size < 0) {\
        self->count = 0;\
        return true;\
    }\
    if(a_size > self->capacity) {\
        T##_carray_reserve(self, a_size);\
        self->count = a_size;\
        return true;\
    }\
    self->count = a_size;\
    return true;\
}\
void T##_carray_add(T##_CArray *self, T *val) {\
    if(self->capacity > self->count) {\
        self->data[self->count] = *val;\
        self->count += 1;\
    } else {\
        bool ret = T##_carray_reserve(self, (self->capacity+1)*2);\
        if(!ret) return;\
        self->data[self->count] = *val;\
        self->count += 1;\
    }\
}\

#endif // C_BASIC_H