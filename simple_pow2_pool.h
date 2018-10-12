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

#ifndef FILE_simple_pow2_pool_h_INCLUDED
#define FILE_simple_pow2_pool_h_INCLUDED

#include <assert.h>
#include <stddef.h>

#ifdef USE_STDCXX_LIBRARY
#include <new>
#include <utility>
#endif


namespace impl {

class simple_pow2_pool_base
{
protected:
	struct FreeNode
	{
		struct FreeNode *m_next;
	};

	static size_t constexpr constexpr_pow2(int n)
	{
		return size_t(1) << n;
	}

	static bool constexpr constexpr_is_pow2(size_t n)
	{
		return !(n & (n - 1));
	}

	simple_pow2_pool_base(void *pool, size_t size)
		: m_cur(pool)
		, m_left(size)
	{}

	static void add_to_list(FreeNode *&head, void *ptr)
	{
		auto node = static_cast<FreeNode *>(ptr);

		node->m_next = head;
		head = node;
	}

	static void *remove_from_list(FreeNode *&head)
	{
		void *ret = head;

		head = head->m_next;
		return ret;
	}

	// Allocate a new object of a given size
	// when there are no free slots available.
	void *extend(size_t alloc_size, size_t align_size, FreeNode **);

	// Special case when POW_MIN == POW_ALIGN.
	void *extend(size_t size)
	{
		void *ret = m_cur;
		auto cur = static_cast<char *>(m_cur);

		if (m_left < size)
			out_of_memory();

		m_cur = cur + size;
		m_left -= size;

		return ret;
	}

	[[noreturn]] static void out_of_memory();

	void *m_cur;
	size_t m_left;
};

} // namespace impl


// Simple pool for small power 2 sized objects. Max object size is 2^9.
//
// For instance, assuming that cacheline is 2^6 bytes and we want to
// allocate objects of size 2^5 (half-cacheline), 2^6 (one cacheline)
// and 2^7 (two cachelines) and we want a cacheline alignment for
// objects that are multiple of a cacheline, the following object can
// be instantiated: simple_pow2_pool<5,7,6>.
//
template<int POW_MIN, int POW_MAX = POW_MIN, int POW_ALIGN = POW_MIN>
class simple_pow2_pool : impl::simple_pow2_pool_base
{
	static_assert(POW_MIN > 0 && POW_MAX <= 9 &&
	    POW_MIN <= POW_MAX && POW_ALIGN >= POW_MIN && POW_ALIGN <= POW_MAX,
	    "template argument is out of range");
	static_assert(constexpr_pow2(POW_MIN) >= sizeof(FreeNode),
	    "not enough space to hide a pointer inside a free slot");

	static bool constexpr constexpr_good_size(size_t n)
	{
		return constexpr_is_pow2(n) &&
		    n >= constexpr_pow2(POW_MIN) &&
		    n <= constexpr_pow2(POW_MAX);
	}

	FreeNode *m_freelists[POW_MAX - POW_MIN + 1] {};
public:
	simple_pow2_pool(simple_pow2_pool const &) = delete;
	void operator=(simple_pow2_pool const &) = delete;

	simple_pow2_pool(void *pool, size_t size)
		: impl::simple_pow2_pool_base(pool, size)
	{}

	template<class T>
	void *alloc()
	{
		static_assert(constexpr_good_size(sizeof(T)), "bad type");

		for (auto i = POW_MIN; i <= POW_MAX; i++) {
			size_t const alloc_size = constexpr_pow2(i);
			size_t constexpr align_size = constexpr_pow2(POW_ALIGN);

			if (alloc_size != sizeof(T))
				continue;

			auto *&freelist = m_freelists[i - POW_MIN];

			if (freelist) {
				return this->remove_from_list(freelist);
			} else if (POW_MIN == POW_ALIGN) {
				return this->extend(alloc_size);
			} else {
				return this->extend(alloc_size,
				    align_size, &freelist);
			}
		}

		assert(false); // [[unreachable]]
		return nullptr;
	}

#ifdef USE_STDCXX_LIBRARY
	template<class T, class ... Args>
	T *emplace_alloc(Args && ... args)
	{
		return new(alloc<T>()) T(std::forward<Args>(args) ...);
	}
#endif

	template<class T>
	void dealloc(void *ptr) noexcept
	{
		static_assert(constexpr_good_size(sizeof(T)), "bad type");

		for (auto i = POW_MIN; i <= POW_MAX; i++) {
			size_t const size = constexpr_pow2(i);

			if (size != sizeof(T))
				continue;

			return this->add_to_list(m_freelists[i - POW_MIN], ptr);
		}

		assert(false); // [[unreachable]]
	}
};

#endif // FILE_simple_pow2_pool_h_INCLUDED
