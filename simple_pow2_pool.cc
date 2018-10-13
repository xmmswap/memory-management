/*
 * Copyright (C) 2018 XMM SWAP LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * [ MIT license: http://www.opensource.org/licenses/mit-license.php ]
 */

#include "simple_pow2_pool.h"

#ifdef USE_STDCXX_LIBRARY
#include <exception>
#include <stdexcept>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

void
impl::simple_pow2_pool_base::out_of_memory()
{
#ifdef USE_STDCXX_LIBRARY
	throw std::bad_alloc();
#else
	fprintf(stderr, "simple_pow2_pool: out of memory\n");
	abort();
#endif
}

void *
impl::simple_pow2_pool_base::extend(size_t alloc_size, size_t align_size,
    impl::simple_pow2_pool_base::FreeNode **freelists)
{
	auto cur = static_cast<char *>(m_cur);
	void *ret = extend(alloc_size < align_size ? align_size : alloc_size);

	for (size_t i = 0; alloc_size < align_size; alloc_size *= 2, i++)
		add_to_list(freelists[i], cur + alloc_size);

	return ret;
}
