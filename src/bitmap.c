#include "bitmap.h"
#include <arpa/inet.h>
#include <string.h>

Error bitmap_header_from_bytes(BitmapHeader *b, const u8 *data,
                               const usize len) {
  memset(b, 0, sizeof(BitmapHeader));
  b->magic[0] = (char)data[0];
  b->magic[1] = (char)data[1];

  memcpy(&b->size, data + 2, sizeof(u32));
  memcpy(&b->offset, data + 4 + 4 + 2, sizeof(u32));

  return OK;
}

Error bitmap_image_header_from_bytes(BitmapImageHeader *b, const u8 *data,
                                     const usize len) {
  memset(b, 0, sizeof(BitmapImageHeader));

  memcpy(b, data + 14, sizeof(BitmapImageHeader));

  return OK;
}

Error bitmap_to_1bpp(Buffer *buffer) {
  Buffer src;
  buffer_init(&src);
  src.data = buffer->data;
  src.len = buffer->len;

  buffer_init(buffer);

  // get header from src
  BitmapHeader header;
  BitmapImageHeader img_header;
  if (bitmap_header_from_bytes(&header, src.data, src.len) ||
      bitmap_image_header_from_bytes(&img_header, src.data, src.len)) {
    return ERR_BMP_HEADER;
  }

  buffer->len = ((usize)img_header.w * (usize)img_header.h) / 8;
  buffer->data = malloc(buffer->len);
  memset(buffer->data, 0, buffer->len);

  if (img_header.bpp != 24) {
    return ERR_BMP_UNSUPPORTED_BPP;
  }

  const usize pixel_len = 3;
  const usize stride = (img_header.w * pixel_len) % 4;

  u8 *current = src.data + header.offset;
  u8 *byte = buffer->data; // currnet byte to write to
  usize bit = 8;
  // now just loop over the 24bpp image
  for (usize y = 0; y < img_header.h; y++) {
    for (usize x = 0; x < img_header.w; x++) {
      u32 pixel = 0;
      memcpy(&pixel, current, pixel_len);

      if (pixel != 0) {
        *byte = ((*byte) | ((u32)1 << (bit - 1)));
      }

      bit--;
      current += pixel_len;
      if (bit == 0) {
        bit = 8;
        byte++;
      }
    }
    // rows are aligned! pad now!
    current += stride;
  }

  // lastly invert rows
  for (usize i = 0; i < buffer->len / 2; i++) {
    u8 tmp = buffer->data[buffer->len - 1 - i];
    buffer->data[buffer->len - 1 - i] = buffer->data[i];
    buffer->data[i] = tmp;
  }

  buffer_free(&src);

  return OK;
}
