#ifndef MACROS_H_
#define MACROS_H_

#define MIN(a, b) a < b ? a : b
#define MAX(a, b) a > b ? a : b

#ifdef TEST
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#endif

#ifdef DEBUG_MALLOC

/**
 * This is an example of how to debug malloc
 * Simply macro alloc, record allocs and free in some way and
 * maybe over-allocate and put a magic number at the end of the allocation
 * to detect out of bounds writes!
 */

#include "types.h"

#define malloc(n) debug_malloc(n, __FILE__, __LINE__)
#define free(n) debug_free(n, __FILE__, __LINE__)

void *debug_malloc(usize size, char *file, usize line);
void debug_free(void *ptr, char *file, usize line);

#endif

#endif
