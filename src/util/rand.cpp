#include "rand.h"


std::mt19937 g_random_engine(0);

u64 get_random_seed() {
	return std::random_device{}();
}
void set_global_random_engine_seed(u64 seed) {
	if(seed == 0) {
		seed = std::random_device{}();
	}
	srand(static_cast<unsigned int>(seed));
	g_random_engine = std::mt19937(seed);
}
f64 randf_range(f64 start, f64 end) {
	// std::random_device{}()
	std::uniform_real_distribution<f64> r_range(start, end);
	return r_range(g_random_engine);
}

i64 randi_range(i64 start, i64 end) {
	std::uniform_int_distribution<i64> r_range(start, end);
	return r_range(g_random_engine);
}
i32 randi32_range(i32 start, i32 end) {
	std::uniform_int_distribution<i32> r_range(start, end);
	return r_range(g_random_engine);
}
f64 randf() {
	return f64(rand())/f64(RAND_MAX);
}