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

void buffer_pad_to(Buffer *buffer, const size len, const u8 val);
void buffer_pad_by(Buffer *buffer, const size len, const u8 val);

void buffer_inject(Buffer *buffer, const size loc, const u8 *data,
                   const size len);
Error buffer_inject_file(Buffer *buffer, size loc, FILE *file);

void buffer_set(Buffer *buffer, const size loc, const u8 val, const u8 len);

void buffer_resize(Buffer *buffer, const size new_len);

void buffer_free(Buffer *buffer);

#endif
