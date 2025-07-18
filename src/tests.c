/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Unit tests
 *  Copyright (c) 2025, Bob Mottram
 *  bob@libreserver.org
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

static void test_rotate()
{
  printf("test_rotate\n");
  struct grid_2d grid;
  int grid_x, grid_y;
  unsigned char occupancy1[] = {
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 1, 0, 0, 1, 0, 0, 1, 0, 1,
    1, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    1, 0, 1, 1, 0, 0, 0, 1, 1, 1,
    1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 0, 1, 1, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };

  unsigned char expected[] = {
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 0, 0, 1, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 1, 0, 1, 1, 1, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
    1, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 1, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };

  int dimension_x = 10;
  int dimension_y = 10;
  create_grid_from_pattern(dimension_x, dimension_y, &grid, occupancy1);

  printf("Not rotated:\n");
  show_grid(&grid);

  rotate_grid(&grid);
  printf("Rotated:\n");
  show_grid(&grid);

  for (grid_y = 0; grid_y < dimension_y; grid_y++) {
    for (grid_x = 0; grid_x < dimension_x; grid_x++) {
      if (grid.occupancy[grid_x][grid_y] != expected[grid_y*dimension_x + grid_x]) {
        printf("Not rotated at %d,%d\n", grid_x, grid_y);
      }
      assert(grid.occupancy[grid_x][grid_y] == expected[grid_y*dimension_x + grid_x]);
    }
  }

  free_grid(&grid);
}

static void test_decode()
{
  printf("test_decode\n");
  unsigned char human_readable = 1;
  int dimension_x, dimension_y;
  struct grid_2d grid, grid2;
  char gs1_url[MAX_DECODE_LENGTH];
  gs1_url[0] = 0;

  char * decode_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  assert(decode_result != NULL);

  unsigned char occupancy1[] = {
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 1, 0, 0, 1, 0, 0, 1, 0, 1,
    1, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    1, 0, 1, 1, 0, 0, 0, 1, 1, 1,
    1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 0, 1, 1, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };
  dimension_x = 10;
  dimension_y = 10;
  create_grid_from_pattern(dimension_x, dimension_y, &grid, occupancy1);
  show_grid(&grid);
  datamatrix_decode(&grid, 1, &gs1_url[0], decode_result, human_readable);
  assert(strlen(decode_result) > 0);
  assert(strcmp(decode_result, "123") == 0);

  free_grid(&grid);

  unsigned char occupancy2[] = {
    1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0,
    1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0,
    1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1,
    1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1,
    1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  };
  dimension_x = 12;
  dimension_y = 12;

  /*
   * Expected translation table
   02 01 02 03 02 06 01 05 01 08 07 02 07 04 07 07 03 01 03 03
   02 02 02 04 02 07 06 01 06 03 06 06 07 05 07 08 03 02 03 04
   03 06 02 05 02 08 06 02 06 04 06 07 08 01 08 03 08 06 03 05
   03 07 05 01 05 03 05 06 06 05 06 08 08 02 08 04 08 07 04 01
   03 08 05 02 05 04 05 07 09 01 09 03 09 06 08 05 08 08 04 02
   04 03 04 06 05 05 05 08 09 02 09 04 09 07 12 01 12 03 12 06
   04 04 04 07 10 01 10 03 10 06 09 05 09 08 12 02 12 04 12 07
   04 05 04 08 10 02 10 04 10 07 11 01 11 03 11 06 12 05 12 08
   01 01 01 03 01 06 10 05 10 08 11 02 11 04 11 07 00 00 00 00
   01 02 01 04 01 07 07 01 07 03 07 06 11 05 11 08 00 00 00 00

   * Current translation table
   02 01 02 03 02 06 01 05 01 08 05 02 05 04 05 07 00 00 00 00
   02 02 02 04 02 07 04 01 04 03 04 06 05 05 05 08 00 00 00 00
   00 00 02 05 02 08 04 02 04 04 04 07 06 01 06 03 06 06 00 00
   00 00 03 01 03 03 03 06 04 05 04 08 06 02 06 04 06 07 00 00
   00 00 03 02 03 04 03 07 07 01 07 03 07 06 06 05 06 08 00 00
   00 00 00 00 03 05 03 08 07 02 07 04 07 07 10 01 10 03 10 06
   00 00 00 00 08 01 08 03 08 06 07 05 07 08 10 02 10 04 10 07
   00 00 00 00 08 02 08 04 08 07 09 01 09 03 09 06 10 05 10 08
   01 01 01 03 01 06 08 05 08 08 09 02 09 04 09 07 00 00 00 00
   01 02 01 04 01 07 05 01 05 03 05 06 09 05 09 08 00 00 00 00

   * Expected codeword pattern

   XX     XX    XX    XX    XX    XX
   XX
   XX  01 01 02 02 02 03 03 03 00 00  XX
   XX  01 01 01 04 04 03 03 03 00 00
   XX  01 01 01 04 04 04 09 09 00 00  XX
   XX  00 05 05 04 04 04 09 09 09 06
   XX  00 05 05 05 08 08 09 09 09 06  XX
   XX  06 05 05 05 08 08 08 0a 0a 06
   XX  06 06 07 07 08 08 08 0a 0a 0a  XX
   XX  06 06 07 07 07 0b 0b 0a 0a 0a
   XX  02 02 07 07 07 0b 0b 0b 00 00  XX
   XX  02 02 02 03 03 0b 0b 0b 00 00
   XX
   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  */

  create_grid_from_pattern(dimension_x, dimension_y, &grid2, occupancy2);
  show_grid(&grid2);
  datamatrix_decode(&grid2, 1, &gs1_url[0], decode_result, human_readable);
  assert(strlen(decode_result) > 0);
  assert(strcmp(decode_result, "Test") == 0);

  free(decode_result);
  free_grid(&grid2);
}

