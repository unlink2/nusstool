
/**
 * When built without test
 */
#ifndef TEST

/// only use main if binary
#if TYPE == bin

#include <stdio.h>

int main(int argc, char **argv) {
  printf("Hello world!\n");
  return 0;
}

#endif
#endif

/**
 * When built with test
 */
#ifdef TEST

#include "macros.h"
#include "example.h"

int main(int argc, char **argv) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_is_zero),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif
