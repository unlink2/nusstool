#ifndef NUSUSB_H_
#define NUSUSB_H_

#include "buffer.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

// undef to remove this feature
// #define NO_NUSUSB

Error nus_usb_boot();
Error nus_usb_load(Buffer *buffer, u32 addr);
Error nus_usb_dump(Buffer *buffer, u32 addr);
Error nus_usb_ram_wr(Buffer *buffer, u32 addr);
Error nus_usb_ram_rd(Buffer *buffer, u32 addr);

#endif
