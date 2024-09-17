#include "bit_stream.h"
#include <limits.h>
#include <stddef.h>

void bs_select_next_bit(struct BitStream *bs) {
  // TODO check data length and realloc if needed
  bs->bit_offset += 1;
  if (bs->bit_offset == CHAR_BIT) {
    bs->byte_offset++;
    bs->bit_offset = 0;
  }
}

// insert a bit into the currently selected bit and set state to the next bit
void bs_write_bit(struct BitStream *bs, bool bit) {
  // TODO assert no overflow
  bs->data[bs->byte_offset] &= ~(1 << (CHAR_BIT - bs->bit_offset - 1));
  bs->data[bs->byte_offset] |= (!!bit << (CHAR_BIT - bs->bit_offset - 1));
  bs_select_next_bit(bs);
}

// extract a bit from the currently selected bit and set state to the next bit
bool bs_read_bit(struct BitStream *bs) {
  // TODO assert no underflow
  unsigned char bit =
      ((bs->data[bs->byte_offset]) >> (CHAR_BIT - 1 - bs->bit_offset)) & 1;
  bs_select_next_bit(bs);
  return bit;
}
