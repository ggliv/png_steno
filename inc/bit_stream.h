#ifndef BIT_STREAM_H
#define BIT_STREAM_H

#include <stddef.h>
#include <stdbool.h>

struct BitStream {
  unsigned char *data;
  size_t data_len;
  size_t byte_offset;
  size_t bit_offset;
};

void bs_write_bit(struct BitStream *bs, bool bit);
bool bs_read_bit(struct BitStream *bs);

#endif // BIT_STREAM_H
