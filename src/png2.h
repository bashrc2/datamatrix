#ifndef PNG2_H
#define PNG2_H

#include <stdio.h>
#include "lodepng.h"

static unsigned char * read_png_file(char * filename,
                                     unsigned int * width,
                                     unsigned int * height,
                                     unsigned int * bitsperpixel)
{
  unsigned error;
	unsigned char* image;
	unsigned w, h;
	error = lodepng_decode24_file(&image, &w, &h, filename);
	if (error) printf("read_png_file: error %u: %s\n", error, lodepng_error_text(error));

  *width = w;
  *height = h;
  *bitsperpixel=24;
  return image;
}

static int write_png_file(char* filename,
                          unsigned int width, unsigned int height,
                          unsigned int bitsperpixel,
                          unsigned char *buffer)
{
	unsigned error=1;
	unsigned int i;
	unsigned char * image = buffer;

	if (bitsperpixel == 32) {
		error = lodepng_encode32_file(filename, image, width, height);
	}
	if (bitsperpixel == 24) {
		error = lodepng_encode24_file(filename, image, width, height);
	}
	if (bitsperpixel == 8) {
		image = (unsigned char*)malloc(width*height*3);
		if (image) {
			for (i = 0; i < width*height; i++) {
				image[i*3] = buffer[i];
				image[i*3+1] = buffer[i];
				image[i*3+2] = buffer[i];
			}
			error = lodepng_encode24_file(filename, image, width, height);
			free(image);
		}
	}

	if (error) {
		printf("write_png_file: error %u: %s\n", error, lodepng_error_text(error));
		return -1;
	}
  return 0;
}

#endif
