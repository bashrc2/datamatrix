/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Dot peen fonts
 *  Copyright (c) 2026, Bob Mottram
 *  bob@libreserver.org
 *
 *  This is partly based on, or inspired by, an ECC kernel module
 *  by Fabrice Bellard <fabrice.bellard@netgem.com>
 *  "ECC code for correcting errors detected by DiskOnChip 2000 and
 *  Millennium ECC hardware" written in 2000 under GPLv2 license
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

#define FONT_WIDTH  5
#define FONT_HEIGHT 10

char * font_upper[] = {
    "  ●  ",
    " ● ● ",
    "●   ●",
    "●   ●",
    "●●●●●",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",

    "●●●● ",
    "●   ●",
    "●   ●",
    "●●●● ",
    "●   ●",
    "●   ●",
    "●●●● ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●    ",
    "●    ",
    "●    ",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "●●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●●●● ",
    "     ",
    "     ",
    "     ",

    "●●●●●",
    "●    ",
    "●    ",
    "●●●● ",
    "●    ",
    "●    ",
    "●●●●●",
    "     ",
    "     ",
    "     ",

    "●●●●●",
    "●    ",
    "●    ",
    "●●●● ",
    "●    ",
    "●    ",
    "●    ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●    ",
    "● ●●●",
    "●   ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●   ●",
    "●   ●",
    "●●●●●",
    "●   ●",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    " ●●●●",
    "   ● ",
    "   ● ",
    "   ● ",
    "   ● ",
    "●  ● ",
    " ●●  ",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●  ● ",
    "● ●  ",
    "●●   ",
    "● ●  ",
    "●  ● ",
    "●   ●",
    "     ",
    "     ",
    "     ",

    "●    ",
    "●    ",
    "●    ",
    "●    ",
    "●    ",
    "●    ",
    "●●●●●",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●● ●●",
    "● ● ●",
    "● ● ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●●  ●",
    "● ● ●",
    "●  ●●",
    "●   ●",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "●●●● ",
    "●   ●",
    "●   ●",
    "●●●● ",
    "●    ",
    "●    ",
    "●    ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●● ",
    "   ● ",
    "    ●",
    "     ",

    "●●●● ",
    "●   ●",
    "●   ●",
    "●●●● ",
    "● ●  ",
    "●  ● ",
    "●   ●",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●    ",
    " ●●● ",
    "    ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "●●●●●",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",
    
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    " ● ● ",
    "  ●  ",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "● ● ●",
    "● ● ●",
    " ● ● ",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●   ●",
    " ● ● ",
    "  ●  ",
    " ● ● ",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",

    "●   ●",
    "●   ●",
    " ● ● ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "     ",
    "     ",
    "     ",

    "●●●●●",
    "    ●",
    "   ● ",
    "  ●  ",
    " ●   ",
    "●    ",
    "●●●●●",
    "     ",
    "     ",
    "     "
};

char * font_lower[] = {
    "     ",
    "     ",
    " ●●● ",
    "    ●",
    " ●●●●",
    "●   ●",
    " ●●●●",
    "     ",
    "     ",
    "     ",

    "●    ",
    "●    ",
    "●●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●●●● ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    " ●●● ",
    "●   ●",
    "●    ",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "    ●",
    "    ●",
    " ●●●●",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●●●",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    " ●●● ",
    "●   ●",
    "●●●●●",
    "●    ",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "  ●● ",
    " ●  ●",
    " ●   ",
    "●●●  ",
    " ●   ",
    " ●   ",
    " ●   ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    " ●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●●●",
    "    ●",
    "●   ●",
    " ●●● ",
    
    "●    ",
    "●    ",
    "● ●● ",
    "●●  ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",

    "  ●  ",
    "     ",
    " ●●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "  ●  ",
    "     ",
    " ●●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "● ●  ",
    " ●   ",

    "●    ",
    "●    ",
    "●   ●",
    "●  ● ",
    "●●●  ",
    "●  ● ",
    "●   ●",
    "     ",
    "     ",
    "     ",

    " ●●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "●● ● ",
    "● ● ●", 
    "● ● ●", 
    "● ● ●", 
    "● ● ●", 
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "● ●● ",
    "●●  ●",
    "●   ●",
    "●   ●",
    "●   ●",
    "     ",
    "     ",
    "     ",
    
    "     ",
    "     ",
    " ●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    " ●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●●●● ",
    "●    ",
    "●    ",
    "●    ",

    "     ",
    "     ",
    " ●●● ",
    "●   ●",
    "●   ●",
    "●   ●",
    " ●●●●",
    "    ●",
    "    ●",
    "    ●",

    "     ",
    "     ",
    "● ●● ",
    "●●  ●",
    "●    ",
    "●    ",
    "●    ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    " ●●●●",
    "●    ",
    " ●●● ",
    "    ●",
    "●●●● ",
    "     ",
    "     ",
    "     ",

    " ●   ",
    " ●   ",
    "●●●● ",
    " ●   ",
    " ●   ",
    " ●   ",
    "  ●● ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●  ●●",
    " ●● ●",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "●   ●",
    "●   ●",
    "●   ●",
    " ● ● ",
    "  ●  ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "●   ●",
    "●   ●",
    "● ● ●",
    "● ● ●",
    " ● ● ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "●   ●",
    " ● ● ",
    "  ●  ",
    " ● ● ",
    "●   ●",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "●   ●",
    "●   ●",
    "●   ●",
    "●  ●●",
    " ●● ●",
    "    ●",
    "●   ●",
    " ●●● ",

    "     ",
    "     ",
    "●●●●●",
    "   ● ",
    "  ●  ",
    " ●   ",
    "●●●●●",
    "     ",
    "     ",
    "     "
};

