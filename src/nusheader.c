#include "nusheader.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

void nus_add_header(Buffer *buffer) {
  usize old_len = buffer->len;
  u8 *old_data = buffer->data;

  buffer->len += NUS_HEADER_SIZE;
  buffer->data = malloc(buffer->len);

  // this is different from the regular buffer resize function
  memset(buffer->data, 0, NUS_HEADER_SIZE);
  memcpy(buffer->data + NUS_HEADER_SIZE, old_data, old_len);

  free(old_data);
}

void nus_set_header(Buffer *buffer, NusHeader *header) {
  nus_crc(header, buffer->data, buffer->len);
  u8 header_bytes[NUS_HEADER_SIZE];
  nus_to_bytes(header, header_bytes);
  memcpy(buffer->data, header_bytes, NUS_HEADER_SIZE);
}

void nus_fprint(FILE *file, const NusHeader *header) {
  fprintf(file, "cfg flags: %x\n", header->cfg_flags);
  fprintf(file, "clock rate: %x\n", header->clck_rate);
  fprintf(file, "lu version: %x\n", header->lu_ver);
  fprintf(file, "boot address: %x\n", header->boot_addr);
  fprintf(file, "crc: (%x - %x)\n", header->crc.crc1, header->crc.crc2);

  fprintf(file, "title: ");
  // because the title may not be \0 terminated we need to
  // make sure to avoid an overrun
  for (usize i = 0; i < NUS_TITLE_LEN && header->title[i] != '\0'; i++) {
    fprintf(file, "%c", header->title[i]);
  }
  fprintf(file, "\n");

  fprintf(file, "category: %c\n", header->category);
  fprintf(file, "uid: %c%c\n", header->unique[0], header->unique[1]);
  fprintf(file, "destination: %c\n", header->destination);
  fprintf(file, "version: %d\n", header->version);
}

void nus_init(NusHeader *header) {
  memset(header, 0, sizeof(NusHeader));
  header->cfg_flags = 0x80371240; // NOLINT
  header->clck_rate = 0xF;        // NOLINT
  header->boot_addr = 0x80010000; // NOLINT
  header->category = 'N';
  header->destination = 'A';
}

u32 ntohl_from_(const u8 *data, usize offset) {
  return ntohl(*((u32 *)(data + offset)));
}

Error nus_from_bytes(NusHeader *header, const u8 *data, const usize len) {
  if (len < NUS_HEADER_SIZE) {
    return ERR_HEADER_NOT_ENOUGH_DATA;
  }

  // default values first!
  nus_init(header);

  // read the corresponding bytes
  header->cfg_flags = ntohl_from_(data, 0x00);
  header->clck_rate = ntohl_from_(data, 0x04);
  header->boot_addr = ntohl_from_(data, 0x08);
  header->lu_ver = ntohl_from_(data, 0x0C);
  header->crc.crc1 = ntohl_from_(data, 0x10);
  header->crc.crc2 = ntohl_from_(data, 0x14);

  memcpy(header->reserved_1, data + 0x18, 0x08);
  memcpy(header->title, data + 0x20, NUS_TITLE_LEN);
  memcpy(header->reserved_2, data + 0x34, 0x07);

  header->category = (char)data[0x3B];

  header->unique[0] = (char)data[0x3C];
  header->unique[1] = (char)data[0x3D];

  header->destination = (char)data[0x3E];
  header->version = (char)data[0x3F];

  // attempt crc, if it fails just leave it be!
  // if it didn't failt it should correct a wrong crc value
  // or calculate the same result as the exisiting value
  nus_crc(header, data, len);

  return OK;
}

// writes a BE u32 to result at offset
void htonl_to_(u32 n, u8 *result, const usize offset) {
  n = htonl(n);
  memcpy(result + offset, &n, sizeof(u32));
}

void nus_to_bytes(NusHeader *header, u8 *result) {
  htonl_to_(header->cfg_flags, result, 0x00);
  htonl_to_(header->clck_rate, result, 0x04);
  htonl_to_(header->boot_addr, result, 0x08);
  htonl_to_(header->lu_ver, result, 0x0C);
  htonl_to_(header->crc.crc1, result, 0x10);
  htonl_to_(header->crc.crc2, result, 0x14);

  memcpy(result + 0x18, header->reserved_1, 0x08);
  memcpy(result + 0x20, header->title, NUS_TITLE_LEN);
  memcpy(result + 0x34, header->reserved_2, 0x07);

  result[0x3B] = header->category;

  result[0x3C] = header->unique[0];
  result[0x3D] = header->unique[1];

  result[0x3E] = header->destination;
  result[0x3F] = header->version;
}

Error calc_crc_(const u8 *data, const usize len, NusCrc *crc) {
  // this is just some magic number used as an initial value
  const u32 INITIAL = -120959524;

  if (len < NUS_CRC_LEN + NUS_CRC_START) {
    return ERR_CRC_NOT_ENOUGH_DATA;
  }

  u32 t2 = INITIAL;
  u32 t3 = INITIAL;
  u32 a2 = INITIAL;
  u32 t4 = INITIAL;
  u32 crc1 = INITIAL;
  u32 crc2 = INITIAL;

  for (u32 idx = 0; idx < NUS_CRC_LEN; idx += 4) {
    // take the next 4 bytes and convert them to
    // a native integer
    u32 current_data = ntohl_from_(data, NUS_CRC_START + idx);

    u32 a1 = crc1 + current_data;

    if (a1 < crc1) {
      t2 = t2 + 1;
    }

    u32 v1 = current_data & 0x1f;                                // NOLINT
    u32 a0 = (current_data << v1) | (current_data >> (32 - v1)); // NOLINT
    crc1 = a1;

    t3 ^= current_data;

    crc2 = crc2 + a0;

    if (a2 < current_data) {
      a2 ^= crc1 ^ current_data;
    } else {
      a2 ^= a0;
    }

    t4 = t4 + (current_data ^ crc2);
  }

  crc1 = (crc1 ^ t2) ^ t3;
  crc2 = (crc2 ^ a2) ^ t4;

  crc->crc1 = crc1;
  crc->crc2 = crc2;

  return OK;
}

Error nus_crc(NusHeader *header, const u8 *data, const usize len) {
  NusCrc result;
  Error err = calc_crc_(data, len, &result);

  header->crc = result;

  return err;
}

#ifdef TEST

#include "macros.h"

void test_crc_fail(void **state) {
  u8 data1[] = {0, 1, 2, 3};
  NusCrc result;
  assert_int_equal(ERR_CRC_NOT_ENOUGH_DATA, calc_crc_(data1, 3, &result));

  // test failure off by 1 erro
  assert_int_equal(ERR_CRC_NOT_ENOUGH_DATA,
                   calc_crc_(data1, NUS_CRC_END - 1, &result));
}

void test_crc(void **state) {
  u8 *test_data = malloc(NUS_CRC_END);
  NusCrc result;

  for (u32 i = 0; i < NUS_CRC_END; i++) {
    test_data[i] = (u8)i;
  }

  assert_int_equal(OK, calc_crc_(test_data, NUS_CRC_END, &result));

  assert_int_equal(4207429594, result.crc1);
  assert_int_equal(3000934689, result.crc2);

  free(test_data);
}

#endif
