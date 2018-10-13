// Copyright (C) 2018 XMM SWAP LTD

#include "simple_pow2_pool.h"

#include <stdlib.h>

static char s_pool[4096];

int main(int argc, char *argv[])
{
	size_t counter = argc > 1 ? strtod(argv[1], nullptr) : 1e8;
	size_t seed = argc > 2 ? strtod(argv[2], nullptr) : counter;

	simple_pow2_pool<3,6,5> pool(s_pool, sizeof(s_pool));
	size_t constexpr num_raw_objects = 31; // up to 32 size_t[8]
	void *raw_objects[num_raw_objects] {};
	unsigned short raw_sizes[num_raw_objects] {};
	size_t values[num_raw_objects];

	srandom(seed);

	for (size_t i = 0; i < counter; i++) {
		size_t r = random();
		size_t n = r % num_raw_objects;
		auto *ptr = static_cast<size_t *>(raw_objects[n]);

		assert((ptr == nullptr) == (raw_sizes[n] == 0));

		if (ptr) {
			assert(ptr[0] < i);
			assert(ptr[0] == values[n]);

			switch (raw_sizes[n]) {
			case 1:
				pool.dealloc<size_t[1]>(ptr);
				break;
			case 2:
				pool.dealloc<size_t[2]>(ptr);
				break;
			case 4:
				pool.dealloc<size_t[4]>(ptr);
				break;
			case 8:
				pool.dealloc<size_t[8]>(ptr);
				break;
			default:
				assert(false);
			}

			raw_objects[n] = nullptr;
			raw_sizes[n] = 0;
		} else {
			switch ((r >> 19) & 3) {
			case 0:
				raw_objects[n] = pool.alloc<size_t[1]>();
				raw_sizes[n] = 1;
				break;
			case 1:
				raw_objects[n] = pool.alloc<size_t[2]>();
				raw_sizes[n] = 2;
				break;
			case 2:
				raw_objects[n] = pool.alloc<size_t[4]>();
				raw_sizes[n] = 4;
				break;
			case 3:
				raw_objects[n] = pool.alloc<size_t[8]>();
				raw_sizes[n] = 8;
				break;
			}

			ptr = static_cast<size_t *>(raw_objects[n]);
			ptr[0] = i;
			values[n] = i;
		}
	}

	// Deallocate all.
	for (size_t i = 0; i < num_raw_objects; i++) {
		switch (raw_sizes[i]) {
		case 0:
			assert(raw_objects[i] == nullptr);
			break;
		case 1:
			pool.dealloc<size_t[1]>(raw_objects[i]);
			break;
		case 2:
			pool.dealloc<size_t[2]>(raw_objects[i]);
			break;
		case 4:
			pool.dealloc<size_t[4]>(raw_objects[i]);
			break;
		case 8:
			pool.dealloc<size_t[8]>(raw_objects[i]);
			break;
		default:
			assert(false);
		}
	}

	for (size_t n = 0; n < num_raw_objects; n++)
		raw_objects[n] = pool.alloc<size_t[1]>();
	for (size_t n = 0; n < num_raw_objects; n++)
		pool.dealloc<size_t[1]>(raw_objects[n]);

	for (size_t n = 0; n < num_raw_objects; n++)
		raw_objects[n] = pool.alloc<size_t[2]>();
	for (size_t n = 0; n < num_raw_objects; n++)
		pool.dealloc<size_t[2]>(raw_objects[n]);

	for (size_t n = 0; n < num_raw_objects; n++)
		raw_objects[n] = pool.alloc<size_t[4]>();
	for (size_t n = 0; n < num_raw_objects; n++)
		pool.dealloc<size_t[4]>(raw_objects[n]);

	for (size_t n = 0; n < num_raw_objects; n++)
		raw_objects[n] = pool.alloc<size_t[8]>();
	for (size_t n = 0; n < num_raw_objects; n++)
		pool.dealloc<size_t[8]>(raw_objects[n]);
}
