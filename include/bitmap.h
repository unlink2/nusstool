#ifndef BITMAP_H_
#define BITMAP_H_

#include "buffer.h"
#include "types.h"

typedef struct Bitmap {
  char magic[2]; // BM, BA...
  u32 size;
  u16 reserved_1;
  u32 reserved_2;
  u32 offset;
  // Array of 32 bit words
} Bitmap;

// TODO this currently only supports 24bpp bmp
// and does not really verify anything else!
// In general this will only work on bmps that are
// evenly divisible by 4 in width for now
Error bitmap_to_1bpp(Buffer *buffer);

#endif
