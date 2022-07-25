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

#endif
