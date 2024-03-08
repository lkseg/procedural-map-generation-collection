#include "util.h"

u64 alloc_count = 0;
u64 free_count = 0;

/* void *mem_zero_alloc(isize count, isize size) {
	assert(count >= 0);
	in_debug(if(count > 0) alloc_count += 1);
	return calloc(count, size);
}

void *mem_zero_realloc(void *ptr, isize old_count, isize count, isize size) {
	assert(count>=old_count && size>=0 && count>=0);
	void *d = mem_realloc(ptr, count, size);

	byte *start = ((byte*)d) + old_count*size;
	// added byte count
	isize added_size = (count - old_count)*size;

	assert(start+added_size == ((byte*)d)+count*size);
	memset(start, 0, added_size);
	return d;
}
 */
void _internal_crash(const std::string &file_name, int line, const std::string &msg) {
	println("\n-----------CRASH-----------");
	println("error: ", msg);
	println("file:  ", file_name);
	println("line:  ", line);
	println("---------------------------\n");
 	abort();
}




bool approx(f32 a, f32 b, f32 delta) {
	return abs(a-b) < delta;
}
bool approx(f64 a, f64 b, f64 delta) {
	return abs(a-b) < delta;
}



// f64 randf_range(f64 start, f64 end) {
// 	static std::default_random_engine r_engine(time(0));
// 	std::uniform_real_distribution<f64> r_range(start, end);
// 	return r_range(r_engine);
// }
// u64 default_seed = 21382;
// u64 default_seed = std::random_device{}();



// #include <chrono>

Chrono_Clock get_time() {
	return std::chrono::system_clock::now();
}
f64 time_diff(Chrono_Clock start, Chrono_Clock end) {
	return std::chrono::duration<f64>(end-start).count();
}
