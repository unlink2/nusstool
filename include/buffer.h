#ifndef BUFFER_H_
#define BUFFER_H_

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

typedef struct Buffer { // NOLINT
  size len;
  u8 *data;
} Buffer;

void buffer_init(Buffer *buffer);
Error buffer_read(Buffer *buffer, FILE *file);
Error buffer_write(const Buffer *buffer, FILE *file);

void buffer_pad_to(Buffer *buffer, size loc, size len, u8 val);
void buffer_pad_by(Buffer *buffer, size loc, size len, u8 val);

void buffer_inject(Buffer *buffer, size loc, u8 *data, size len);
void buffer_inject_file(Buffer *buffer, size loc, FILE *file);

void buffer_resize(Buffer *buffer);

void buffer_free(Buffer *buffer);

#endif
