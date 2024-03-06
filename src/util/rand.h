#include "basic.h"
#include <random>
// extern std::mt19937 g_random_engine;

u64 get_random_seed();
void set_global_random_engine_seed(u64);
// [start, end]
f64 randf_range(f64, f64);
f64 randf();
i64 randi_range(i64, i64);
i32 randi32_range(i32, i32);