static void test_gs1_decode()
{
  printf("test_gs1_decode\n");
  int human_readable = 1;
  int dimension_x, dimension_y;
  struct grid_2d grid, grid2;
  char gs1_url[MAX_DECODE_LENGTH];
  gs1_url[0] = 0;

  char * decode_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  assert(decode_result != NULL);

  /* example from GS1 General Specifications */
  unsigned char occupancy1[] = {
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
    1,1,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,1,1,1,
    1,0,0,0,1,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,
    1,0,1,1,0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,
    1,1,1,1,0,0,1,0,0,1,0,1,0,1,1,1,1,0,1,0,
    1,0,0,1,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,1,
    1,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,1,1,0,0,
    1,0,0,1,0,0,0,0,0,0,1,1,1,0,0,1,0,0,0,1,
    1,1,0,0,0,1,0,0,1,1,0,1,0,1,1,1,0,1,0,0,
    1,1,0,1,1,1,1,1,0,1,0,0,0,0,1,0,0,1,0,1,
    1,0,0,1,1,0,1,0,0,1,0,1,0,1,0,1,1,0,0,0,
    1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,1,1,
    1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,1,0,0,
    1,1,0,1,1,1,0,0,0,0,1,0,1,1,0,0,1,1,1,1,
    1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,
    1,1,0,1,0,0,0,1,0,1,0,0,1,1,1,0,1,0,0,1,
    1,1,0,0,0,1,0,1,0,0,0,0,0,1,1,1,0,1,1,0,
    1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,
    1,0,0,0,1,0,1,1,1,0,0,1,0,1,0,0,1,0,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  };
  dimension_x = 20;
  dimension_y = 20;
  create_grid_from_pattern(dimension_x, dimension_y, &grid, occupancy1);
  show_grid(&grid);
  datamatrix_decode(&grid, 1, &gs1_url[0], decode_result, human_readable);
  assert(strlen(decode_result) > 0);
  printf("%s\n", decode_result);
  assert(strcmp(decode_result,
                "STANDARD: GS1\nGTIN: 00068780000108\nPACK DATE: 31 Dec 2030\nBATCH/LOT: ABC123") == 0);

  free_grid(&grid);

  /* try with gs1 digital link url */
  decode_result[0] = 0;
  decode_strcat(&gs1_url[0], "https://test.domain");
  create_grid_from_pattern(dimension_x, dimension_y, &grid2, occupancy1);
  show_grid(&grid2);
  datamatrix_decode(&grid2, 1, &gs1_url[0], decode_result, human_readable);
  assert(strlen(decode_result) > 0);
  printf("%s\n", decode_result);
  assert(strcmp(decode_result,
                "https://test.domain/01/00068780000108/13/301231/10/ABC123") == 0);

  free_grid(&grid2);

  free(decode_result);
}

