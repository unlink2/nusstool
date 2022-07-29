#ifndef NUSHEADER_H_
#define NUSHEADER_H_

#include "buffer.h"
#include "error.h"
#include "types.h"
#include <stdio.h>

#define NUS_HEADER_SIZE 0x40
#define NUS_CRC_START 0x1000
#define NUS_CRC_LEN 0x100000
#define NUS_TITLE_LEN 0x14
#define NUS_CRC_END NUS_CRC_START + NUS_CRC_LEN

typedef struct NusCrc { // NOLINT
  u32 crc1;
  u32 crc2;
} NusCrc;

typedef struct NusHeader { // NOLINT
  u32 cfg_flags;
  u32 clck_rate;
  u32 boot_addr;
  u32 lu_ver;

  NusCrc crc;
  u8 reserved_1[0x08];

  char title[NUS_TITLE_LEN];
  u8 reserved_2[0x07]; // NOLINT
  char category;
  char unique[2];
  char destination;
  u8 version;
} NusHeader;

// Creates enough space in the buffer for a header
// and shifts the remaining buffer back
void nus_add_header(Buffer *buffer);

// Sets the header and calculates the crc
// This function assumes the buffer already has enough room for a header
void nus_set_header(Buffer *buffer, NusHeader *header);

/**
 * Format print the nus header
 */
void nus_fprint(FILE *file, const NusHeader *header);

void nus_init(NusHeader *header);

Error nus_from_bytes(NusHeader *header, const u8 *data, const usize len);
void nus_to_bytes(NusHeader *header, u8 *result);

Error nus_crc(NusHeader *header, const u8 *data, const usize len);

#ifdef TEST

#include "macros.h"

void test_crc_fail(void **state);

void test_crc(void **state);

#endif

#endif
