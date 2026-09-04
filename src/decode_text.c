/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix decoding from a text string
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

/**
 * \brief replaces a unicode dot character with an ascii character
 *        so that the grid has even spacing.
 *        eg. replace instances of "●" with "O"
 * \param datamatrix_text string containing the datamatrix
 * \param dot_text string containing the unicode dot representation
 * \param result string containing ascii datamatrix
 * \param debug set to true to enable debug
 * \returns 1 if replacements were made
 */
static bool datamatrix_unicode_to_ascii(const char * datamatrix_text,
                                        const char * dot_text,
                                        char * result,
                                        const bool debug)
{
    int dot_text_len = (int)strlen(dot_text);
    int i, j, ctr = 0;
    bool found = false;

    for (i = 0; i < (int)strlen(datamatrix_text); i++) {
        if (i > (int)strlen(datamatrix_text) - dot_text_len) {
            result[ctr++] = datamatrix_text[i];
            continue;
        }

        for (j = 0; j < dot_text_len; j++) {
            if (datamatrix_text[i+j] != dot_text[j]) break;
        }
        if (j != dot_text_len) {
            result[ctr++] = datamatrix_text[i];
        }
        else {
            found = true;
            result[ctr++] = 'O';
            i += dot_text_len-1;
        }
    }
    result[ctr] = 0;
    if (debug) {
        printf("datamatrix_unicode_to_ascii\n%s\n", result);
    }
    return found;
}

/**
 * \brief returns the X and Y dimensions of the datamatrix
 * \param datamatrix_ascii string containing the datamatrix
 * \param debug set to true for debug mode, false otherwise
 * \param dimension_x returned X dimension
 * \param dimension_y returned Y dimension
 * \param decode_step returned step between adjacent cells in the X dimension
 * \param empty_char character used to represent empty space
 */
static void get_text_datamatrix_dimensions(const char * datamatrix_ascii,
        const bool debug,
        int * dimension_x,
        int * dimension_y,
        int * decode_step,
        char * empty_char)
{
    int i, start_i = 0;

    *decode_step = 0;
    *dimension_x = 0;
    *dimension_y = 0;

    /* get the X dimension */
    for (i = 0; i < (int)strlen(datamatrix_ascii); i++) {
        if ((datamatrix_ascii[i] == '\n') ||
                (datamatrix_ascii[i] == '\r')) {
            if (*dimension_x > 0) {
                *decode_step = (i - start_i) / *dimension_x;
                break;
            }
            *dimension_x = 0;
        }
        if (datamatrix_ascii[i] != empty_char[0]) {
            if (i == 0) {
                if (*dimension_x == 0) start_i = i;
                *dimension_x += 2;
            }
            else {
                if (datamatrix_ascii[i-1] == empty_char[0]) {
                    if (*dimension_x == 0) start_i = i;
                    *dimension_x += 2;
                }
            }
        }
    }
    if (debug) {
        printf("decode_step: %d\n", *decode_step);
    }
    if (*decode_step == 0) {
        return;
    }

    /* get the Y dimension */
    int dot_ctr = 0;
    for (i = 0; i < (int)strlen(datamatrix_ascii); i++) {
        if ((datamatrix_ascii[i] == '\n') ||
                (datamatrix_ascii[i] == '\r') ||
                (i == (int)strlen(datamatrix_ascii)-1)) {
            if (dot_ctr > 0) {
                *dimension_y = *dimension_y + 1;
                dot_ctr = 0;
            }
        }
        else {
            if (datamatrix_ascii[i] != empty_char[0]) dot_ctr++;
        }
    }
    if (debug) {
        printf("dimension_x: %d\ndimension_y: %d\n",
               *dimension_x, *dimension_y);
    }
}

/**
 * \brief populate the occupancy grid
 * \param occupancy occupancy grid
 * \param dimension_x X dimension of the datamatrix
 * \param dimension_y Y dimension of the datamatrix
 * \param decode_step step between adjacent cells in the X dimension
 * \param datamatrix_ascii string containing the datamatrix
 * \param empty_char character or string used to represent empty space
 */
