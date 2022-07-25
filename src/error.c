#include "error.h"

void error_fprint(FILE *file, Error error) {
  switch (error) {
  case OK:
    break;
  case ERR_CRC_NOT_ENOUGH_DATA:
    fprintf(file, "CRC not enough data\n");
    break;
  case ERR_HEADER_NOT_ENOUGH_DATA:
    fprintf(file, "Header not enough data\n");
    break;
  case ERR_READ:
    fprintf(file, "IO Read Error\n");
    break;
  case ERR_WRITE:
    fprintf(file, "IO Write Error\n");
    break;
  default:
    fprintf(file, "Unknown error\n");
    break;
  }
}
