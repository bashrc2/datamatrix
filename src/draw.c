/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - example computer vision functions
 *  Copyright (c) 2011-2018, Bob Mottram
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
 * @brief Draws a line of the given width and colour
 * @param Array containing image
 * @param width Width of the image
 * @param height Height of the image
 * @param bitsperpixel Number of bits per pixel
 * @param tx top left of the line
 * @param ty top of the line
 * @param bx bottom right of the line
 * @param by bottom of the line
 * @param line_width width of the line
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void draw_line(unsigned char img[],
               unsigned int width, unsigned int height,
               int bitsperpixel,
               int tx, int ty, int bx, int by,
               int line_width,
               int r, int g, int b)
{
  int linewidth = bx - tx;
  int lineheight = by - ty;
  int x, y, n, xx, yy, incr=1;
  int bytes_per_pixel = bitsperpixel/8;
  int half_width = line_width/2;

  if (abs(lineheight) > abs(linewidth)) {
    /* vertical orientation */
    if (by < ty) incr = -1;
    for (y = ty; y != by; y+=incr) {
      if ((y < 0) || (y >= (int)height))
        continue;
      x = tx + ((y - ty) * linewidth / lineheight);
      for (xx = x - half_width; xx < x - half_width + line_width; xx++) {
	      if ((xx < 0) || (xx >= (int)width))
          continue;
        n = (y * width + xx) * bytes_per_pixel;
        if (bytes_per_pixel == 3) {
          img[n] = b;
          img[n+1] = g;
          img[n+2] = r;
        }
        else {
          img[n] = r;
        }
      }
    }
  }
  else {
    /* horizontal orientation */
    if (bx < tx) incr = -1;
    for (x = tx; x != bx; x+=incr) {
      if ((x < 0) || (x >= (int)width))
        continue;
      y = ty + ((x - tx) * lineheight / linewidth);
      for (yy = y - half_width; yy < y - half_width + line_width; yy++) {
	      if ((yy < 0) || (yy >= (int)height))
          continue;
        n = (yy * width + x) * bytes_per_pixel;
        if (bytes_per_pixel == 3) {
          img[n] = b;
          img[n+1] = g;
          img[n+2] = r;
        }
        else {
          img[n] = r;
        }
      }
    }
  }
}
