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
 * \returns 1 if replacements were made
 */
static int datamatrix_unicode_to_ascii(char * datamatrix_text,
									   char * dot_text,
									   char * result,
									   unsigned char debug)
{
    int dot_text_len = (int)strlen(dot_text);
    int i, j, ctr = 0, found = 0;
    for (i = 0; i < (int)strlen(datamatrix_text); i++) {
        if (i <= (int)strlen(datamatrix_text) - dot_text_len) {
            for (j = 0; j < dot_text_len; j++) {
                if (datamatrix_text[i+j] != dot_text[j]) break;
            }
            if (j != dot_text_len) {
                result[ctr++] = datamatrix_text[i];
            }
            else {
				found = 1;
                result[ctr++] = 'O';
                i += dot_text_len-1;
            }
        }
        else {
            result[ctr++] = datamatrix_text[i];
        }
    }
    result[ctr] = 0;
    if (debug == 1) {
        printf("datamatrix_unicode_to_ascii\n%s\n", result);
    }
	return found;
}

/**
 * \brief returns the X and Y dimensions of the datamatrix
 * \param datamatrix_ascii string containing the datamatrix
 * \param debug set to 1 for debug mode, 0 otherwise
 * \param dimension_x returned X dimension
 * \param dimension_y returned Y dimension
 * \param decode_step returned step between adjacent cells in the X dimension
 */
static void get_text_datamatrix_dimensions(char * datamatrix_ascii,
                                           unsigned char debug,
                                           int * dimension_x,
                                           int * dimension_y,
                                           int * decode_step)
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
        if (datamatrix_ascii[i] != ' ') {
            if (i == 0) {
                if (*dimension_x == 0) start_i = i;
                *dimension_x += 2;
            }
            else {
                if (datamatrix_ascii[i-1] == ' ') {
                    if (*dimension_x == 0) start_i = i;
                    *dimension_x += 2;
                }
            }
        }
    }
    if (debug == 1) {
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
            if (datamatrix_ascii[i] != ' ') dot_ctr++;
        }
    }
    if (debug == 1) {
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
 */
static void text_datamatrix_populate_occupancy(unsigned char occupancy[],
                                               int dimension_x,
                                               int dimension_y,
                                               int decode_step,
                                               char * datamatrix_ascii)
{
    int i, j, start_i = -1;
    int x_pos, y_pos=0;

    /* clear the grid */
    memset(occupancy, 0,
           dimension_x * dimension_y * sizeof(unsigned char));

    /* for each line of the datamatrix string */
    for (i = 0; i < (int)strlen(datamatrix_ascii); i++) {
        if ((datamatrix_ascii[i] == '\n') ||
            (datamatrix_ascii[i] == '\r')) {
            start_i = -1;
            continue;
        }
        if (datamatrix_ascii[i] != ' ') {
            if (start_i == -1) {
                start_i = i;
                x_pos = 0;
                for (j = i; j < i + (decode_step * dimension_x);
                     j += decode_step, x_pos++) {
                    if (datamatrix_ascii[j] == ' ') {
                        occupancy[(y_pos * dimension_x) + x_pos] = 0;
                    }
                    else {
                        occupancy[(y_pos * dimension_x) + x_pos] = 1;
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
 * \param debug set to 1 for debug mode, 0 otherwise
 * \returns 0 on success, -1 otherwise 
 */
int decode_datamatrix_from_text(char * datamatrix_text,
                                char * gs1_url,
                                unsigned char debug)
{
    char datamatrix_ascii[MAX_DECODE_STRING_LENGTH];
	char * dot_strings[] = {
		"●", "⦁", "•", "⚫"
	};
	int no_of_dot_strings = 4;	
    int i, dimension_x=0, dimension_y=0, decode_step=0;

	for (i = 0; i < no_of_dot_strings; i++) {
		char * dot_text = dot_strings[i];
		if (datamatrix_unicode_to_ascii(datamatrix_text,
										dot_text,
										&datamatrix_ascii[0],
										debug) == 1) break;
	}

    get_text_datamatrix_dimensions(&datamatrix_ascii[0],
                                   debug,
                                   &dimension_x,
                                   &dimension_y,
                                   &decode_step);
    if ((dimension_x == 0) || (dimension_y == 0) ||
        (decode_step == 0)) return -1;

    /* make an occupancy grid */
    unsigned char * occupancy =
        (unsigned char*)safemalloc(dimension_x * dimension_y *
                                   sizeof(unsigned char));
    if (occupancy == NULL) return -1;

    text_datamatrix_populate_occupancy(occupancy,
                                       dimension_x, dimension_y,
                                       decode_step,
                                       &datamatrix_ascii[0]);

    /* decode the result */
    struct grid_2d grid;
    char * decode_result = (char*)safemalloc(MAX_DECODE_LENGTH *
                                             sizeof(char));
    if (decode_result == NULL) return -1;
    unsigned char human_readable = 1;
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
