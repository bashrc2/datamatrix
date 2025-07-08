/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - centre/surround functions
 *  Copyright (c) 2013-2015, Bob Mottram
 *  bob@libreserver.org
 *
 *  This code is based on Visionutils by Bob Mottram which was
 *  originally distributed under BSD 2-clause license.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include "datamatrix.h"

/**
 * @brief makes an image larger using an appropriate background colour
 * @param img The original image array
 * @param width Width of the original image
 * @param height Height of the original image
 * @param bitsperpixel Number of bits per pixel in the image
 * @param enlarged The returned enlarged image
 * @param enlarged_width Width of the enlarged image
 * @param enlarged_height Height of the enlarged image
 * @return zero on success
 */
int enlarge_image(unsigned char img[], int width, int height,
                  int bitsperpixel,
                  unsigned char enlarged[],
                  int enlarged_width, int enlarged_height)
{
  int i, c, tx, ty, bx, by, x, y, n0=0, n1;
  int bytes_per_pixel = bitsperpixel/8;
  unsigned char background[4];

  if (enlarged_width < width) {
    return -1;
  }
  if (enlarged_height < height) {
    return -2;
  }
  if (bytes_per_pixel > 4) {
    return -3;
  }

  /* location of the original image within the larger one */
  tx = (enlarged_width - width)/2;
  ty = (enlarged_height - height)/2;
  bx = tx + width;
  by = ty + height;

  /* get the background colour */
  for (c = 0; c < bytes_per_pixel; c++) {
    background[c] = img[c];
  }

  /* set the background colour in the enlarged image */
  c = 0;
  for (i = 0; i < enlarged_width*enlarged_height*bytes_per_pixel; i++) {
    enlarged[i] = background[c++];
    if (c >= bytes_per_pixel) c = 0;
  }

  /* copy the original into the enlarged */
  for (y = ty; y < by; y++) {
    n1 = (y*enlarged_width + tx)*bytes_per_pixel;
    for (x = tx; x < bx; x++, n0+=bytes_per_pixel, n1+=bytes_per_pixel) {
      for (c = 0; c < bytes_per_pixel; c++)
        enlarged[n1+c] = img[n0+c];
    }
  }
  return 0;
}

/**
 * @brief Changes the size of an image
 * @param img The original image array
 * @param width Width of the original image
 * @param height Height of the original image
 * @param bitsperpixel Number of bits per pixel in the image
 * @param resized The returned enlarged image
 * @param resized_width Width of the enlarged image
 * @param resized_height Height of the enlarged image
 * @return zero on success
 */
int resize_image(unsigned char img[], int width, int height,
                 int bitsperpixel,
                 unsigned char resized[],
                 int resized_width, int resized_height)
{
  int c, x, y, x2, y2, n1, n2;
  int bytes_per_pixel = bitsperpixel/8;

  if ((resized_width == width) && (resized_height == height)) {
    memcpy(resized, img, width*height*bytes_per_pixel);
    return 0;
  }

  if ((resized_width > width) || (resized_height > height)) {
    return enlarge_image(img, width, height, bitsperpixel,
                         resized, resized_width, resized_height);
  }

  if (bytes_per_pixel > 4) {
    return -1;
  }

  memset(resized, '\0', resized_width*resized_height*bytes_per_pixel);

  for (y = resized_height-1; y >= 0; y--) {
    y2 = y * height / resized_height;
    n2 = (y*resized_width + resized_width-1)*bytes_per_pixel;
    for (x = resized_width-1; x >= 0; x--,n2-=bytes_per_pixel) {
      x2 = x * width / resized_width;
      n1 = (y2*width + x2)*bytes_per_pixel;
      /* NOTE: this isn't optimal, but probably good enough */
      for (c = bytes_per_pixel-1; c >= 0; c--)
        if (resized[n2+c] == 0)
          resized[n2+c] = img[n1+c];
        else
          resized[n2+c] = (unsigned int)((int)img[n1+c]+(int)resized[n2+c])/2;
    }
  }
  return 0;
}

/**
 * @brief Changes the size of a thresholded image (light on dark)
 * @param img The original thresholded image array
 * @param width Width of the original image
 * @param height Height of the original image
 * @param bitsperpixel Number of bits per pixel in the image
 * @param resized The returned resized image
 * @param resized_width Width of the resized image
 * @param resized_height Height of the resized image
 * @return zero on success
 */
int resize_thresholded_image(unsigned char img[], int width, int height,
                             int bitsperpixel,
                             unsigned char resized[],
                             int resized_width, int resized_height)
{
  int c, x, y, x2, y2, n1, n2;
  int bytes_per_pixel = bitsperpixel/8;

  if ((resized_width == width) && (resized_height == height)) {
    memcpy(resized, img, width*height*bytes_per_pixel);
    return 0;
  }

  if ((resized_width > width) || (resized_height > height)) {
    return enlarge_image(img, width, height, bitsperpixel,
                         resized, resized_width, resized_height);
  }

  if (bytes_per_pixel > 4) {
    return -1;
  }

  memset(resized, '\0', resized_width*resized_height*bytes_per_pixel);

  for (y = height-1; y >= 0; y--) {
    y2 = y * resized_height / height;
    n2 = (y*width + width-1)*bytes_per_pixel;
    for (x = width-1; x >= 0; x--, n2-=bytes_per_pixel) {
      if (img[n2] == 0) continue;
      x2 = x * resized_width / width;
      n1 = (y2*resized_width + x2)*bytes_per_pixel;
      for (c = bytes_per_pixel-1; c >= 0; c--) {
        resized[n1+c] = 255;
      }
    }
  }
  return 0;
}
