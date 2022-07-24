#include "example.h"

int is_zero() { return 0; }

#ifdef TEST

#include "macros.h"

void test_is_zero(void **state) { assert_false(is_zero()); }

#endif
