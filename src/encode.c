/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Functions to display an encoded datamatrix pattern
 *  Copyright (c) 2026, Bob Mottram
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

#include <stdio.h>
#include <string.h>
#include "datamatrix.h"

/**
 * \brief draws a dot at the given coordinates
 * \param img returned image array
 * \param width width of the image
 * \param height height of the image
 * \param bytes_per_pixel Number of bytes per pixel
 * \param dot_x x coordinate of the centre of the dot
 * \param dot_y y coordinate of the centre of the dot
 * \param dot_radius radius of the dot
 */
static void encode_image_dot(unsigned char img[], int width, int height,
                             int bytes_per_pixel,
                             int dot_x, int dot_y, int dot_radius)
{
    int tx = dot_x - dot_radius;
    int bx = dot_x + dot_radius;
    int ty = dot_y - dot_radius;
    int by = dot_y + dot_radius;
    int x, y, c, dx, dy, idx;
    int dot_radius_sqr = dot_radius * dot_radius;

    if (tx < 0) tx = 0;
    if (bx >= width) bx = width-1;
    if (ty < 0) ty = 0;
    if (by >= height) by = height-1;

    for (y = ty; y < by; y++) {
        dy = y - dot_y;
        for (x = tx; x < bx; x++) {
            dx = x - dot_x;
            if (SQUARE_MAG(dx, dy) > dot_radius_sqr) continue;

            idx = (y*width + x) * bytes_per_pixel;
            for (c = bytes_per_pixel-1; c >= 0; c--, idx++) img[idx] = 0;
        }
    }
}

/**
 * \brief returns an image from the given datamatrix grid
 * \param img returned image array
 * \param width width of the image
 * \param height height of the image
 * \param bitsperpixel Number of bits per pixel
 * \param encode_width width of the datamatrix grid
 * \param encode_height height of the datamatrix grid
 */
void encode_image(unsigned char img[], int width, int height,
                  int bitsperpixel, unsigned char *grid,
                  unsigned int encode_width, unsigned int encode_height)
{
    unsigned int x, y;
    int bytes_per_pixel = bitsperpixel/8;
    int dot_x, dot_y;
    int half_cell_width = width / ((int)encode_width * 2);
    int half_cell_height = height / ((int)encode_height * 2);
    int dot_radius = half_cell_width * 8 / 10;

    /* clear the image */
    memset(img, 255, width*height*bytes_per_pixel*sizeof(unsigned char));

    /* draw dots */
    for (y = 0; y < encode_height; y++) {
        dot_y = ((int)y * height / (int)encode_height) + half_cell_height;
        for (x = 0; x < encode_width; x++) {
            if (!grid[encode_width * y + x]) continue;
            dot_x = ((int)x * width / (int)encode_width) + half_cell_width;
            encode_image_dot(img, width, height, bytes_per_pixel,
                             dot_x, dot_y, dot_radius);
        }
    }
}
