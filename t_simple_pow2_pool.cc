#include "simple_pow2_pool.h"

#include <stdio.h>
#include <stdlib.h>

static char s_pool[4096];

int main(int argc, char *argv[])
{
	size_t counter = argc > 1 ? strtod(argv[1], nullptr) : 1e8;
	size_t seed = argc > 2 ? strtod(argv[2], nullptr) : counter;

	simple_pow2_pool<3,5,4> pool(s_pool, sizeof(s_pool));
	size_t constexpr num_raw_objects = sizeof(s_pool) / 8;
	void *raw_objects[num_raw_objects] {};

	srandom(seed);

	for (size_t i = 0; i < counter; i++) {
		long r = random();
		auto &raw = raw_objects[r % num_raw_objects];
		auto *ptr = static_cast<size_t *>(raw);

		if (ptr) {
			//fprintf(stderr, "dealloc %p %zu %c %zu\n",
			//    raw, *ptr, *ptr < i ? '<' : '>', i);
			assert(*ptr < i);
			pool.dealloc<size_t>(ptr);
			raw = nullptr;
		} else {
			raw = pool.alloc<size_t>();
			ptr = static_cast<size_t *>(raw);
			*ptr = i;
			//fprintf(stderr, "  alloc %p %zu\n", raw, i);
		}
	}

	for (size_t i = 0; i < num_raw_objects; i++) {
		if (raw_objects[i])
			pool.dealloc<size_t>(raw_objects[i]);
	}

	for (size_t i = 0; i < num_raw_objects; i++)
		raw_objects[i] = pool.alloc<size_t>();

	for (size_t i = 0; i < num_raw_objects; i++)
		pool.dealloc<size_t>(raw_objects[i]);
}
