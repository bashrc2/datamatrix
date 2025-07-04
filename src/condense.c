/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Condense multi-block datamatrix codes in preparation for decoding
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

/**
 * \brief some larger datamatrix may consist of multiple blocks/regions
 *        with internal timing borders. This returns the number
 *        of readable blocks in both dimensions
 * \param grid grid object
 * \param blocks_x number of readable blocks in the x dimension
 * \param blocks_y number of readable blocks in the y dimension
 */
static void grid_data_blocks(struct grid_2d * grid,
                             int * blocks_x,
                             int * blocks_y)
{
  /* most grids will only contain a single block */
  *blocks_x=1;
  *blocks_y=1;

  if (grid->dimension_x == grid->dimension_y) {
    /* square */
    if ((grid->dimension_x >= 32) &&
        (grid->dimension_x <= 52)) {
      *blocks_x = 2;
      *blocks_y = 2;
    }
    else if ((grid->dimension_x >= 64) &&
             (grid->dimension_x <= 104)) {
      *blocks_x = 4;
      *blocks_y = 4;
    }
    else if (grid->dimension_x >= 120) {
      *blocks_x = 6;
      *blocks_y = 6;
    }

    return;
  }

  /* rectangular */
  if ((grid->dimension_x == 32) &&
      (grid->dimension_y == 8)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 8) &&
           (grid->dimension_y == 32)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 12) &&
           (grid->dimension_y == 36)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 16) &&
           (grid->dimension_y == 36)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 36) &&
           (grid->dimension_y == 12)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 36) &&
           (grid->dimension_y == 16)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 48) &&
           (grid->dimension_y == 16)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 16) &&
           (grid->dimension_y == 48)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 8) &&
           (grid->dimension_y == 48)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 48) &&
           (grid->dimension_y == 8)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 8) &&
           (grid->dimension_y == 64)) {
    *blocks_x = 1;
    *blocks_y = 4;
  }
  else if ((grid->dimension_x == 64) &&
           (grid->dimension_y == 8)) {
    *blocks_x = 4;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 12) &&
           (grid->dimension_y == 48)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 48) &&
           (grid->dimension_y == 12)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 12) &&
           (grid->dimension_y == 64)) {
    *blocks_x = 1;
    *blocks_y = 4;
  }
  else if ((grid->dimension_x == 64) &&
           (grid->dimension_y == 12)) {
    *blocks_x = 4;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 16) &&
           (grid->dimension_y == 64)) {
    *blocks_x = 1;
    *blocks_y = 4;
  }
  else if ((grid->dimension_x == 64) &&
           (grid->dimension_y == 16)) {
    *blocks_x = 4;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 24) &&
           (grid->dimension_y == 32)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 32) &&
           (grid->dimension_y == 24)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 24) &&
           (grid->dimension_y == 36)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 36) &&
           (grid->dimension_y == 24)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 24) &&
           (grid->dimension_y == 48)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 48) &&
           (grid->dimension_y == 24)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 24) &&
           (grid->dimension_y == 64)) {
    *blocks_x = 1;
    *blocks_y = 4;
  }
  else if ((grid->dimension_x == 64) &&
           (grid->dimension_y == 24)) {
    *blocks_x = 4;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 26) &&
           (grid->dimension_y == 32)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 32) &&
           (grid->dimension_y == 26)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 26) &&
           (grid->dimension_y == 40)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 40) &&
           (grid->dimension_y == 26)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 26) &&
           (grid->dimension_y == 48)) {
    *blocks_x = 1;
    *blocks_y = 2;
  }
  else if ((grid->dimension_x == 48) &&
           (grid->dimension_y == 26)) {
    *blocks_x = 2;
    *blocks_y = 1;
  }
  else if ((grid->dimension_x == 26) &&
           (grid->dimension_y == 64)) {
    *blocks_x = 1;
    *blocks_y = 4;
  }
  else if ((grid->dimension_x == 64) &&
           (grid->dimension_y == 26)) {
    *blocks_x = 4;
    *blocks_y = 1;
  }
}

