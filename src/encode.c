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
 * \param square_modules draw with square shaped modules
 */
static void encode_image_dot(unsigned char img[], int width, int height,
                             int bytes_per_pixel,
                             int dot_x, int dot_y, int dot_radius,
                             unsigned char square_modules)
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
            if ((square_modules == 0) &&
                    (SQUARE_MAG(dx, dy) > dot_radius_sqr)) continue;

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
 * \param square_modules draw with square shaped modules
 * \param tx Bounding box top left x coordinate
 * \param ty Bounding box top left y coordinate
 * \param bx Bounding box bottom right x coordinate
 * \param by Bounding box bottom right y coordinate
 */
static void encode_image_base(unsigned char img[], int width, int height,
                              int bitsperpixel, unsigned char *grid,
                              unsigned int encode_width, unsigned int encode_height,
                              unsigned char square_modules,
                              int tx, int ty, int bx, int by)
{
    unsigned int x, y;
    int bytes_per_pixel = bitsperpixel/8;
    int dot_x, dot_y;
    int bounding_box_width = bx - tx;
    int bounding_box_height = by - ty;
    int half_cell_width = bounding_box_width / ((int)encode_width * 2);
    int half_cell_height = bounding_box_height / ((int)encode_height * 2);
    int dot_radius = half_cell_width * 8 / 10;

    if (square_modules != 0) {
        dot_radius = half_cell_width + 1;
    }

    /* clear the image */
    memset(img, 255, width*height*bytes_per_pixel*sizeof(unsigned char));

    /* draw dots */
    for (y = 0; y < encode_height; y++) {
        dot_y = ty + ((int)y * bounding_box_height / (int)encode_height) + half_cell_height;
        for (x = 0; x < encode_width; x++) {
            if (!grid[encode_width * y + x]) continue;
            dot_x = tx + ((int)x * bounding_box_width / (int)encode_width) + half_cell_width;
            encode_image_dot(img, width, height, bytes_per_pixel,
                             dot_x, dot_y, dot_radius, square_modules);
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
 * \param square_modules draw with square shaped modules
 * \param description formatted description accompanying the datamatrix
 * \param description_position Position of the formatted description
 * \param character_width Width of each description character in pixels
 * \param line spacing Spacing between description lines in pixels
 * \param character_separation Horizontal separation between characters in pixels
 */
void encode_image(unsigned char img[], int width, int height,
                  int bitsperpixel, unsigned char *grid,
                  unsigned int encode_width, unsigned int encode_height,
                  unsigned char square_modules,
                  char * description,
                  unsigned char description_position,
                  int character_width,
                  int line_spacing,
                  int character_separation)
{
    /* bounding box for the datamatrix pattern within the image */
    int pattern_tx = 0;
    int pattern_ty = 0;
    int pattern_bx = width;
    int pattern_by = height;

    /* get the maximum width of the description in characters */
    int max_description_width = description_text_width(description);
    int lines = description_text_lines(description);
    /* get the width and height of the box containing the description
       in pixels */
    int character_height = character_width * FONT_HEIGHT / FONT_WIDTH;
    int description_width = max_description_width * character_width;
    int description_height =
        ((lines+1) * character_height) + ((lines-1) * line_spacing);

    /* bounding box for the description */
    int text_tx = -1;
    int text_ty = -1;
    int text_len = (int)strlen(description);
    if (text_len > 0) {
        switch(description_position) {
        case DESCRIPTION_BELOW: {
            /* separation between datamatrix and description in pixels */
            int separation = line_spacing;
            int available_height = height - description_height - separation;

            pattern_tx = 0;
            pattern_ty = 0;
            if (encode_width == encode_height) {
                /* square datamatrix */
                if (available_height < width) {
                    pattern_tx = (width - available_height) / 2;
                    pattern_bx = pattern_tx + available_height;
                }
                else {
                    pattern_ty = (available_height - width) / 2;
                    pattern_by = pattern_ty + width;
                }
                pattern_by = available_height;
                text_ty = height - description_height;
            }
            else {
                /* rectangular datamatrix */
                pattern_tx = 0;
                pattern_ty = 0;
                pattern_bx = width;
                pattern_by = width * encode_height / encode_width;
                text_ty = pattern_by + separation;
                if (height - pattern_by > description_height) {
                    text_ty += ((height - pattern_by) - description_height)/2;
                }
            }
            /* description shown below the datamatrix */
            text_tx = (width - description_width)/2;
            break;
        }
        case DESCRIPTION_ABOVE: {
            /* separation between datamatrix and description in pixels */
            int separation = line_spacing;
            int available_height = height - description_height - separation;

            pattern_tx = 0;
            pattern_ty = 0;
            if (encode_width == encode_height) {
                /* square datamatrix */
                if (available_height < width) {
                    pattern_tx = (width - available_height) / 2;
                    pattern_bx = pattern_tx + available_height;
                }
                else {
                    pattern_ty = (available_height - width) / 2;
                    pattern_by = pattern_ty + width;
                }
                pattern_by = available_height;
                pattern_ty = height - (pattern_by - pattern_ty);
                pattern_by = height;
            }
            else {
                /* rectangular datamatrix */
                pattern_tx = 0;
                pattern_bx = width;
                pattern_ty = width * encode_height / encode_width;
                pattern_by = height;
            }
            /* description shown above the datamatrix */
            text_tx = (width - description_width)/2;
            text_ty = (pattern_ty/2) - (description_height/2) + character_height;
            break;
        }
        case DESCRIPTION_RIGHT: {
            pattern_tx = 0;
            pattern_ty = 0;
            pattern_bx = width/2;
            pattern_by = height;
            text_tx = pattern_bx + character_width;
            text_ty = (height/2) - (description_height/2) + character_height;
            break;
        }
        case DESCRIPTION_LEFT: {
            pattern_tx = width/2;
            pattern_ty = 0;
            pattern_bx = width;
            pattern_by = height;
            text_tx = character_width;
            text_ty = (height/2) - (description_height/2) + character_height;
            break;
        }
        }
    }

    encode_image_base(img, width, height, bitsperpixel, grid,
                      encode_width, encode_height,
                      square_modules,
                      pattern_tx, pattern_ty,
                      pattern_bx, pattern_by);
    if (text_tx == -1) return;
    draw_text(img, width, height, bitsperpixel,
              text_tx, text_ty, character_width, line_spacing,
              character_separation, 0, 0, 0, description);
}

/**
 * \brief returns an SVG image from the given datamatrix grid
 * \param image_filename filename of the svg image to be saved
 * \param width width of the image
 * \param height height of the image
 * \param encode_width width of the datamatrix grid
 * \param encode_height height of the datamatrix grid
 * \param square_modules draw with square shaped modules
 */
void encode_svg(char * image_filename, int width, int height,
                unsigned char *grid,
                unsigned int encode_width, unsigned int encode_height,
                unsigned char square_modules)
{
    unsigned int x, y;
    int dot_x, dot_y;
    int half_cell_width = width / ((int)encode_width * 2);
    int half_cell_height = height / ((int)encode_height * 2);
    int dot_radius = half_cell_width * 8 / 10;
    FILE * fp_image;

    if (square_modules != 0) {
        dot_radius = half_cell_width + 1;
    }

    fp_image = fopen(image_filename, "w");
    if (fp_image == NULL) return;
    fprintf(fp_image,
            "<svg height=\"%d\" width=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n",
            width, height);

    /* draw dots */
    for (y = 0; y < encode_height; y++) {
        dot_y = ((int)y * height / (int)encode_height) + half_cell_height;
        for (x = 0; x < encode_width; x++) {
            if (!grid[encode_width * y + x]) continue;
            dot_x = ((int)x * width / (int)encode_width) + half_cell_width;
            if (square_modules == 0) {
                fprintf(fp_image,
                        "<circle r=\"%d\" cx=\"%d\" cy=\"%d\" fill=\"black\" />\n",
                        dot_radius, dot_x, dot_y);
            }
            else {
                fprintf(fp_image,
                        "<rect width=\"%d\" height=\"%d\" x=\"%d\" y=\"%d\" rx=\"0\" ry=\"0\" fill=\"black\" />\n",
                        dot_radius*2, dot_radius*2,
                        dot_x - dot_radius, dot_y - dot_radius);
            }
        }
    }
    fprintf(fp_image, "</svg>\n");
    fclose(fp_image);
}
