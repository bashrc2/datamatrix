/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - example computer vision functions
 *  Copyright (c) 2011-2015, Bob Mottram
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

/* convert an image from mono to colour */
void mono_to_colour(unsigned char * img, int width, int height,
                    int bitsperpixel,
                    unsigned char * colour)
{
    int i,ch;
    int bytesperpixel = bitsperpixel/8;

    for (i = 0; i < width*height; i++)
        for (ch = 0; ch < bytesperpixel; ch++)
            colour[i*bytesperpixel + ch] = img[i];
}

/* convert an image from colour to mono */
void colour_to_mono(unsigned char * img, int width, int height,
                    int bitsperpixel,
                    unsigned char * mono)
{
    int i,ch,v;
    int bytesperpixel = bitsperpixel/8;

    for (i = 0; i < width*height; i++) {
        v = 0;
        for (ch = 0; ch < bytesperpixel; ch++) {
            v += img[i*bytesperpixel + ch];
        }
        mono[i] = v/bytesperpixel;
    }
}

/* convert a mono image to a bitwise image, compressing it
   by a factor of 8. You may previously have used an adaptive
   threshold to binarise the image */
void mono_to_bitwise(unsigned char * img, int width, int height,
                     unsigned char * bitwise)
{
    int i;
    int bit_index = 0;
    int byte_index = 0;
    int bitbin = 1;

    memset((void*)bitwise, '\0',
           width*height/8 * sizeof(unsigned char));

    for (i = 0; i < width*height; i++) {
        if (img[i] > 127)
            bitwise[byte_index] |= bitbin;

        bit_index++;
        bitbin *= 2;
        if (bit_index >= 8) {
            bit_index = 0;
            bitbin = 1;
            byte_index++;
        }
    }
}

void bitwise_to_mono(unsigned char * bitwise, int width, int height,
                     unsigned char * img)
{
    int i;
    int bit_index = 0;
    int byte_index = 0;
    int bitbin = 1;

    memset((void*)img, '\0',
           width*height * sizeof(unsigned char));

    for (i = 0; i < width*height; i++) {
        if (bitwise[byte_index] & bitbin)
            img[i] = 255;

        bit_index++;
        bitbin *= 2;
        if (bit_index >= 8) {
            bit_index = 0;
            bitbin = 1;
            byte_index++;
        }
    }
}

void bitwise_to_colour(unsigned char * bitwise, int width, int height,
                       unsigned char * img, int bytesperpixel)
{
    int i, j;
    int bit_index = 0;
    int byte_index = 0;
    int bitbin = 1;

    memset((void*)img, '\0',
           width*height*bytesperpixel * sizeof(unsigned char));

    for (i = 0; i < width*height; i++) {
        if (bitwise[byte_index] & bitbin) {
            for (j = 0; j < bytesperpixel; j++) {
                img[i*bytesperpixel + j] = 255;
            }
        }

        bit_index++;
        bitbin *= 2;
        if (bit_index >= 8) {
            bit_index = 0;
            bitbin = 1;
            byte_index++;
        }
    }
}

int save_bitwise(char * filename,
                 unsigned char * bitwise, int width, int height)
{
    FILE * fp = fopen(filename, "wb");
    if (!fp) return -1;
    fwrite(bitwise, width*height/8, 1, fp);
    fclose(fp);
    return 0;
}

/* it is assumed that you know the dimensions of the image in advance */
int load_bitwise(char * filename,
                 unsigned char * bitwise, int width, int height)
{
    FILE * fp = fopen(filename, "rb");
    if (!fp) return -1;
    if (fread(bitwise, width*height/8, 1, fp) == 0) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}
