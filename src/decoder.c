#include "crc.h"
#include "huffman.h"
#include "image.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_path>\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char *const input_path = argv[1];
  struct PngImage *img = image_read(input_path);
  int img_w = image_get_width(img), img_h = image_get_height(img);
  unsigned char *buf = malloc(img_w * img_h);

  for (int y = 0; y < img_h; y++) {
    for (int x = 0; x < img_w; x++) {
      int bit_idx = x + y * img_w;
      struct Pixel pix = image_get_pixel(img, x, y);
      char msg_c = ((pix.red & 0x03) << 6) | ((pix.green & 0x03) << 4) |
                   ((pix.blue & 0x03) << 2) | (pix.alpha & 0x03);
      buf[bit_idx] = msg_c;
    }
  }

  struct BitStream bs = {0};
  bs.data = buf;
  huffman_decode(&bs, stdout);
  uint32_t crc_recovered = 0;
  crc_recovered |= ((uint32_t)buf[bs.data_len - 1]) << 24;
  crc_recovered |= ((uint32_t)buf[bs.data_len + 0]) << 16;
  crc_recovered |= ((uint32_t)buf[bs.data_len + 1]) << 8;
  crc_recovered |= ((uint32_t)buf[bs.data_len + 2]) << 0;
  uint32_t crc_calculated = crc32(buf, bs.data_len - 1);
  if (crc_recovered != crc_calculated) {
    fprintf(stderr, "WARNING: Error detected in decoded message.");
    fprintf(stderr, "CRC32 (recovered): %u\n", crc_recovered);
    fprintf(stderr, "CRC32 (calculated): %u\n", crc_calculated);
  }
  image_free(img);
  free(buf);
  return EXIT_SUCCESS;
}
