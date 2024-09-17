#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "bit_stream.h"
#include <stdio.h>

struct BitStream huffman_encode(const char *const message);
void huffman_decode(struct BitStream *bs, FILE *writeback);


#endif // HUFFMAN_H
