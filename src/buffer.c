#include "buffer.h"
#include "error.h"
#include <string.h>
#include "macros.h"

void buffer_init(Buffer *buffer) { buffer->data = NULL; }

Error file_len(FILE *file, size *len) {
  if (!fseek(file, 0, SEEK_END)) {
    return ERR_READ;
  }
  *len = ftell(file);

  if (!fseek(file, 0, SEEK_SET)) {
    return ERR_READ;
  }

  return OK;
}

Error buffer_read(Buffer *buffer, FILE *file) {
  size flen = 0;
  if (file_len(file, &flen)) {
    return ERR_READ;
  }

  buffer->data = malloc(flen + 1);
  buffer->len = flen;

  if (!fread(buffer->data, flen, 1, file)) {
    return ERR_READ;
  }

  return OK;
}

Error buffer_write(const Buffer *buffer, FILE *file) {
  if (!fwrite(buffer->data, buffer->len, 1, file)) {
    return ERR_WRITE;
  }
  return OK;
}

void buffer_pad_to(Buffer *buffer, const size loc, const size len,
                   const u8 val) {
  // if we already have the desired size dont do anything
  if (len <= buffer->len) {
    return;
  }

  size old_len = buffer->len;
  buffer_resize(buffer, len);

  // memset the rest of the buffer to the desired value
  memset(buffer->data + old_len, (int)(buffer->len - old_len), val);
}

void buffer_pad_by(Buffer *buffer, const size loc, const size len,
                   const u8 val) {
  size old_len = buffer->len;
  buffer_resize(buffer, buffer->len + len);

  // memset the rest of the buffer to the destired value
  memset(buffer->data + old_len, (int)len, val);
}

void buffer_inject(Buffer *buffer, const size loc, const u8 *data,
                   const size len) {
  size old_len = buffer->len;
  buffer_resize(buffer, buffer->len + len);

  // copy to destination
  memcpy(buffer->data + old_len, data, len);
}

Error buffer_inject_file(Buffer *buffer, const size loc, FILE *file) {
  size flen = 0;
  file_len(file, &flen);

  size old_len = buffer->len;
  buffer_resize(buffer, buffer->len + flen);

  // we can read the file straight into the resized buffer!
  if (!fread(buffer->data + old_len, flen, 1, file)) {
    return ERR_READ;
  }
  return OK;
}

void buffer_resize(Buffer *buffer, const size new_len) {
  u8 *new_buffer = malloc(new_len);
  memcpy(new_buffer, buffer->data, MIN(buffer->len, new_len));

  free(buffer->data);

  buffer->len = new_len;
  buffer->data = new_buffer;
}

void buffer_free(Buffer *buffer) {
  if (buffer->data != NULL) {
    free(buffer->data);
    buffer->data = NULL;
  }
}
