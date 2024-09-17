#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_FAILURE_USERMSG
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image.h"
#include "stb/stb_image_write.h"
#include <limits.h>
#if CHAR_BIT != 8
#error Machine must have 8-bit bytes.
#endif
#include <stdio.h>
#include <stdlib.h>

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