static void text_datamatrix_populate_occupancy(bool occupancy[],
        const int dimension_x,
        const int dimension_y,
        const int decode_step,
        const char * datamatrix_ascii,
        const char * empty_char)
{
    int i, j, start_i = -1;
    int x_pos, y_pos=0;

    /* clear the grid */
    memset(occupancy, false,
           (size_t)(dimension_x * dimension_y) * sizeof(unsigned char));

    /* for each line of the datamatrix string */
    for (i = 0; i < (int)strlen(datamatrix_ascii); i++) {
        if ((datamatrix_ascii[i] == '\n') ||
                (datamatrix_ascii[i] == '\r')) {
            start_i = -1;
            continue;
        }
        if (datamatrix_ascii[i] != empty_char[0]) {
            if (start_i == -1) {
                start_i = i;
                x_pos = 0;
                for (j = i; j < i + (decode_step * dimension_x);
                        j += decode_step, x_pos++) {
                    if (datamatrix_ascii[j] == empty_char[0]) {
                        occupancy[(y_pos * dimension_x) + x_pos] = false;
                    }
                    else {
                        occupancy[(y_pos * dimension_x) + x_pos] = true;
                    }
                }
                y_pos++;
            }
        }
    }
}

/**
 * \brief decode a string containing a datamatrix
 * \param datamatrix_text string containing a datamatrix
 * \param gs1_url optional GS1 URL prefix
 * \param custom_dot_char character or string representing a dot
 * \param custom_empty_char character or string representing a space
 * \param debug set to true for debug mode, false otherwise
 * \returns 0 on success, -1 otherwise
 */
int decode_datamatrix_from_text(const char * datamatrix_text,
                                const char * gs1_url,
                                char * custom_dot_char,
                                char * custom_empty_char,
                                const bool debug)
{
    char datamatrix_ascii[MAX_DECODE_STRING_LENGTH];
    char empty_char[5];
    char * dot_strings[] = {
        "●", "█", "⦁", "•", "⚫"
    };
    const int no_of_dot_strings = 5;
    int i, dimension_x=0, dimension_y=0, decode_step=0;

    sprintf(&empty_char[0], " ");
    if (((int)strlen(custom_empty_char) > 0) &&
            ((int)strlen(custom_empty_char) <= 3)) {
        sprintf(&empty_char[0], "%s", custom_empty_char);
    }

    if (((int)strlen(custom_dot_char) > 0) &&
            ((int)strlen(custom_dot_char) <= 3)) {
        /* using custom dot character */
        datamatrix_unicode_to_ascii(datamatrix_text,
                                    custom_dot_char,
                                    &datamatrix_ascii[0],
                                    debug);
    }
    else {
        for (i = 0; i < no_of_dot_strings; i++) {
            char * dot_text = dot_strings[i];
            if (datamatrix_unicode_to_ascii(datamatrix_text,
                                            dot_text,
                                            &datamatrix_ascii[0],
                                            debug)) break;
        }
    }

    get_text_datamatrix_dimensions(&datamatrix_ascii[0],
                                   debug,
                                   &dimension_x,
                                   &dimension_y,
                                   &decode_step,
                                   &empty_char[0]);
    if ((dimension_x == 0) || (dimension_y == 0) ||
            (decode_step == 0)) return -1;

    /* make an occupancy grid */
    bool * occupancy =
        (bool*)safemalloc((size_t)(dimension_x * dimension_y) *
                          sizeof(bool));
    if (occupancy == nullptr) return -1;

    text_datamatrix_populate_occupancy(occupancy,
                                       dimension_x, dimension_y,
                                       decode_step,
                                       &datamatrix_ascii[0],
                                       &empty_char[0]);

    /* decode the result */
    struct grid_2d grid;
    char * decode_result =
        (char*)safemalloc((size_t)MAX_DECODE_LENGTH * sizeof(char));
    if (decode_result == nullptr) return -1;
    bool human_readable = true;
    decode_result[0] = 0;
    /* decode */
    create_grid_from_pattern(dimension_x, dimension_y, &grid, occupancy);
    datamatrix_decode(&grid, debug, gs1_url, decode_result,
                      human_readable);
    if (decode_result[0] == 0) {
        printf("Could not decode\n");
        free(decode_result);
        free(occupancy);
        return -1;
    }
    printf("%s\n", decode_result);
    free(decode_result);
    free(occupancy);
    return 0;
}
