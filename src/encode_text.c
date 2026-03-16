/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix encoding a text string to a datamatrix
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

#include <stdio.h>
#include <stdlib.h>
#include "png2.h"
#include "datamatrix.h"
#include "iec16022ecc200.h"

/**
 * \brief encode the datamatrix grid as an image
 * \param image_filename image filename
 * \param grid grid containing the datamatrix
 * \param encode_width width of the grid
 * \param encode_height height of the grid
 * \param encode_image_width width of the datamatrix image
 * \param encode_image_height height of the datamatrix image
 * \returns 0 on success, -1 otherwise
 */
static int encode_datamatrix_to_image(char * image_filename,
                                      unsigned char * grid,
                                      unsigned int encode_width,
                                      unsigned int encode_height,
                                      int encode_image_width,
                                      int encode_image_height)
{
    int image_filename_length = strlen(image_filename);
    /* check that the output image filename is long enough */
    if (image_filename_length < 4) {
        printf("Output filename too short.\n");
        free(grid);
        return -1;
    }

    /* check that the output image filename is png format */
    if ((image_filename[image_filename_length-4] != '.') ||
        (image_filename[image_filename_length-3] != 'p') ||
        (image_filename[image_filename_length-2] != 'n') ||
        (image_filename[image_filename_length-1] != 'g')) {
        printf("Output filename must be png format.\n");
        free(grid);
        return -1;
    }
    unsigned char * encode_image_data =
        (unsigned char*)safemalloc(encode_image_width*
                                   encode_image_height*3);
    encode_image(encode_image_data,
                 encode_image_width, encode_image_height, 24,
                 grid, encode_width, encode_height);
    write_png_file(image_filename,
                   encode_image_width, encode_image_height, 24,
                   encode_image_data);

    free(grid);
    return 0;
}

/**
 * \brief encode text into a datamatrix as text or an image
 * \param text the text to be encoded
 * \param encode_scale Scaling factor for text datamatrix output
 * \param is_square 1 if the datamatrix should be square
 * \param csv 1 if output should be in CSV format
 * \param show_coords 1 if the output should be a list of dot coordinates
 * \param coords_offset_x X offset added to dot coordinates
 * \param coords_offset_y Y offset added to dot coordinates
 * \param image_filename filename to save datamatrix image to
 * \param encode_image_width Width of the datamatrix image
 * \param dot_char character or string representing a dot
 * \param empty_char character or string representing a space
 * \param debug 1 to show debug, 0 otherwise
 * \returns 0 on success, -1 otherwise
 */
int encode_datamatrix_to_text(char * text,
                              int encode_scale,
                              unsigned char is_square,
                              unsigned char csv,
                              unsigned char show_coords,
                              float coords_offset_x,
                              float coords_offset_y,
                              char * image_filename,
                              int encode_image_width,
							  char * dot_char,
							  char * empty_char,
                              unsigned char debug)
{
    char * encoding = NULL;
    int barcodelen = 0;
    unsigned char *grid = 0;
    unsigned int encode_width = 0, encode_height = 0;
    unsigned int len = 0,
        maxlen = 0,
        encode_ecclen = 0,
        square = 0,
        noquiet = 0;

    barcodelen = strlen(text);

    /* force square shape? */
    if (is_square == 1) square = 1;

    /* csv output has no quiet zone */
    if (csv == 1) noquiet = 1;

    grid = iec16022ecc200(&encode_width, &encode_height,
                          &encoding, barcodelen,
                          (unsigned char *)text, &len,
                          &maxlen, &encode_ecclen, square, noquiet);
    if (debug == 1)
        printf("encoded: '%s' %dx%d\n",
               text, encode_width, encode_height);
    /* show the datamatrix */
    int encode_image_height = \
        encode_image_width * encode_height / encode_width;

    if (grid && (image_filename[0] != 0)) {
        return encode_datamatrix_to_image(image_filename,
                                          grid,
                                          encode_width,
                                          encode_height,
                                          encode_image_width,
                                          encode_image_height);
    }

    /* encode as text */
    unsigned int S = encode_scale;
    unsigned int x, y, x_directional;
	char dot_chr[5];
    char empty_chr[5];
    sprintf(&dot_chr[0], "%s%s", dot_char, empty_char);
    sprintf(&empty_chr[0], "%s%s", empty_char, empty_char);
    if (S > 1) {
        sprintf(&dot_chr[0], "█");
        sprintf(&empty_chr[0], "%s", empty_char);
    }
    if (csv == 1) {
        S = 1;
        sprintf(&dot_chr[0], "1,");
        sprintf(&empty_chr[0], "0,");
    }
    float x_coord, y_coord;
    unsigned char direction = 0;
    if (show_coords == 1) S = 1;
    for (y = 0; y < encode_height * S; y++) {
        for (x = 0; x < encode_width * S; x++) {
            if (show_coords == 0) {
                printf("%s",
                       grid[encode_width *
                            (y / S) + (x / S)] ? dot_chr : empty_chr);
            }
            else {
                /* show dot coordinates */
                x_directional = x;
                if (direction == 1) x_directional = encode_width - 1 - x;
                if (grid[encode_width * y + x_directional]) {
                    x_coord =
                        coords_offset_x +
                        (x_directional * encode_image_width /
                         (float)encode_width);
                    y_coord =
                        coords_offset_y +
                        (y * encode_image_height /
                         (float)encode_height);
                    printf("%.3f, %.3f,\n", x_coord, y_coord);
                }
            }
        }
        if (show_coords == 0) printf("\n");
        /* change direction for each row */
        direction = 1 - direction;
    }

    if (grid) {
        free(grid);
        return 0;
    }
    return -1;
}
