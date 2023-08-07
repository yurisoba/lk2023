#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static inline uintptr_t align_up(uintptr_t sz, size_t alignment)
{
	uintptr_t mask = alignment - 1;
	if ((alignment & mask) == 0) {  /* power of two? */
		return (sz + mask) & ~mask;
	}
	return (((sz + mask) / alignment) * alignment);
}

int main(void)
{
	assert(align_up(120, 4) == 120);
	assert(align_up(121, 4) == 124);
	assert(align_up(122, 4) == 124);
	assert(align_up(123, 4) == 124);
}
