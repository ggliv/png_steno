#ifndef IMAGE_H
#define IMAGE_H

struct PngImage;
struct Pixel {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
};
struct PngImage *image_read(const char *file_path);
void image_write(struct PngImage *img, const char *file_path);
void image_free(struct PngImage *img);
int image_get_width(struct PngImage *img);
int image_get_height(struct PngImage *img);
struct Pixel image_get_pixel(struct PngImage *img, int x, int y);
void image_set_pixel(struct PngImage *img, int x, int y, struct Pixel pix);

#endif // IMAGE_H