void test_strcat()
{
  printf("test_strcat\n");
  char initial_string[32];
  strcpy(initial_string, "First string");
  assert(strcmp(initial_string, "First string") == 0);
  decode_strcat(initial_string, " second string");
  assert(strcmp(initial_string, "First string second string") == 0);
  decode_strcat_char(initial_string, ' ');
  decode_strcat_char(initial_string, 'a');
  decode_strcat_char(initial_string, 'b');
  decode_strcat_char(initial_string, 'c');
  assert(strcmp(initial_string, "First string second string abc") == 0);
}

void test_condense()
{
  printf("test_condense\n");
  int dimension_x, dimension_y, grid_x, grid_y;
  int condensed_dimension_x, condensed_dimension_y;
  struct grid_2d grid;

  char * decode_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  assert(decode_result != NULL);

  unsigned char occupancy[] = {
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
    1,0,0,1,0,1,0,0,1,0,0,0,0,1,0,1,1,0,0,1,0,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,
    1,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0,0,0,1,1,0,0,1,0,1,1,1,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,0,0,
    1,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,1,1,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,1,1,1,1,1,0,1,1,1,
    1,1,0,0,0,1,1,0,1,0,1,1,0,1,1,1,0,0,1,1,0,0,1,1,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
    1,1,0,0,1,1,1,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,1,0,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,
    1,0,1,1,0,0,0,1,0,1,0,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0,0,1,1,1,1,1,0,
    1,0,1,0,1,0,1,1,0,1,1,0,1,1,1,0,0,0,1,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,
    1,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,0,
    1,0,1,0,1,1,0,0,1,1,0,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,1,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,1,1,
    1,1,1,1,1,1,0,1,1,1,0,0,0,1,1,1,0,1,0,0,0,0,1,1,0,0,1,0,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,0,
    1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,1,1,0,0,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,
    1,1,1,1,0,1,1,1,1,1,1,0,1,1,0,0,0,1,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,1,0,0,1,0,0,0,1,0,
    1,1,1,1,0,1,1,0,1,1,0,0,0,1,0,1,1,1,1,1,0,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,0,0,0,1,1,0,1,1,
    1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,
    1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,1,0,1,0,1,1,0,1,1,1,0,0,0,1,1,1,
    1,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,1,1,0,1,1,1,1,0,1,1,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,
    1,1,1,1,0,1,1,1,1,1,0,0,1,0,1,0,0,0,0,1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,0,1,0,1,1,0,1,1,1,
    1,0,0,1,1,1,1,1,0,0,0,1,1,0,1,0,1,1,0,1,0,0,1,1,0,0,1,1,1,0,1,1,0,0,1,1,1,1,1,0,1,0,0,0,
    1,1,0,0,0,1,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,
    1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,1,0,1,1,0,0,1,0,0,1,0,0,0,1,1,1,1,0,0,0,1,0,0,1,0,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,
    1,0,1,0,1,0,1,0,1,1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,1,1,1,0,1,0,1,1,0,1,
    1,0,0,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,1,0,1,1,0,0,
    1,0,0,1,1,1,0,1,1,0,1,0,1,0,0,1,0,0,0,1,1,1,1,0,0,1,1,1,0,0,0,1,1,1,0,0,1,1,0,0,0,1,0,1,
    1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,
    1,0,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,0,1,1,1,0,0,1,0,1,0,1,0,1,1,1,1,1,1,0,1,0,0,0,1,
    1,1,1,0,0,0,1,1,1,0,0,1,1,0,1,0,1,0,1,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,
    1,1,0,0,0,1,0,0,0,1,0,0,1,1,0,1,1,0,1,1,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,1,0,1,1,0,1,
    1,1,1,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0,0,1,0,
    1,1,1,0,1,1,1,1,1,0,0,0,1,0,0,1,0,0,1,1,1,1,1,0,0,0,1,0,1,0,1,1,1,1,0,1,1,0,0,1,1,0,0,1,
    1,1,1,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,
    1,0,0,0,0,1,1,1,1,0,0,1,0,1,1,0,0,1,0,1,1,1,1,0,0,0,1,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
    1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,0,1,1,0,0,0,1,0,0,0,1,0,
    1,0,1,0,0,1,1,1,0,0,0,1,1,1,0,1,1,0,1,1,1,1,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,
    1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,0,1,0,0,0,0,0,1,1,1,1,0,0,1,0,0,1,1,0,1,0,0,0,
    1,1,0,1,0,0,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,1,0,1,0,1,1,0,0,1,1,1,1,0,1,1,1,
    1,1,1,0,1,1,1,1,0,1,1,0,0,0,1,1,1,0,1,1,1,0,1,0,0,1,1,1,0,0,0,1,0,0,1,1,1,0,0,0,1,0,1,0,
    1,1,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,0,1,1,
    1,0,1,0,1,0,1,0,1,1,0,1,0,0,1,0,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,0,0,
    1,1,1,0,0,0,1,0,1,0,1,1,1,1,0,1,0,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,0,1,1,0,0,0,1,0,1,0,1,1,
    1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,1,0,0,0,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  };

  unsigned char condensed[] = {
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
    1,0,0,1,0,1,0,0,1,0,0,0,0,1,0,1,1,0,0,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,
    1,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0,0,0,1,1,0,0,1,1,1,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,0,0,
    1,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,1,0,0,0,0,1,1,1,1,1,1,0,1,1,1,
    1,1,0,0,0,1,1,0,1,0,1,1,0,1,1,1,0,0,1,1,0,1,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
    1,1,0,0,1,1,1,0,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,0,1,0,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,
    1,0,1,1,0,0,0,1,0,1,0,0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,1,1,0,1,1,0,0,1,1,1,1,1,0,
    1,0,1,0,1,0,1,1,0,1,1,0,1,1,1,0,0,0,1,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,
    1,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,0,
    1,0,1,0,1,1,0,0,1,1,0,1,0,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,1,1,
    1,1,1,1,1,1,0,1,1,1,0,0,0,1,1,1,0,1,0,0,0,1,0,0,1,0,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,0,
    1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,
    1,1,1,1,0,1,1,1,1,1,1,0,1,1,0,0,0,1,1,1,0,0,0,1,0,1,1,0,1,0,0,1,1,1,0,0,1,0,0,0,1,0,
    1,1,1,1,0,1,1,0,1,1,0,0,0,1,0,1,1,1,1,1,0,0,1,0,1,1,0,1,0,1,1,0,1,0,0,0,0,1,1,0,1,1,
    1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,1,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,
    1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,1,0,1,0,1,1,0,1,1,1,0,0,0,1,1,1,
    1,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,1,1,1,1,1,0,1,1,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,
    1,1,1,1,0,1,1,1,1,1,0,0,1,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1,0,1,1,1,1,0,1,0,1,1,0,1,1,1,
    1,0,0,1,1,1,1,1,0,0,0,1,1,0,1,0,1,1,0,1,0,1,0,0,1,1,1,0,1,1,0,0,1,1,1,1,1,0,1,0,0,0,
    1,1,0,0,0,1,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,
    1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,1,0,1,1,0,0,0,1,0,0,0,1,1,1,1,0,0,0,1,0,0,1,0,1,1,0,
    1,0,1,0,1,0,1,0,1,1,0,1,0,1,1,0,0,1,1,0,1,1,0,0,0,0,0,1,1,0,0,0,1,1,1,0,1,0,1,1,0,1,
    1,0,0,1,1,0,1,1,1,1,0,1,0,1,1,1,1,0,1,1,0,0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,1,0,1,1,0,0,
    1,0,0,1,1,1,0,1,1,0,1,0,1,0,0,1,0,0,0,1,1,0,0,1,1,1,0,0,0,1,1,1,0,0,1,1,0,0,0,1,0,1,
    1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,
    1,0,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,0,1,0,0,1,0,1,0,1,0,1,1,1,1,1,1,0,1,0,0,0,1,
    1,1,1,0,0,0,1,1,1,0,0,1,1,0,1,0,1,0,1,1,0,0,1,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,
    1,1,0,0,0,1,0,0,0,1,0,0,1,1,0,1,1,0,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,1,0,1,1,0,1,
    1,1,1,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,0,0,1,0,
    1,1,1,0,1,1,1,1,1,0,0,0,1,0,0,1,0,0,1,1,1,0,0,0,1,0,1,0,1,1,1,1,0,1,1,0,0,1,1,0,0,1,
    1,1,1,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,
    1,0,0,0,0,1,1,1,1,0,0,1,0,1,1,0,0,1,0,1,1,0,0,0,1,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
    1,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,1,1,0,0,0,1,0,0,0,1,0,
    1,0,1,0,0,1,1,1,0,0,0,1,1,1,0,1,1,0,1,1,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,
    1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,0,0,1,1,0,1,0,0,0,
    1,1,0,1,0,0,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,1,0,1,0,1,1,0,0,1,1,1,1,0,1,1,1,
    1,1,1,0,1,1,1,1,0,1,1,0,0,0,1,1,1,0,1,1,1,0,0,1,1,1,0,0,0,1,0,0,1,1,1,0,0,0,1,0,1,0,
    1,1,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,1,1,0,1,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,0,1,1,
    1,0,1,0,1,0,1,0,1,1,0,1,0,0,1,0,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,0,0,
    1,1,1,0,0,0,1,0,1,0,1,1,1,1,0,1,0,1,1,0,0,1,1,0,0,0,0,1,1,1,0,1,1,0,0,0,1,0,1,0,1,1,
    1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,1,0,0,0,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  };

  dimension_x = 44;
  dimension_y = 44;
  create_grid_from_pattern(dimension_x, dimension_y, &grid, occupancy);
  condense_data_blocks(&grid, 1);

  condensed_dimension_x = 42;
  condensed_dimension_y = 42;

  assert(grid.dimension_x == condensed_dimension_x);
  assert(grid.dimension_y == condensed_dimension_y);

  for (grid_y = 0; grid_y < condensed_dimension_y; grid_y++) {
    for (grid_x = 0; grid_x < condensed_dimension_x; grid_x++) {
      if (grid.occupancy[grid_x][grid_y] != condensed[grid_y*condensed_dimension_y + grid_x]) {
        printf("Grid coord: %d,%d\n", grid_x, grid_y);
      }
      assert(grid.occupancy[grid_x][grid_y] == condensed[grid_y*condensed_dimension_y + grid_x]);
    }
  }

  grid.dimension_x = dimension_x;
  grid.dimension_y = dimension_y;

  free_grid(&grid);
}

