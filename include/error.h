#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>

typedef enum Error {
  OK = 0,
  ERR_CRC_NOT_ENOUGH_DATA,
  ERR_HEADER_NOT_ENOUGH_DATA,
  ERR_READ,
  ERR_WRITE,
  ERR_NUS_USB,
  ERR_BMP_BAD_COLOR
} Error;

void error_fprint(FILE *file, Error error);

// TODO instead of returning errors,
// return structs that are lead with the
// error header - this will avoid out params
typedef struct ErrorHeader {
  Error error;
} ErrorHeader;

#endif
