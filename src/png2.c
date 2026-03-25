/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Wrapper for lodepng
 *  Copyright (c) 2011-2026, Bob Mottram
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

unsigned char * read_png_file(char * filename,
                              unsigned int * width,
                              unsigned int * height,
                              unsigned int * bitsperpixel)
{
    unsigned error;
    unsigned char* image;
    unsigned w, h;
    error = lodepng_decode24_file(&image, &w, &h, filename);
    if (error) printf("read_png_file: error %u: %s\n", error,
                      lodepng_error_text(error));

    *width = w;
    *height = h;
    *bitsperpixel=24;
    return image;
}

int write_png_file(char* filename,
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
        printf("write_png_file: error %u: %s\n", error,
               lodepng_error_text(error));
        return -1;
    }
    return 0;
}