void test_iso15434_translate()
{
  printf("test_iso15434_translate\n");
  char format_code[3];
  char test_data[MAX_DECODE_LENGTH];
  char iso15434_uii[MAX_DECODE_LENGTH];
  char * result;
  unsigned char debug = 1;

  format_code[0] = 0;
  test_data[0] = 0;
  iso15434_uii[0] = 0;

  decode_strcat(&format_code[0], "06");
  decode_strcat(&test_data[0], "9S12345");

  result = iso15434_translate_data_qualifier(&test_data[0], 0,
                                             strlen(&test_data[0]),
                                             &iso15434_uii[0],
                                             &format_code[0],
                                             debug);
  if (result != NULL) {
    assert(strcmp(result, "PACKAGE ID: 12345") == 0);
    free(result);
    assert(strcmp(&iso15434_uii[0], "12345") == 0);
  }

  format_code[0] = 0;
  test_data[0] = 0;
  iso15434_uii[0] = 0;

  decode_strcat(&format_code[0], "12");
  decode_strcat(&test_data[0], "PNO 987654");

  result = iso15434_translate_data_qualifier(&test_data[0], 0,
                                             strlen(&test_data[0]),
                                             &iso15434_uii[0],
                                             &format_code[0],
                                             debug);
  if (result != NULL) {
    assert(strcmp(result, "PART NUMBER: 987654") == 0);
    free(result);
    assert(strcmp(&iso15434_uii[0], "987654") == 0);
  }

  format_code[0] = 0;
  test_data[0] = 0;
  iso15434_uii[0] = 0;

  decode_strcat(&format_code[0], "05");
  decode_strcat(&test_data[0], "8002268435460012427936");

  result = iso15434_translate_data_qualifier(&test_data[0], 0,
                                             strlen(&test_data[0]),
                                             &iso15434_uii[0],
                                             &format_code[0],
                                             debug);
  assert(result != NULL);
  assert(strcmp(result, "CMT NO: 268435460012427936") == 0);
  free(result);
  assert(strcmp(&iso15434_uii[0], "268435460012427936") == 0);
}