char * font_numbers[] = {
    " ●●● ",
    "●   ●",
    "●  ●●",
    "● ● ●",
    "●●  ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "  ●  ",
    " ●●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "    ●",
    "   ● ",
    "  ●  ",
    " ●   ",
    "●●●●●",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "    ●",
    "  ●● ",
    "    ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "   ● ",
    "  ●● ",
    " ● ● ",
    "●  ● ",
    "●●●●●",
    "   ● ",
    "   ● ",
    "     ",
    "     ",
    "     ",

    "●●●●●",
    "●    ",
    "●●●● ",
    "    ●",
    "    ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●    ",
    "●●●● ",
    "●   ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    "●●●●●",
    "    ●",
    "   ● ",
    "  ●  ",
    " ●   ",
    "●    ",
    "●    ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●   ●",
    " ●●● ",
    "●   ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     ",

    " ●●● ",
    "●   ●",
    "●   ●",
    " ●●●●",
    "    ●",
    "●   ●",
    " ●●● ",
    "     ",
    "     ",
    "     "
};

char * font_misc_lookup = " .,:/-()[]";
char * font_misc[] = {
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    " ●●  ",
    " ●●  ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    " ●●  ",
    " ●●  ",
    "●    ",
    "     ",
    "     ",

    "     ",
    " ●●  ",
    " ●●  ",
    "     ",
    " ●●  ",
    " ●●  ",
    "     ",
    "     ",
    "     ",
    "     ",

    "    ●",
    "    ●",
    "   ● ",
    "  ●  ",
    " ●   ",
    "●    ",
    "●    ",
    "     ",
    "     ",
    "     ",

    "     ",
    "     ",
    "     ",
    "●●●●●",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",
    "     ",

    "   ● ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "   ● ",
    "     ",
    "     ",
    "     ",

    " ●   ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    " ●   ",
    "     ",
    "     ",
    "     ",


    "  ●● ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●● ",
    "     ",
    "     ",
    "     ",

    " ●●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    "  ●  ",
    " ●●  ",
    "     ",
    "     ",
    "     ",
};

/**
 * \brief Returns the character matrix for the given character
 * \param chr character to be returned
 * \param dotmatrix returned matrix containing the character dot pattern
 */
static void lookup_character(char chr, char dotmatrix[])
{
    int offset, w, h, i;

    if ((chr >= '0') && (chr <= '9')) {
        offset = FONT_HEIGHT * (int)(chr - '0');
        i = 0;
        for (h = 0; h < FONT_HEIGHT; h++) {
            for (w = 0; w < FONT_WIDTH; w++, i++) {
                dotmatrix[i] = font_numbers[h + offset][w];
            }
        }
    }
    else if ((chr >= 'A') && (chr <= 'Z')) {
        offset = FONT_HEIGHT * (int)(chr - 'A');
        i = 0;
        for (h = 0; h < FONT_HEIGHT; h++) {
            for (w = 0; w < FONT_WIDTH; w++, i++) {
                dotmatrix[i] = font_upper[h + offset][w];
            }
        }
    }
    else if ((chr >= 'a') && (chr <= 'z')) {
        offset = FONT_HEIGHT * (int)(chr - 'a');
        i = 0;
        for (h = 0; h < FONT_HEIGHT; h++) {
            for (w = 0; w < FONT_WIDTH; w++, i++) {
                dotmatrix[i] = font_lower[h + offset][w];
            }
        }
    }
    else {
        int ctr = 0;
        for (ctr = 0; ctr < (int)strlen(font_misc_lookup); ctr++) {
            if (chr == font_misc_lookup[ctr]) {
                offset = FONT_HEIGHT * ctr;
                i = 0;
                for (h = 0; h < FONT_HEIGHT; h++) {
                    for (w = 0; w < FONT_WIDTH; w++, i++) {
                        dotmatrix[i] = font_misc[h + offset][w];
                    }
                }
                return;
            }
        }

        i = 0;
        for (h = 0; h < FONT_HEIGHT; h++) {
            for (w = 0; w < FONT_WIDTH; w++, i++) {
                dotmatrix[i] = font_misc[h][w];
            }
        }
    }
}

/**
 * \brief Draws a character within the given bounding box
 * \param img Array containing image
 * \param width Width of the image
 * \param height Height of the image
 * \param bitsperpixel Number of bits per pixel
 * \param tx top left corner of the bounding box
 * \param ty top corner of the bounding box
 * \param bx bottom right  corner of the bounding box
 * \param by bottom  corner of the bounding box
 * \param r Red
 * \param g Green
 * \param b Blue
 * \param chr character to be returned
 */
void draw_character(unsigned char img[],
                    unsigned int width, unsigned int height,
                    int bitsperpixel,
                    int tx, int ty, int bx, int by,
                    int r, int g, int b,
                    char chr)
{
    int dx = bx - tx;
    int dy = by - ty;
    int dot_radius = dx / (FONT_WIDTH*2);
    char dotmatrix[FONT_WIDTH*FONT_HEIGHT];

    lookup_character(chr, &dotmatrix[0]);
    
    for (int y = 0; y < FONT_HEIGHT; y++) {
        for (int x = 0; x < FONT_WIDTH; x++) {
            if (dotmatrix[y*FONT_WIDTH + x] != ' ') {
                int centre_x = tx + dot_radius + (x * dx / FONT_WIDTH);
                int centre_y = ty + dot_radius + (y * dy / FONT_HEIGHT);

                draw_dot(img, width, height, bitsperpixel,
                         centre_x, centre_y, dot_radius,
                         r, g, b);
            }
        }
    }
}
