#include "buffer.h"
#include "error.h"

void buffer_init(Buffer *buffer) { buffer->data = NULL; }

Error buffer_read(Buffer *buffer, FILE *file) {
  if (!fseek(file, 0, SEEK_END)) {
    return ERR_READ_ERROR;
  }
  const long fsize = ftell(file);

  if (!fseek(file, 0, SEEK_SET)) {
    return ERR_READ_ERROR;
  }

  buffer->data = malloc(fsize + 1);
  buffer->len = fsize;

  if (!fread(buffer->data, fsize, 1, file)) {
    return ERR_READ_ERROR;
  }

  return OK;
}

Error buffer_write(const Buffer *buffer, FILE *file) {
  if (!fwrite(buffer->data, buffer->len, 1, file)) {
    return ERR_WRITE_ERROR;
  }
  return OK;
}

void buffer_pad_to(Buffer *buffer, size loc, size len, u8 val) {}

void buffer_pad_by(Buffer *buffer, size loc, size len, u8 val) {}

void buffer_inject(Buffer *buffer, size loc, u8 *data, size len) {}

void buffer_inject_file(Buffer *buffer, size loc, FILE *file) {}

void buffer_resize(Buffer *buffer) {}

void buffer_free(Buffer *buffer) {
  if (buffer->data != NULL) {
    free(buffer->data);
    buffer->data = NULL;
  }
}