void test_hibc_translate()
{
  printf("test_hibc_translate\n");
  char test_data[MAX_DECODE_LENGTH];
  char hibc_result[MAX_DECODE_LENGTH];

  test_data[0] = 0;
  hibc_result[0] = 0;
  decode_strcat(&test_data[0], "+A99912345/$$52001510X3/16D20111212/S77DEFG457");
  hibc_semantics(&test_data[0], &hibc_result[0], 1);
  printf("%s\n", &hibc_result[0]);
  assert(strcmp(&hibc_result[0],
                "LABELER ID: A999\nPRODUCT ID: 1234\nUNIT OF MEASURE: 5\nEXPIRY: 2020 DAY 015\nLOT NUMBER: 10X3\nMANUFACTURE DATE YYYYMMDD: 12 Dec 2011\nSUPPLIER SERIAL NUMBER: 77DEFG457\n") == 0);

  test_data[0] = 0;
  hibc_result[0] = 0;
  decode_strcat(&test_data[0], "+A99912345/$10X3/16D20111231/14D20200131");
  hibc_semantics(&test_data[0], &hibc_result[0], 1);
  printf("%s\n", &hibc_result[0]);
  assert(strcmp(&hibc_result[0],
                "LABELER ID: A999\nPRODUCT ID: 1234\nUNIT OF MEASURE: 5\nLOT NUMBER: 10X3\nMANUFACTURE DATE YYYYMMDD: 31 Dec 2011\nEXPIRATION DATE YYYYMMDD: 31 Jan 2020\n") == 0);
}

