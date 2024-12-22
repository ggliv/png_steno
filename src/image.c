#include <limits.h>
#if CHAR_BIT != 8
#error Machine must have 8-bit bytes.
#endif
#include <stdio.h>
#include <stdlib.h>

#define WUFFS_IMPLEMENTATION
#define WUFFS_CONFIG__STATIC_FUNCTIONS
#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__CRC32
#define WUFFS_CONFIG__MODULE__ZLIB
#define WUFFS_CONFIG__MODULE__ADLER32
#define WUFFS_CONFIG__MODULE__DEFLATE
#define WUFFS_CONFIG__MODULE__PNG
#define WUFFS_CONFIG__ENABLE_DROP_IN_REPLACEMENT__STB
#include "wuffs/release/c/wuffs-v0.4.c"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "image.h"

struct PngImage {
  int width;
  int height;
  unsigned char *pixels_rgba;
};

struct PngImage *image_read(const char *file_path) {
  struct PngImage *img = malloc(sizeof(struct PngImage));
  img->pixels_rgba =
      stbi_load(file_path, &img->width, &img->height, NULL, STBI_rgb_alpha);
  if (img->pixels_rgba == NULL) {
    fprintf(stderr, "ERROR (STBI): %s\n", stbi_failure_reason());
    exit(EXIT_FAILURE);
  }
  return img;
}

int image_get_width(struct PngImage *img) { return img->width; }

int image_get_height(struct PngImage *img) { return img->height; }

struct Pixel image_get_pixel(struct PngImage *img, int x, int y) {
  struct Pixel pix;
  int start = (y * image_get_width(img) * 4) + x * 4;
  pix.red = img->pixels_rgba[start++];
  pix.blue = img->pixels_rgba[start++];
  pix.green = img->pixels_rgba[start++];
  pix.alpha = img->pixels_rgba[start++];
  return pix;
}

void image_set_pixel(struct PngImage *img, int x, int y, struct Pixel pix) {
  int start = (y * image_get_width(img) * 4) + x * 4;
  img->pixels_rgba[start++] = pix.red;
  img->pixels_rgba[start++] = pix.blue;
  img->pixels_rgba[start++] = pix.green;
  img->pixels_rgba[start++] = pix.alpha;
}

void image_write(struct PngImage *img, const char *file_path) {
  if (!stbi_write_png(file_path, img->width, img->height, STBI_rgb_alpha,
                      img->pixels_rgba, STBI_rgb_alpha * img->width)) {
    fprintf(stderr, "ERROR: STBI write\n");
    exit(EXIT_FAILURE);
  }
}

void image_free(struct PngImage *img) {
  stbi_image_free(img->pixels_rgba);
  free(img);
}
