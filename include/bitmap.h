#ifndef BITMAP_H_
#define BITMAP_H_

#include "buffer.h"
#include "types.h"

/**
 * Subcommand that converts
 * regular bitmaps
 * to the bmp1 format that is used in-game
 * to store 1bpp assets.
 *
 * The bmp1 format is simply an array of
 * pixel data. each bit is 1 pixel.
 * There is no width or height information or color attached!
 * The origin is top left.
 * Each row is always padded to the full byte!
 */

typedef struct BitmapHeader {
  char magic[2]; // BM, BA...
  u32 size;
  u16 reserved_1;
  u32 reserved_2;
  u32 offset;
  // Array of 32 bit words
} BitmapHeader;

typedef struct BitmapImageHeader {
  u32 header_size;
  i32 w;
  i32 h;
  u16 planes;
  u16 bpp;
  u32 compression;
  u32 img_size;
  u32 xppm;
  u32 yppm;
  u32 total_colors;
  u32 important_colors;
} BitmapImageHeader;

Error bitmap_header_from_bytes(BitmapHeader *b, const u8 *data,
                               const usize len);

Error bitmap_image_header_from_bytes(BitmapImageHeader *b, const u8 *data,
                                     const usize len);

// TODO this currently only supports 24bpp bmp
// and does not really verify anything else!
// In general this will only work on bmps that are
// evenly divisible by 4 in width for now
// TODO handle size of bmp
Error bitmap_to_1bpp(Buffer *buffer);

#endif
