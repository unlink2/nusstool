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

  const usize img_len = (usize)img_header.w * (usize)img_header.h;
  buffer->len = img_len / 8 + (img_len % 8 ? 1 : 0);
  buffer->data = malloc(buffer->len);

  // length of a row of converted bmp1 data
  const usize buffer_row_len = buffer->len / img_header.h;

  memset(buffer->data, 0, buffer->len);

  if (img_header.bpp != 24) {
    return ERR_BMP_UNSUPPORTED_BPP;
  }

  const usize pixel_len = 3;
  const usize stride = (img_header.w * pixel_len) % 4;

  u8 *current = src.data + header.offset + img_len * pixel_len - 1;
  // now just loop over the 24bpp image
  for (usize y = 0; y < img_header.h; y++) {
    // obtain the current row in reverse order
    u8 *byte = buffer->data + buffer->len - 1 -
               (buffer_row_len * (img_header.h - y)) + buffer_row_len;
    usize bit = 0;

    // rows are aligned! pad now!
    current -= stride;
    for (usize x = 0; x < img_header.w; x++) {
      u32 pixel = 0;
      memcpy(&pixel, current, pixel_len);

      if (pixel != 0) {
        *byte = ((*byte) | ((u32)1 << (7 - bit)));
        printf("1");
      } else {
        printf("0");
      }
      printf("(%x) ", pixel);

      bit++;
      current -= pixel_len;
      if (bit == 8) {
        bit = 0;
        byte--;
      }
    }
    printf("\n");
  }

  buffer_free(&src);

  return OK;
}