void test_gs1_currency()
{
  printf("test_gs1_currency\n");
  int application_identifier = 3910;
  char data_str1[] = "978123";
  char * result1 = get_currency_value(application_identifier, data_str1);
  assert(result1 != NULL);
  printf("%s\n", result1);
  assert(strcmp(result1, "123.00 Euro") == 0);
  free(result1);

  application_identifier = 3911;
  char data_str2[] = "7101231";
  char * result2 = get_currency_value(application_identifier, data_str2);
  assert(result2 != NULL);
  printf("%s\n", result2);
  assert(strcmp(result2, "123.10 Rand") == 0);
  free(result2);

  application_identifier = 3912;
  char data_str3[] = "71012325";
  char * result3 = get_currency_value(application_identifier, data_str3);
  assert(result3 != NULL);
  printf("%s\n", result3);
  assert(strcmp(result3, "123.25 Rand") == 0);
  free(result3);
}

void test_gs1_decimal()
{
  printf("test_gs1_decimal\n");
  int application_identifier = 3920;
  char data_str1[] = "12345";
  char * result1 = get_decimal_value(application_identifier, data_str1);
  assert(result1 != NULL);
  printf("%s\n", result1);
  assert(strcmp(result1, "12345.00") == 0);
  free(result1);

  application_identifier = 3921;
  char data_str2[] = "1234567";
  char * result2 = get_decimal_value(application_identifier, data_str2);
  assert(result2 != NULL);
  printf("%s\n", result2);
  assert(strcmp(result2, "123456.70") == 0);
  free(result2);

  application_identifier = 3922;
  char data_str3[] = "1234567";
  char * result3 = get_decimal_value(application_identifier, data_str3);
  assert(result3 != NULL);
  printf("%s\n", result3);
  assert(strcmp(result3, "12345.67") == 0);
  free(result3);
}

void test_gs1_country()
{
  printf("test_gs1_country\n");
  char data_str1[] = "764";
  char * result1 = get_country(data_str1);
  assert(result1 != NULL);
  printf("%s\n", result1);
  assert(strcmp(result1, "Thailand") == 0);
  free(result1);

  char data_str2[] = "826ABCD";
  char * result2 = get_country(data_str2);
  assert(result2 != NULL);
  printf("%s\n", result2);
  assert(strcmp(result2, "United Kingdom ABCD") == 0);
  free(result2);
}

void test_date_conversion()
{
  printf("test_date_conversion\n");
  char * id_human_readable1 = "YYYYDDMM";
  char * id_value1 = "20250407";
  char * result1 = data_id_convert_date(id_human_readable1, id_value1);
  assert(result1 != NULL);
  printf("%s\n", result1);
}

void test_gs1_coupon()
{
  printf("test_gs1_coupon\n");
  char data_str1[] = "123456ABCDEF8765432";
  char * result1 = get_coupon(data_str1);
  assert(result1 != NULL);
  printf("%s\n", result1);
  assert(strcmp(result1, "COMPANY/COUPON REF: 123456ABCDEF\nSERIAL: 8765432") == 0);
  free(result1);
}

void test_gs1_issn()
{
  printf("test_gs1_issn\n");
  char data_str1[] = "1234567059";
  char * result1 = get_issn(data_str1);
  assert(result1 != NULL);
  printf("%s\n", result1);
  assert(strcmp(result1, "ISSN: 1234-5679\nVARIANT: 05\nCHECK DIGIT: 9") == 0);
  free(result1);
}

void run_all_tests()
{
  test_strcat();
  test_decode();
  test_gs1_decode();
  test_condense();
  test_rotate();
  test_iso15434_translate();
  test_hibc_translate();
  test_gs1_currency();
  test_gs1_decimal();
  test_gs1_country();
  test_date_conversion();
  test_gs1_coupon();
  test_gs1_issn();
  printf("All tests complete\n");
}
