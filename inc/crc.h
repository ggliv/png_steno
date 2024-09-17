#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <inttypes.h>

uint32_t crc32(const unsigned char *message, size_t len);

#endif
