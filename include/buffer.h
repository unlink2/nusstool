#ifndef BUFFER_H_
#define BUFFER_H_

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

typedef struct Buffer { // NOLINT
  usize len;
  u8 *data;
} Buffer;

void buffer_init(Buffer *buffer);
Error buffer_read(Buffer *buffer, FILE *file);
Error buffer_write(const Buffer *buffer, FILE *file);
Error buffer_write_array(const Buffer *buffer, FILE *file, char *name,
                         char *type);
Error buffer_write_text_array(const Buffer *buffer, FILE *file, char *name,
                              char *type);

void buffer_pad_to(Buffer *buffer, const usize len, const u8 val);
void buffer_pad_by(Buffer *buffer, const usize len, const u8 val);

void buffer_inject(Buffer *buffer, const usize loc, const u8 *data,
                   const usize len);
Error buffer_inject_file(Buffer *buffer, usize loc, FILE *file);

void buffer_set(Buffer *buffer, const usize loc, const u8 val, const u8 len);

void buffer_resize(Buffer *buffer, const usize new_len);

void buffer_free(Buffer *buffer);

#endif
