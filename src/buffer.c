#include "buffer.h"
#include "error.h"
#include <string.h>
#include "macros.h"

void buffer_init(Buffer *buffer) {
  buffer->data = NULL;
  buffer->len = 0;
}

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
  size flen = 524288; // start at 0.5mb
  size total_read = 0;

  buffer->data = malloc(flen + 1);
  buffer->len = flen;

  char buf = '\0';
  // read 1 byte until no more bytes are left to read
  // TODO this is likely slow, but ok for now
  while (fread(&buf, 1, 1, file) > 0) {
    // the resize call will not have any effect
    // unless the requested size is larger than the new size!
    if (total_read >= buffer->len) {
      buffer_resize(buffer, buffer->len * 2);
    }

    // write to next byte
    buffer->data[total_read++] = buf;
  }

  // make sure we have the correct length in the end
  // any additional data does not matter and can be ignored
  // TODO maybe keep track of alloc lenght at some point
  buffer->len = total_read;

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
  if (new_len < buffer->len) {
    return;
  }

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
