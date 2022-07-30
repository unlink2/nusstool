#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>

typedef enum Error {
  OK = 0,
  ERR_CRC_NOT_ENOUGH_DATA,
  ERR_HEADER_NOT_ENOUGH_DATA,
  ERR_READ,
  ERR_WRITE,
  ERR_NUS_USB
} Error;

void error_fprint(FILE *file, Error error);

#endif
