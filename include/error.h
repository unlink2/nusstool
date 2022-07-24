#ifndef ERROR_H_
#define ERROR_H_

typedef enum Error {
  OK,
  ERR_CRC_NOT_ENOUGH_DATA,
  ERR_HEADER_NOT_ENOUGH_DATA,
  ERR_READ_ERROR,
  ERR_WRITE_ERROR,
} Error;

#endif
