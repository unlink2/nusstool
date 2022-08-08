#include "bitmap.h"
#include <arpa/inet.h>
#include <string.h>

Bitmap bitmap_header_from(Buffer *buffer) {
  Bitmap b;
  b.magic[0] = (char)buffer->data[0];
  b.magic[1] = (char)buffer->data[1];

  memcpy(&b.size, buffer->data + 2, sizeof(u32));
  memcpy(&b.offset, buffer->data + 4 + 4 + 2, sizeof(u32));

  return b;
}

Error bitmap_to_1bpp(Buffer *buffer) {
  Buffer src;
  buffer_init(&src);
  src.data = buffer->data;
  src.len = buffer->len;

  buffer_init(buffer);
  buffer->data = malloc(64);

  // get header from src
  Bitmap header = bitmap_header_from(&src);
  printf("%c%c - size: %x; offset: %x\n", header.magic[0], header.magic[1],
         header.size, header.offset);
  for (usize i = header.offset; i < src.len; i++) {
  }

  buffer_free(&src);

  return OK;
}
