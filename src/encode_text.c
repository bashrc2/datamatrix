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
 * \param square_modules draw with square shaped modules
 * \param description formatted description accompanying the datamatrix
 * \param description_position Position of the formatted description
 * \param character_width Width of each description character in pixels
 * \param line spacing Spacing between description lines in pixels
 * \param character_separation Horizontal separation between characters in pixels
 * \returns 0 on success, -1 otherwise
 */
static int encode_datamatrix_to_image(char * image_filename,
                                      unsigned char * grid,
                                      const int encode_width,
                                      const int encode_height,
                                      const int encode_image_width,
                                      const int encode_image_height,
                                      const bool square_modules,
                                      const char * description,
                                      const unsigned char description_position,
                                      const int character_width,
                                      const int line_spacing,
                                      const int character_separation)
{
    int image_filename_length = (int)strlen(image_filename);
    /* check that the output image filename is long enough */
    if (image_filename_length < 4) {
        printf("Output filename too short.\n");
        free(grid);
        return -1;
    }

    /* check that the output image filename is png format */
    if (ends_with(image_filename, ".png")) {
        unsigned char * encode_image_data =
            (unsigned char*)safemalloc((size_t)(encode_image_width *
                                                encode_image_height * 3));
        encode_image(encode_image_data,
                     encode_image_width, encode_image_height, 24,
                     grid, encode_width, encode_height,
                     square_modules,
                     description,
                     description_position,
                     character_width,
                     line_spacing, character_separation);
        write_png_file(image_filename,
                       encode_image_width, encode_image_height, 24,
                       encode_image_data);

        free(grid);
    }
    else if (ends_with(image_filename, ".svg")) {
        encode_svg(image_filename,
                   encode_image_width, encode_image_height,
                   grid, encode_width, encode_height,
                   square_modules,
                   description,
                   description_position,
                   character_width,
                   line_spacing, character_separation);
        free(grid);
    }
    else {
        printf("Output filename must be png format.\n");
        free(grid);
        return -1;
    }
    return 0;
}

/**
 * \brief encode text into a datamatrix as text or an image
 * \param text the text to be encoded
 * \param description a formatted description to appear alongside
 *        or underneath the datamatrix pattern
 * \param description_position Position of the formatted description
 * \param character_width Width of each description character in pixels
 * \param line spacing Spacing between description lines in pixels
 * \param encode_scale Scaling factor for text datamatrix output
 * \param is_square true if the datamatrix should be square
 * \param csv true if output should be in CSV format
 * \param show_coords true if the output should be a list of dot coordinates
 * \param coords_offset_x X offset added to dot coordinates
 * \param coords_offset_y Y offset added to dot coordinates
 * \param image_filename filename to save datamatrix image to
 * \param encode_image_width Width of the datamatrix image
 * \param dot_char character or string representing a dot
 * \param empty_char character or string representing a space
 * \param square_modules draw with square shaped modules
 * \param debug 1 to show debug, 0 otherwise
 * \returns 0 on success, -1 otherwise
 */
int encode_datamatrix_to_text_or_image(const char * text,
                                       const char * description,
                                       const unsigned char description_position,
                                       const int character_width,
                                       const int line_spacing,
                                       const int encode_scale,
                                       const bool is_square,
                                       const bool csv,
                                       const bool show_coords,
                                       const float coords_offset_x,
                                       const float coords_offset_y,
                                       char * image_filename,
                                       int encode_image_width,
                                       const char * dot_char,
                                       const char * empty_char,
                                       const bool square_modules,
                                       const bool debug)
{
    char * encoding = nullptr;
    unsigned int barcodelen = 0;
    unsigned char *grid = 0;
    unsigned int encode_width = 0, encode_height = 0;
    unsigned int len = 0,
        maxlen = 0,
        encode_ecclen = 0;
    bool square = false;
    bool noquiet = false;
    /* a small horizontal separation between characters so that they don't
       appear joined together */
    int character_separation = character_width / FONT_WIDTH;

    barcodelen = (unsigned int)strlen(text);

    /* force square shape? */
    if (is_square) square = true;

    /* csv output has no quiet zone */
    if (csv == true) noquiet = true;

    grid = iec16022ecc200(&encode_width, &encode_height,
                          &encoding, barcodelen,
                          (unsigned char *)text, &len,
                          &maxlen, &encode_ecclen,
                          square, noquiet);
    if (debug)
        printf("encoded: '%s' %dx%d\n",
               text, encode_width, encode_height);
    /* show the datamatrix */
    int encode_image_height = \
        UINT_TO_INT((unsigned int)encode_image_width *
                    encode_height / encode_width);

    if (description[0] != 0) {
        /* allow extra height for description of rectangular datamatrix */
        if ((encode_width != encode_height) &&
                ((description_position == DESCRIPTION_ABOVE) ||
                 (description_position == DESCRIPTION_BELOW))) {
            encode_image_height *= 2;
        }
        /* allow extra width for description on either side */
        if ((description_position == DESCRIPTION_LEFT) ||
                (description_position == DESCRIPTION_RIGHT)) {
            encode_image_width *= 2;
        }
    }

    int encode_width_int = UINT_TO_INT(encode_width);
    int encode_height_int = UINT_TO_INT(encode_height);
    if (grid && (image_filename[0] != 0)) {
        return encode_datamatrix_to_image(image_filename,
                                          grid,
                                          encode_width_int,
                                          encode_height_int,
                                          encode_image_width,
                                          encode_image_height,
                                          square_modules,
                                          description,
                                          description_position,
                                          character_width,
                                          line_spacing,
                                          character_separation);
    }

    /* encode as text */
    int S = encode_scale;
    int x, y, x_directional;
    char dot_chr[8];
    char empty_chr[8];

    if ((description[0] != 0) && (description_position == DESCRIPTION_ABOVE)) {
        printf("%s", description);
    }

    if (!square_modules) {
        /* round */
        sprintf(&dot_chr[0], "%s%s", dot_char, empty_char);
    }
    else {
        /* square */
        sprintf(&dot_chr[0], "%s%s", dot_char, dot_char);
        S = 1;
    }
    sprintf(&empty_chr[0], "%s%s", empty_char, empty_char);
    if (S > 1) {
        sprintf(&dot_chr[0], "█");
        sprintf(&empty_chr[0], "%s", empty_char);
    }
    if (csv == true) {
        S = 1;
        sprintf(&dot_chr[0], "1,");
        sprintf(&empty_chr[0], "0,");
    }
    float x_coord, y_coord;
    bool direction = false;
    if (show_coords) S = 1;
    for (y = 0; y < encode_height_int * S; y++) {
        for (x = 0; x < encode_width_int * S; x++) {
            if (!show_coords) {
                printf("%s",
                       grid[encode_width_int *
                                         (y / S) + (x / S)] ? dot_chr : empty_chr);
            }
            else {
                /* show dot coordinates */
                x_directional = x;
                if (direction) x_directional = encode_width_int - 1 - x;
                if (grid[encode_width_int * y + x_directional]) {
                    x_coord =
                        coords_offset_x +
                        ((float)(x_directional * encode_image_width) /
                         (float)encode_width);
                    y_coord =
                        coords_offset_y +
                        ((float)(y * encode_image_height) /
                         (float)encode_height);
                    printf("%.3f, %.3f,\n", x_coord, y_coord);
                }
            }
        }
        if (!show_coords) printf("\n");
        /* change direction for each row */
        direction = !direction;
    }

    if ((description[0] != 0) && (description_position == DESCRIPTION_BELOW)) {
        printf("%s", description);
    }

    if (grid) {
        free(grid);
        return 0;
    }
    return -1;
}
