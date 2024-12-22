#include "bit_stream.h"
#include "crc.h"
#include "huffman.h"
#include "image.h"
#include <stdio.h>
#include <stdlib.h>

char *file_to_buf(const char *file_path) {
  FILE *file_handle = fopen(file_path, "r");
  if (file_handle == NULL) {
    fprintf(stderr, "Could not open message file.");
  }
  fseek(file_handle, 0, SEEK_END);
  long file_len = ftell(file_handle);
  if (file_len < 0) {
    fprintf(stderr, "Could not read message.\n");
    exit(EXIT_FAILURE);
  }
  rewind(file_handle);
  char *buf = malloc(file_len + 1);
  if (fread(buf, 1, file_len, file_handle) != (unsigned long)file_len) {
    fprintf(stderr, "Could not read message.\n");
    exit(EXIT_FAILURE);
  }
  fclose(file_handle);
  buf[file_len] = '\0';
  return buf;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "Usage: %s <png_input_path> <png_output_path> <message_path>\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  const char *const png_input_path = argv[1], *const png_output_path = argv[2],
                    *const message_path = argv[3];
  const char *const message = file_to_buf(message_path);
  struct BitStream bs = huffman_encode(message);
  uint32_t crc = crc32(bs.data, bs.data_len - 1);
  bs.data = realloc(bs.data, bs.data_len + sizeof(crc));
  bs.data[bs.data_len - 1] = (crc >> 24) & 0xFF;
  bs.data[bs.data_len + 0] = (crc >> 16) & 0xFF;
  bs.data[bs.data_len + 1] = (crc >> 8) & 0xFF;
  bs.data[bs.data_len + 2] = (crc >> 0) & 0xFF;
  bs.data_len += sizeof(crc);
  struct PngImage *img = image_read(png_input_path);
  int img_width = image_get_width(img);
  int img_height = image_get_height(img);

  if ((size_t)(img_width * img_height) < bs.data_len) {
    fprintf(stderr,
            "ERROR: Message is too long to encode into provided PNG. Max: %d, "
            "message: %lu.\n",
            img_width * img_height, bs.data_len);
    exit(EXIT_FAILURE);
  }

  struct Pixel pix;

  for (int y = 0; y < img_height; y++) {
    for (int x = 0; x < img_width; x++) {
      size_t byte_idx = x + y * img_width;
      if (byte_idx >= bs.data_len) {
        goto image_loop_done;
      }

      pix = image_get_pixel(img, x, y);
      unsigned char msg_c = bs.data[byte_idx];

      pix.red &= 0xFC;
      pix.green &= 0xFC;
      pix.blue &= 0xFC;
      pix.alpha &= 0xFC;

      pix.red |= (msg_c & 0xC0) >> 6;
      pix.green |= (msg_c & 0x30) >> 4;
      pix.blue |= (msg_c & 0x0C) >> 2;
      pix.alpha |= (msg_c & 0x03);

      image_set_pixel(img, x, y, pix);
    }
  }
image_loop_done:
  image_write(img, png_output_path);
  image_free(img);
  free(bs.data);
  free((void *)message);
  return EXIT_SUCCESS;
}