/**
 * \brief larger datamatrix contain multiple blocks of data.
 *        This condenses them into a single block
 * \param grid grid object
 */
unsigned char condense_data_blocks(struct grid_2d * grid,
                                   unsigned char debug)
{
  int blocks_x=1, blocks_y=1, grid_x, grid_y, block_x, block_y;
  int original_tx, original_ty, new_tx, new_ty, new_bx, new_by;
  int new_x, new_y, original_x, original_y, i;

  grid_data_blocks(grid, &blocks_x, &blocks_y);

  if ((blocks_x == 1) && (blocks_y == 1)) return 0;

  /* multiple data blocks become one big block */
  int original_cells_per_block_x = grid->dimension_x / blocks_x;
  int original_cells_per_block_y = grid->dimension_y / blocks_y;

  int new_dimension_x = grid->dimension_x - ((blocks_x - 1) * 2);
  int new_dimension_y = grid->dimension_y - ((blocks_y - 1) * 2);

  /* the area which contains data without the fixed pattern */
  int data_width_x = original_cells_per_block_x - 2;
  int data_width_y = original_cells_per_block_y - 2;

  if (debug == 1) {
    printf("Condense dimensions %dx%d -> %dx%d\n",
           grid->dimension_x, grid->dimension_y,
           new_dimension_x, new_dimension_y);
  }

  unsigned char ** new_occupancy = grid->occupancy_buffer;
  unsigned char * new_damage = grid->damage_buffer;
  memset(new_damage, 0, new_dimension_x * new_dimension_y * sizeof(unsigned char));

  /* create a new occupancy and damage pattern */
  for (block_x = 0; block_x < blocks_x; block_x++) {
    original_tx = 1 + (block_x * original_cells_per_block_x);
    new_tx = 1 + (block_x * data_width_x);
    new_bx = new_tx + data_width_x;
    for (block_y = 0; block_y < blocks_y; block_y++) {
      original_ty = 1 + (block_y * original_cells_per_block_y);
      new_ty = 1 + (block_y * data_width_y);
      new_by = new_ty + data_width_y;

      /* copy the block at (block_x, block_y) */
      for (new_x = new_tx; new_x < new_bx; new_x++) {
        original_x = new_x - new_tx + original_tx;
        for (new_y = new_ty; new_y < new_by; new_y++) {
          original_y = new_y - new_ty + original_ty;
          new_occupancy[new_x][new_y] = grid->occupancy[original_x][original_y];
          new_damage[new_y*new_dimension_x + new_x] =
            grid->damage[original_y*grid->dimension_x + original_x];
        }
      }
    }
  }

  /* solid border */
  for (grid_y = 0; grid_y < new_dimension_y; grid_y++) {
    new_occupancy[0][grid_y] = 1;
  }
  for (grid_x = 0; grid_x < new_dimension_x; grid_x++) {
    new_occupancy[grid_x][new_dimension_y-1] = 1;
  }

  /* timing border */
  for (grid_y = 0; grid_y < new_dimension_y; grid_y++) {
    new_occupancy[new_dimension_x-1][grid_y] = grid_y % 2;
  }
  for (grid_x = 0; grid_x < new_dimension_x; grid_x++) {
    new_occupancy[grid_x][0] = 1 - (grid_x % 2);
  }

  /* copy back to the original occupancy and damage arrays */
  i = 0;
  for (grid_y = 0; grid_y < new_dimension_y; grid_y++) {
    for (grid_x = 0; grid_x < new_dimension_x; grid_x++, i++) {
      grid->occupancy[grid_x][grid_y] = new_occupancy[grid_x][grid_y];
      grid->damage[i] = new_damage[i];
    }
  }
  grid->dimension_x = new_dimension_x;
  grid->dimension_y = new_dimension_y;

  if (debug == 1) {
    printf("\nCondensed:\n");
    show_grid(grid);
  }

  return 1;
}
