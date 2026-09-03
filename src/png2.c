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

/*
 * \brief reads a PNG file and returns an array of pixels
 * \param filename PNG filename
 * \param width Returned width of the image
 * \param height Returned height of the image
 * \param bitsperpixel Returned bits per pixel
 * \return array of pixels
 */
unsigned char * read_png_file(const char * filename,
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

/*
 * \brief saves a PNG to file
 * \param filename PNG filename
 * \param width Width of the image
 * \param height Height of the image
 * \param bitsperpixel Bits per pixel
 * \param buffer array of pixels
 * \return true if the image was saved
 */
bool write_png_file(const char * filename,
                    const int width, const int height,
                    const int bitsperpixel,
                    unsigned char buffer[])
{
    unsigned error=1;
    int i;
    unsigned char * image = buffer;

    if (bitsperpixel == 32) {
        error = lodepng_encode32_file(filename, image,
                                      (unsigned int)width, (unsigned int)height);
    }
    if (bitsperpixel == 24) {
        error = lodepng_encode24_file(filename, image,
                                      (unsigned int)width, (unsigned int)height);
    }
    if (bitsperpixel == 8) {
        image = (unsigned char*)safemalloc((size_t)(width * height * 3));
        if (image) {
            for (i = 0; i < width*height; i++) {
                image[i*3] = buffer[i];
                image[i*3+1] = buffer[i];
                image[i*3+2] = buffer[i];
            }
            error = lodepng_encode24_file(filename, image,
                                          (unsigned int)width,
                                          (unsigned int)height);
            free(image);
        }
    }

    if (error) {
        printf("write_png_file: error %u: %s\n", error,
               lodepng_error_text(error));
        return false;
    }
    return true;
}
