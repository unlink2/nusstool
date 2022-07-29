#include "nususb.h"
#include "cfg.h"
#include "error.h"
#include "macros.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#ifndef NO_NUSUSB

#include <libftdi1/ftdi.h>

#define NUS_USB_VENDOR 0x0403
#define NUS_USB_DEVICE 0x6001
#define NUS_USB_READ_TIMEOUT 5000
#define NUS_USB_WRITE_TIMEOUT 5000
#define BUFFER_SIZE 1024
#define NUS_BAUD 115200
#define NUS_USB_BUF_LEN 512

#define NUS_ROM_BASE_ADDRESS 0x10000000
#define NUS_RAM_BASE_ADDRESS 0x80000000

u8 write_buffer[NUS_USB_BUF_LEN];
u8 read_buffer[NUS_USB_BUF_LEN];

void setup_command(char cmd, u32 address, u32 len, u32 argument) {
  memcpy(write_buffer, "cmdW", 5);
  write_buffer[0] = 'c';
  write_buffer[1] = 'm';
  write_buffer[2] = 'd';
  write_buffer[3] = cmd;

  if (nuss_verbose) {
    fprintf(stderr, "sending command '%c' to address %x len %d arg %d\n", cmd,
            address, len, argument);
  }

  address = htonl(address);
  len = htonl(len / 0x200); // take buffer size into account
  argument = htonl(argument);
  memcpy(&write_buffer[4], &address, sizeof(u32));
  memcpy(&write_buffer[8], &len, sizeof(u32));
  memcpy(&write_buffer[12], &argument, sizeof(u32));
}

usize send_command_(struct ftdi_context *ftdi) {
  usize write_amount = ftdi_write_data(ftdi, write_buffer, NUS_USB_BUF_LEN);

  if (nuss_verbose) {
    fprintf(stderr, "sent %li bytes\n", write_amount);
  }
  usize read_amount = ftdi_read_data(ftdi, read_buffer, NUS_USB_BUF_LEN);

  if (nuss_verbose) {
    fprintf(stderr, "read %li bytes\n", read_amount);
  }

  return write_amount;
}

Error usb_test(struct ftdi_context *ftdi) {
  // test connection
  setup_command('t', 0, 0, 0);
  send_command_(ftdi);
  // test command should return k or r (r is newer)
  if (read_buffer[3] == 'k' || read_buffer[3] == 'r') {
    if (nuss_verbose) {
      printf("init test: ok\n");
    }
  } else {
    if (nuss_verbose) {
      fprintf(stderr, "Init test failed!\n");
    }
    ftdi_free(ftdi);
    return ERR_NUS_USB;
  }
  return OK;
}

Error usb_init(struct ftdi_context **ftdi) {
  *ftdi = ftdi_new();
  if (*ftdi == NULL) {
    if (nuss_verbose) {
      fprintf(stderr, "ftdi failed\n");
    }
    return ERR_NUS_USB;
  }

  u32 device = ftdi_usb_open(*ftdi, NUS_USB_VENDOR, NUS_USB_DEVICE);
  if (device < 0) {
    ftdi_free(*ftdi);
    if (nuss_verbose) {
      fprintf(stderr, "unable to open ftdi device: %d (%s)\n", device,
              ftdi_get_error_string(*ftdi));
    }
    return ERR_NUS_USB;
  }

  (*ftdi)->usb_read_timeout = NUS_USB_READ_TIMEOUT;
  (*ftdi)->usb_write_timeout = NUS_USB_WRITE_TIMEOUT;

  u32 baud_ok = ftdi_set_baudrate(*ftdi, NUS_BAUD);

  if (baud_ok < 0) {
    ftdi_free(*ftdi);
    return ERR_NUS_USB;
  }

  if ((*ftdi)->type == TYPE_R && nuss_verbose) {
    u32 chipid = 0;
    fprintf(stderr, "ftdi_read_chipid: %d\n",
            ftdi_read_chipid((*ftdi), &chipid));
    fprintf(stderr, "ftdi chipid: %X\n", chipid);
  }

  memset(write_buffer, 0, NUS_USB_BUF_LEN);
  memset(read_buffer, 0, NUS_USB_BUF_LEN);

  if (usb_test(*ftdi)) {
    return ERR_NUS_USB;
  }

  return OK;
}

Error usb_free(struct ftdi_context *ftdi) {
  if (ftdi_usb_close(ftdi) < 0) {
    fprintf(stderr, "ftdi close failed: %s\n", ftdi_get_error_string(ftdi));
    ftdi_free(ftdi);
    return ERR_NUS_USB;
  }
  ftdi_free(ftdi);

  return OK;
}

Error nus_usb_boot() {
  struct ftdi_context *ftdi = NULL;
  if (usb_init(&ftdi)) {
    return ERR_NUS_USB;
  }

  if (nuss_verbose) {
    fprintf(stderr, "Booting...\n");
  }

  // put it into pif boot mode
  setup_command('s', NUS_ROM_BASE_ADDRESS, 0, 1);
  send_command_(ftdi);

  // free ftdi
  if (usb_free(ftdi)) {
    return ERR_NUS_USB;
  }

  return OK;
}

Error nus_usb_load(Buffer *buffer) {
  struct ftdi_context *ftdi = NULL;
  if (usb_init(&ftdi)) {
    return ERR_NUS_USB;
  }

  // test size and fill if needed
  const u32 crc_area = 0x100000 + 4096;
  if (buffer->len < crc_area) {
    if (nuss_verbose) {
      fprintf(stderr, "Filling rom space...\n");
    }

    setup_command('f', NUS_ROM_BASE_ADDRESS, crc_area, 0);
    send_command_(ftdi);

    if (usb_test(ftdi)) {
      return ERR_NUS_USB;
    }
  }

  // init write
  setup_command('W', NUS_ROM_BASE_ADDRESS, buffer->len, 0);
  send_command_(ftdi);

  if (nuss_verbose) {
    fprintf(stderr, "Write mode...\n");
  }

  const usize step = 0x8000;
  for (usize sent = 0; sent < buffer->len; sent += step) { // NOLINT
    u32 amount = ftdi_write_data(ftdi, buffer->data + sent,
                                 MIN(step, buffer->len - sent));
    if (amount > 0 && nuss_verbose) {
      fprintf(stderr, "sent %li/%li bytes\n", amount + sent, buffer->len);
    } else {
      if (nuss_verbose) {
        fprintf(stderr, "send timeout!\n");
      }
      ftdi_free(ftdi);
      return ERR_NUS_USB;
    }
  }

  // free ftdi
  if (usb_free(ftdi)) {
    return ERR_NUS_USB;
  }

  return OK;
}

#else

Error nus_usb_boot() {
  fprintf(stderr,
          "Nus usb is disabled. Recompile with the feature turned on!\n");

  return ERR_NUS_USB;
}

Error nus_usb_load(Buffer *buffer) { return nus_usb_boot(); }

#endif
