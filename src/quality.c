/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix quality metrics
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
 * \brief returns the grid cell width
 * \param grid grid object
 * \return cell width
 */
static float get_cell_width(struct grid_2d * grid)
{
  float longest_side =
    get_longest_side(grid->perimeter.x0, grid->perimeter.y0,
                     grid->perimeter.x1, grid->perimeter.y1,
                     grid->perimeter.x2, grid->perimeter.y2,
                     grid->perimeter.x3, grid->perimeter.y3);
  if (grid->dimension_x > grid->dimension_y) {
    return longest_side / grid->dimension_x;
  }
  return longest_side / grid->dimension_y;
}

/**
 * \brief returns 1 if the given point is in the grid quiet zone perimeter
 * \param grid grid object
 * \param x x coordinate to test
 * \param y y coordinate to test
 * \return 1 if the point is in the quiet zone
 */
static unsigned char point_in_quiet_zone(struct grid_2d * grid,
                                         int x, int y)
{
  int points[4*2] = {
    (int)grid->quiet_zone_perimeter.x0, (int)grid->quiet_zone_perimeter.y0,
    (int)grid->quiet_zone_perimeter.x1, (int)grid->quiet_zone_perimeter.y1,
    (int)grid->quiet_zone_perimeter.x2, (int)grid->quiet_zone_perimeter.y2,
    (int)grid->quiet_zone_perimeter.x3, (int)grid->quiet_zone_perimeter.y3
  };
  if (point_in_polygon(x, y, &points[0], 4) != 0) return 1;
  return 0;
}

/**
 * \brief returns 1 if the given point is within the grid perimeter
 * \param grid grid object
 * \param x x coordinate to test
 * \param y y coordinate to test
 * \return 1 if the given point is inside the perimeter
 */
static unsigned char point_in_perimeter(struct grid_2d * grid,
                                        int x, int y)
{
  int points[4*2] = {
    (int)grid->perimeter.x0, (int)grid->perimeter.y0,
    (int)grid->perimeter.x1, (int)grid->perimeter.y1,
    (int)grid->perimeter.x2, (int)grid->perimeter.y2,
    (int)grid->perimeter.x3, (int)grid->perimeter.y3
  };
  if (point_in_polygon(x, y, &points[0], 4) != 0) return 1;
  return 0;
}

/**
 * \brief calculates the perimeter of the quiet zone around the datamatrix symbol
 * \param grid grid object
 */
static void calculate_quiet_zone(struct grid_2d * grid)
{
  int i;
  float centre_x=0, centre_y=0,dx,dy, fraction, cell_width;
  float points[4*2] = {
    grid->perimeter.x0, grid->perimeter.y0,
    grid->perimeter.x1, grid->perimeter.y1,
    grid->perimeter.x2, grid->perimeter.y2,
    grid->perimeter.x3, grid->perimeter.y3
  };

  get_centroid(grid->perimeter.x0, grid->perimeter.y0,
               grid->perimeter.x1, grid->perimeter.y1,
               grid->perimeter.x2, grid->perimeter.y2,
               grid->perimeter.x3, grid->perimeter.y3,
               &centre_x, &centre_y);

  /* how much to we need to expand, equivalent to one cell width */
  cell_width = get_cell_width(grid);
  float longest_side =
    get_longest_side(grid->perimeter.x0, grid->perimeter.y0,
                     grid->perimeter.x1, grid->perimeter.y1,
                     grid->perimeter.x2, grid->perimeter.y2,
                     grid->perimeter.x3, grid->perimeter.y3);
  fraction = (longest_side + cell_width) / longest_side;

  /* calculate the expanded points */
  for (i = 0; i < 4; i++) {
    dx = points[i*2] - centre_x;
    dy = points[i*2+1] - centre_y;
    points[i*2] = centre_x + (dx*fraction);
    points[i*2+1] = centre_y + (dy*fraction);
  }
  grid->quiet_zone_perimeter.x0 = points[0];
  grid->quiet_zone_perimeter.y0 = points[1];
  grid->quiet_zone_perimeter.x1 = points[2];
  grid->quiet_zone_perimeter.y1 = points[3];
  grid->quiet_zone_perimeter.x2 = points[4];
  grid->quiet_zone_perimeter.y2 = points[5];
  grid->quiet_zone_perimeter.x3 = points[6];
  grid->quiet_zone_perimeter.y3 = points[7];
}

/**
 * \brief get the offset of a grid cell from its theoretical centre
 * \param thresholded_image_data thresholded (binary) image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param x theoretical/ideal x coordinate of the centre of the grid cell
 * \param y theoretical/ideal y coordinate of the centre of the grid cell
 * \param radius radius of the grid cell
 * \param offset_x returned x offset of the cell centre
 * \param offset_y returned y offset of the cell centre
 * \param elongation returned elongation of the cell
 * \param no_of_pixels returned number of pixels within the cell
 */
static void grid_nonuniformity_test_cell(unsigned char thresholded_image_data[],
                                         int image_width, int image_height,
                                         int image_bytesperpixel,
                                         int x, int y, int radius,
                                         int * offset_x, int * offset_y,
                                         float * elongation,
                                         int * no_of_pixels,
                                         int * cell_fill)
{
  int search_x, search_y, av_x=0, av_y=0, hits=0;
  int cell_width, cell_height, cell_pixels=0;
  int min_x=image_width, min_y=image_height, max_x=0, max_y=0;
  int n = (y*image_width + x)*image_bytesperpixel;

  *offset_x = NO_OFFSET;
  *elongation = 0;
  *cell_fill = 0;
  *no_of_pixels = 0;
  if (thresholded_image_data[n] == 0) return;

  for (search_y = y - radius; search_y <= y + radius; search_y++) {
    if ((search_y < 0) || (search_y >= image_height)) continue;
    for (search_x = x - radius; search_x <= x + radius; search_x++) {
      if ((search_x < 0) || (search_x >= image_width)) continue;
      cell_pixels++;
      n = (search_y*image_width + search_x)*image_bytesperpixel;
      if (thresholded_image_data[n] != 0) {
        if (search_x < min_x) min_x = search_x;
        if (search_x > max_x) max_x = search_x;
        if (search_y < min_y) min_y = search_y;
        if (search_y > max_y) max_y = search_y;
        av_x += search_x;
        av_y += search_y;
        hits++;
      }
    }
  }

  /* calculate cell fill */
  if (cell_pixels > 0) {
    *cell_fill = hits * 100 / cell_pixels;
  }

  if (hits > 0) {
    /* calculate elongation */
    cell_width = max_x - min_x;
    cell_height = max_y - min_y;
    if (cell_height > 0) {
      *elongation = fabs(1.0f - (cell_width / (float)cell_height));
    }

    /* calculate offset from centre */
    av_x /= hits;
    av_y /= hits;
    *offset_x = x - av_x;
    *offset_y = y - av_y;
    *no_of_pixels = hits;
  }
}

/**
 * \brief calculate grid non-uniformity, elongation, dots per element and cell fill
 * \param grid grid object
 * \param thresholded_image_data binary image
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bitsperpixel Number of bits per pixel
 */
static void quality_metric_grid_nonuniformity(struct grid_2d * grid,
                                              unsigned char thresholded_image_data[],
                                              int image_width, int image_height,
                                              int image_bitsperpixel)
{
  int image_bytesperpixel = image_bitsperpixel/8;
  int grid_x, grid_y, offset_x=0, offset_y=0;
  int av_offset_x=0, av_offset_y=0, offset_hits=0;
  int cell_no_of_pixels, av_cell_no_of_pixels=0;
  int cell_fill=0, total_cell_fill=0;
  float grid_non_uniformity_x, grid_non_uniformity_y;
  float cell_elongation, av_elongation=0;
  float xi, yi, grid_pos_x, grid_pos_y;
  float horizontal_x1, horizontal_y1, horizontal_x2, horizontal_y2;
  float vertical_x1, vertical_y1, vertical_x2, vertical_y2;
  float horizontal_dx1 = grid->perimeter.x3 - grid->perimeter.x0;
  float horizontal_dy1 = grid->perimeter.y3 - grid->perimeter.y0;
  float horizontal_dx2 = grid->perimeter.x2 - grid->perimeter.x1;
  float horizontal_dy2 = grid->perimeter.y2 - grid->perimeter.y1;

  float vertical_dx1 = grid->perimeter.x1 - grid->perimeter.x0;
  float vertical_dy1 = grid->perimeter.y1 - grid->perimeter.y0;
  float vertical_dx2 = grid->perimeter.x2 - grid->perimeter.x3;
  float vertical_dy2 = grid->perimeter.y2 - grid->perimeter.y3;

  float cell_width = get_cell_width(grid);
  int cell_radius = (int)(cell_width/2);

  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    /* horizontal line */
    grid_pos_y = grid_y + 0.5f;
    horizontal_x1 = grid->perimeter.x0 + (horizontal_dx1 * grid_pos_y / grid->dimension_y);
    horizontal_y1 = grid->perimeter.y0 + (horizontal_dy1 * grid_pos_y / grid->dimension_y);
    horizontal_x2 = grid->perimeter.x1 + (horizontal_dx2 * grid_pos_y / grid->dimension_y);
    horizontal_y2 = grid->perimeter.y1 + (horizontal_dy2 * grid_pos_y / grid->dimension_y);

    for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
      /* vertical line */
      grid_pos_x = grid_x + 0.5f;
      vertical_x1 = grid->perimeter.x0 + (vertical_dx1 * grid_pos_x / grid->dimension_x);
      vertical_y1 = grid->perimeter.y0 + (vertical_dy1 * grid_pos_x / grid->dimension_x);
      vertical_x2 = grid->perimeter.x3 + (vertical_dx2 * grid_pos_x / grid->dimension_x);
      vertical_y2 = grid->perimeter.y3 + (vertical_dy2 * grid_pos_x / grid->dimension_x);
      intersection(horizontal_x1, horizontal_y1,
                   horizontal_x2, horizontal_y2,
                   vertical_x1, vertical_y1,
                   vertical_x2, vertical_y2,
                   &xi, &yi);
      if (xi != PARALLEL_LINES) {
        grid_nonuniformity_test_cell(thresholded_image_data,
                                     image_width, image_height,
                                     image_bytesperpixel,
                                     (int)xi, (int)yi, cell_radius,
                                     &offset_x, &offset_y,
                                     &cell_elongation,
                                     &cell_no_of_pixels,
                                     &cell_fill);
        if (offset_x != NO_OFFSET) {
          av_offset_x += abs(offset_x);
          av_offset_y += abs(offset_y);
          av_elongation += cell_elongation;
          av_cell_no_of_pixels += cell_no_of_pixels;
          total_cell_fill += cell_fill;
          offset_hits++;
        }
      }
    }
  }

  grid->grid_non_uniformity = 0;
  grid->grid_non_uniformity_grade = 0;
  grid->elongation = 0;
  grid->dots_per_element = 0;
  grid->cell_fill = 0;
  if (offset_hits > 0) {
    grid->cell_fill = (unsigned char)(total_cell_fill / offset_hits);
    grid->elongation = (av_elongation / offset_hits) * 100;
    grid->dots_per_element = av_cell_no_of_pixels / offset_hits;

    grid_non_uniformity_x = fabs(av_offset_x / (float)offset_hits);
    grid_non_uniformity_y = fabs(av_offset_y / (float)offset_hits);
    grid->grid_non_uniformity = (grid_non_uniformity_x + grid_non_uniformity_y) * 100 / cell_width;
    /* calculate grade as per GS1 2D Barcode Verification Process Implementation Guideline
       table 9-5 */
    if (grid->grid_non_uniformity <= 75) {
      grid->grid_non_uniformity_grade = 1;
    }
    if (grid->grid_non_uniformity <= 63) {
      grid->grid_non_uniformity_grade = 2;
    }
    if (grid->grid_non_uniformity <= 50) {
      grid->grid_non_uniformity_grade = 3;
    }
    if (grid->grid_non_uniformity <= 38) {
      grid->grid_non_uniformity_grade = 4;
    }
  }
}

/**
 * \brief calculate distributed damage as a percentage
 * \param grid grid object
 */
static void quality_metric_distributed_damage(struct grid_2d * grid)
{
  int n, x, y, damage=0, cells=0;

  for (y = 1; y < grid->dimension_y-1; y++) {
    n = y*grid->dimension_x;
    for (x = 1; x < grid->dimension_x-1; x++, cells++) {
      if (grid->damage[n+x] > 0) {
        damage++;
      }
    }
  }
  grid->distributed_damage = (unsigned char)(damage * 100 / cells);
}

/**
 * \brief calculate axial non-uniformity as the percent difference between
 *        cell width and height
 * \param grid grid object
 */
static void quality_metric_axial_nonuniformity(struct grid_2d * grid)
{
  float cell_width_longest, cell_width_shortest;
  float longest_side =
    get_longest_side(grid->perimeter.x0, grid->perimeter.y0,
                     grid->perimeter.x1, grid->perimeter.y1,
                     grid->perimeter.x2, grid->perimeter.y2,
                     grid->perimeter.x3, grid->perimeter.y3);
  float shortest_side =
    get_shortest_side(grid->perimeter.x0, grid->perimeter.y0,
                      grid->perimeter.x1, grid->perimeter.y1,
                      grid->perimeter.x2, grid->perimeter.y2,
                      grid->perimeter.x3, grid->perimeter.y3);
  if (grid->dimension_x > grid->dimension_y) {
    cell_width_longest = longest_side / grid->dimension_x;
    cell_width_shortest = shortest_side / grid->dimension_y;
  }
  else {
    cell_width_longest = longest_side / grid->dimension_y;
    cell_width_shortest = shortest_side / grid->dimension_x;
  }
  grid->axial_non_uniformity = fabs(1.0f - (cell_width_shortest/cell_width_longest))*100;
  /* calculate grade as per GS1 2D Barcode Verification Process Implementation Guideline
     table 9-4 */
  grid->axial_non_uniformity_grade = 0;
  if (grid->axial_non_uniformity <= 12) {
    grid->axial_non_uniformity_grade = 1;
  }
  if (grid->axial_non_uniformity <= 10) {
    grid->axial_non_uniformity_grade = 2;
  }
  if (grid->axial_non_uniformity <= 8) {
    grid->axial_non_uniformity_grade = 3;
  }
  if (grid->axial_non_uniformity <= 6) {
    grid->axial_non_uniformity_grade = 4;
  }
}

/**
 * \brief returns the bounding box for the grid, including the quiet zone
 * \param grid grid object
 * \param image_width width of the image
 * \param image_height height of the image
 * \param min_x returned bounding box top left x coordinate
 * \param min_y returned bounding box top left y coordinate
 * \param max_x returned bounding box bottom right x coordinate
 * \param max_y returned bounding box bottom right y coordinate
 */
static void get_grid_bounding_box(struct grid_2d * grid,
                                  int image_width, int image_height,
                                  int * min_x, int * min_y,
                                  int * max_x, int * max_y)
{
  int i;
  int points[4*2] = {
    (int)grid->quiet_zone_perimeter.x0, (int)grid->quiet_zone_perimeter.y0,
    (int)grid->quiet_zone_perimeter.x1, (int)grid->quiet_zone_perimeter.y1,
    (int)grid->quiet_zone_perimeter.x2, (int)grid->quiet_zone_perimeter.y2,
    (int)grid->quiet_zone_perimeter.x3, (int)grid->quiet_zone_perimeter.y3
  };

  *min_x=image_width;
  *min_y=image_height;
  *max_x=0;
  *max_y=0;

  /* get the bounding box for the perimeter */
  for (i = 0; i < 4; i++) {
    if (points[i*2] < *min_x) *min_x = points[i*2];
    if (points[i*2+1] < *min_y) *min_y = points[i*2+1];
    if (points[i*2] > *max_x) *max_x = points[i*2];
    if (points[i*2+1] > *max_y) *max_y = points[i*2+1];
  }

  /* ensure that the bounding box is inside of the image */
  if (*min_x < 0) *min_x = 0;
  if (*min_y < 0) *min_y = 0;
  if (*max_x >= image_width) *max_x = image_width-1;
  if (*max_y >= image_height) *max_y = image_height-1;
}

/**
 * \brief Calculate grid cell modulation
 *        Also calculates quiet zone occupancy and contrast uniformity
 * \param grid grid object
 * \param image_data image array
 * \param thresholded_image_data binary image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param mage_bitsperpixel Number of bits per pixel
 */
static void quality_metric_modulation(struct grid_2d * grid,
                                      unsigned char image_data[],
                                      unsigned char thresholded_image_data[],
                                      int image_width, int image_height,
                                      int image_bitsperpixel)
{
  int image_bytesperpixel = image_bitsperpixel/8;
  /* symbol contrast converted back to a pixel value */
  float symbol_contrast = grid->symbol_contrast * 255 * image_bytesperpixel / 100.0f;
  int hits, reflectance, global_threshold;
  float modulation, cell_modulation, min_modulation=1;
  int min_x=image_width,min_y=image_height,max_x=0,max_y=0;
  int x, y, n, b;
  int occupied_reflectance=0, occupied_reflectance_hits=0;
  int empty_reflectance=0, empty_reflectance_hits=0;
  int quiet_zone_pixels=0, quiet_zone_occupancy=0;

  get_grid_bounding_box(grid, image_width, image_height,
                        &min_x, &min_y, &max_x, &max_y);

  /* calculate average reflectance for occupied and empty cells */
  for (y = min_y; y <= max_y; y++) {
    for (x = min_x; x <= max_x; x++) {
      if (point_in_quiet_zone(grid, x, y) == 0) continue;

      n = (y*image_width + x)*image_bytesperpixel;
      if (thresholded_image_data[n] != 0) {
        /* occupied cell */
        for (b = 0; b < image_bytesperpixel; b++) {
          occupied_reflectance += image_data[n+b];
        }
        occupied_reflectance_hits++;
        /* is this the quiet zone? */
        if (point_in_perimeter(grid, x, y) == 0) {
          quiet_zone_pixels++;
          quiet_zone_occupancy++;
        }
      }
      else {
        /* empty cell */
        for (b = 0; b < image_bytesperpixel; b++) {
          empty_reflectance += image_data[n+b];
        }
        empty_reflectance_hits++;
        /* is this the quiet zone? */
        if (point_in_perimeter(grid, x, y) == 0) {
          quiet_zone_pixels++;
        }
      }
    }
  }

  /* calculate occupancy of the quiet zone */
  grid->quiet_zone = 0;
  if (quiet_zone_pixels > 0) {
    grid->quiet_zone = (unsigned char)(100-(quiet_zone_occupancy * 100 / quiet_zone_pixels));
  }

  /* average occupied cell reflectance */
  if (occupied_reflectance_hits > 0) {
    occupied_reflectance /= occupied_reflectance_hits;
  }
  /* average empty cell reflectance */
  if (empty_reflectance_hits > 0) {
    empty_reflectance /= empty_reflectance_hits;
  }

  /* calculate global threshold, half way between occupied and empty reflectance */
  global_threshold = empty_reflectance + ((occupied_reflectance - empty_reflectance)/2);

  /* calculate the average of all cell modulation */
  modulation = 0;
  hits = 0;
  for (y = min_y; y <= max_y; y++) {
    for (x = min_x; x <= max_x; x++) {
      if (point_in_quiet_zone(grid, x, y) == 0) continue;

      n = (y*image_width + x)*image_bytesperpixel;
      reflectance = 0;
      for (b = 0; b < image_bytesperpixel; b++) {
        reflectance += image_data[n+b];
      }
      /* cell modulation
         from GS1 2D Barcode Verification Process Implementation Guideline 9.1.3 */
      cell_modulation = 2.0f * abs(reflectance - global_threshold) / symbol_contrast;
      modulation += cell_modulation;
      if (cell_modulation < min_modulation) {
        min_modulation = cell_modulation;
      }
      hits++;
    }
  }
  if (hits > 0) {
    modulation /= hits;
  }
  grid->modulation = (unsigned char)(modulation * 100);
  /* contrast uniformity is the minimum cell modulation */
  grid->contrast_uniformity = (unsigned char)(min_modulation * 100);
  /* calculate grade as per GS1 2D Barcode Verification Process Implementation Guideline
     table 9-2 */
  grid->modulation_grade = 0;
  if (grid->modulation >= 20) {
    grid->modulation_grade = 1;
  }
  if (grid->modulation >= 30) {
    grid->modulation_grade = 2;
  }
  if (grid->modulation >= 40) {
    grid->modulation_grade = 3;
  }
  if (grid->modulation >= 50) {
    grid->modulation_grade = 4;
  }
}

/**
 * \brief contrast between highest and lowest reflectance
 *        Also calculates minimum reflectance
 * \param grid grid object
 * \param image_data image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param mage_bitsperpixel Number of bits per pixel
 */
static void quality_metric_symbol_contrast(struct grid_2d * grid,
                                           unsigned char image_data[],
                                           int image_width, int image_height,
                                           int image_bitsperpixel)
{
  int image_bytesperpixel = image_bitsperpixel/8;
  int x, y, n, b, min_x=image_width, min_y=image_height, max_x=0, max_y=0;
  int reflectance, min_reflectance=-1, max_reflectance=0;

  get_grid_bounding_box(grid, image_width, image_height,
                        &min_x, &min_y, &max_x, &max_y);

  /* for all pixels inside the perimeter get the min and max reflectance */
  for (y = min_y; y <= max_y; y++) {
    for (x = min_x; x <= max_x; x++) {
      if (point_in_quiet_zone(grid, x, y) == 0) continue;

      n = (y*image_width + x)*image_bytesperpixel;
      reflectance = 0;
      for (b = 0; b < image_bytesperpixel; b++) {
        reflectance += image_data[n+b];
      }
      if ((min_reflectance == -1) || (reflectance < min_reflectance)) {
        min_reflectance = reflectance;
      }
      if (reflectance > max_reflectance) {
        max_reflectance = reflectance;
      }
    }
  }

  /* minimum reflectance grade */
  grid->minimum_reflectance = (unsigned char)(min_reflectance*100/(255*image_bytesperpixel));
  grid->minimum_reflectance_grade = 0;
  if (grid->minimum_reflectance < 50) {
    /* below half the maximum reflectance */
    grid->minimum_reflectance_grade = 4;
  }

  /* symbol contrast as a percentage */
  grid->symbol_contrast = (unsigned char)((max_reflectance - min_reflectance) * 100 / (255*image_bytesperpixel));
  /* calculate grade as per GS1 2D Barcode Verification Process Implementation Guideline
     table 9-1 */
  grid->symbol_contrast_grade = 0;
  if (grid->symbol_contrast >= 20) {
    grid->symbol_contrast_grade = 1;
  }
  if (grid->symbol_contrast >= 40) {
    grid->symbol_contrast_grade = 2;
  }
  if (grid->symbol_contrast >= 55) {
    grid->symbol_contrast_grade = 3;
  }
  if (grid->symbol_contrast >= 70) {
    grid->symbol_contrast_grade = 4;
  }
}

/**
 * \brief AS9132 calculate angle of distortion
 * \param grid grid object
 */
static void quality_metric_angle_of_distortion(struct grid_2d * grid)
{
  float corner_radians =
    corner_angle(grid->perimeter.x0, grid->perimeter.y0,
                 grid->perimeter.x1, grid->perimeter.y1,
                 grid->perimeter.x2, grid->perimeter.y2);
  if (corner_radians < 0) corner_radians = -corner_radians;
  if (corner_radians > PI)
    corner_radians = (2 * (float)PI) - corner_radians;
  float angle_degrees = corner_radians / (float)PI * 180;
  grid->angle_of_distortion = 90 - angle_degrees;
}

/**
 * \brief Calculates all quality metrics for symbol verification
 * \param grid grid object
 * \param image_data image array
 * \param thresholded_image_data binary image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param mage_bitsperpixel Number of bits per pixel
 */
void calculate_quality_metrics(struct grid_2d * grid,
                               unsigned char image_data[],
                               unsigned char thresholded_image_data[],
                               int image_width, int image_height,
                               int image_bitsperpixel)
{
  calculate_quiet_zone(grid);
  quality_metric_angle_of_distortion(grid);
  quality_metric_symbol_contrast(grid, image_data,
                                 image_width, image_height,
                                 image_bitsperpixel);
  quality_metric_axial_nonuniformity(grid);
  quality_metric_modulation(grid, image_data,
                            thresholded_image_data,
                            image_width, image_height,
                            image_bitsperpixel);
  quality_metric_grid_nonuniformity(grid, thresholded_image_data,
                                    image_width, image_height,
                                    image_bitsperpixel);
  quality_metric_distributed_damage(grid);

  /* calculate unused error correction grade as per
     GS1 2D Barcode Verification Process Implementation Guideline
     table 9-6 */
  grid->unused_error_correction_grade = 0;
  if (grid->unused_error_correction >= 25) {
    grid->unused_error_correction_grade = 1;
  }
  if (grid->unused_error_correction >= 37) {
    grid->unused_error_correction_grade = 2;
  }
  if (grid->unused_error_correction >= 50) {
    grid->unused_error_correction_grade = 3;
  }
  if (grid->unused_error_correction >= 62) {
    grid->unused_error_correction_grade = 4;
  }

  /* calculate clock track regularity grade as per
     GS1 2D Barcode Verification Process Implementation Guideline
     table 9-3 */
  grid->clock_track_regularity_grade = 0;
  if (grid->clock_track_regularity < 25) {
    grid->clock_track_regularity_grade = 1;
  }
  if (grid->clock_track_regularity < 20) {
    grid->clock_track_regularity_grade = 2;
  }
  if (grid->clock_track_regularity < 15) {
    grid->clock_track_regularity_grade = 3;
  }
  if (grid->clock_track_regularity < 10) {
    grid->clock_track_regularity_grade = 4;
  }

  /* calculate fixed pattern damage grade */
  grid->fixed_pattern_damage_grade = 0;
  if (grid->fixed_pattern_damage < 17) {
    grid->fixed_pattern_damage_grade = 1;
  }
  if (grid->fixed_pattern_damage < 13) {
    grid->fixed_pattern_damage_grade = 2;
  }
  if (grid->fixed_pattern_damage < 9) {
    grid->fixed_pattern_damage_grade = 3;
  }
  if (grid->fixed_pattern_damage == 0) {
    grid->fixed_pattern_damage_grade = 4;
  }
}

/**
 * \brief returns the overall quality grading
 *        See GS1 2D Barcode Verification Process Implementation
 *        Guideline 7.3
 * \param grid grid object
 * \return overall grade in the range 0-4
 */
static unsigned char overall_quality_grade(struct grid_2d * grid)
{
  unsigned char grade = grid->symbol_contrast_grade;
  if (grid->axial_non_uniformity_grade < grade) {
    grade = grid->axial_non_uniformity_grade;
  }
  if (grid->grid_non_uniformity_grade < grade) {
    grade = grid->grid_non_uniformity_grade;
  }
  if (grid->unused_error_correction_grade < grade) {
    grade = grid->unused_error_correction_grade;
  }
  if (grid->clock_track_regularity_grade < grade) {
    grade = grid->clock_track_regularity_grade;
  }
  if (grid->modulation_grade < grade) {
    grade = grid->modulation_grade;
  }
  if (grid->fixed_pattern_damage_grade < grade) {
    grade = grid->fixed_pattern_damage_grade;
  }
  if (grid->minimum_reflectance_grade < grade) {
    grade = grid->minimum_reflectance_grade;
  }
  return grade;
}

/**
 * \brief displays human readable quality metrics
 * \param grid grid object
 */
static void show_quality_metrics_human_readable(struct grid_2d * grid)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("Timestamp: %d-%02d-%02d %02d:%02d:%02d\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  printf("Symbol contrast: %d (%d%%)\n",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("Axial non-uniformity: %d (%.1f%%)\n",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("Grid non-uniformity: %d (%.1f%%)\n",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("Modulation: %d (%d%%)\n", (int)grid->modulation_grade, (int)grid->modulation);
  printf("Unused error correction: %d (%d%%)\n",
         (int)grid->unused_error_correction_grade, (int)grid->unused_error_correction);
  printf("Clock track regularity: %d (%d%%)\n",
         (int)grid->clock_track_regularity_grade, (int)grid->clock_track_regularity);
  printf("Fixed pattern damage: %d (%d%%)\n",
         (int)grid->fixed_pattern_damage_grade, (int)grid->fixed_pattern_damage);
  printf("Minimum reflectance: %d (%d%%)\n",
         (int)grid->minimum_reflectance_grade, (int)grid->minimum_reflectance);
  printf("Overall symbol grade: %d.0 (%c)\n\n", (int)grade, grade_letter[grade]);
  printf("Matrix size: %dx%d\n", grid->dimension_x, grid->dimension_y);
  printf("Angle of distortion: %.1f°\n", grid->angle_of_distortion);
  printf("Contrast uniformity: %d%%\n", (int)grid->contrast_uniformity);
  printf("Dots per element: %d\n", grid->dots_per_element);
  printf("Elongation: %.1f%%\n", grid->elongation);
  printf("Quiet zone: %d%%\n", (int)grid->quiet_zone);
  printf("Distributed damage: %d%%\n", (int)grid->distributed_damage);
  printf("Cell fill: %d%%\n", (int)grid->cell_fill);
}

/**
 * \brief displays quality metrics in CSV format
 * \param grid grid object
 */
static void show_quality_metrics_csv(struct grid_2d * grid)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("Metric, Grade, Value,\n");

  printf("Timestamp,, \"%d-%02d-%02d %02d:%02d:%02d\",\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  printf("Symbol contrast, %d, %d,\n",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("Axial non-uniformity, %d, \"%.1f\",\n",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("Grid non-uniformity, %d, \"%.1f\",\n",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("Modulation, %d, %d,\n", (int)grid->modulation_grade, (int)grid->modulation);
  printf("Unused error correction, %d, %d,\n",
         (int)grid->unused_error_correction_grade, (int)grid->unused_error_correction);
  printf("Clock track regularity, %d, %d,\n",
         (int)grid->clock_track_regularity_grade, (int)grid->clock_track_regularity);
  printf("Fixed pattern damage, %d, %d,\n",
         (int)grid->fixed_pattern_damage_grade, (int)grid->fixed_pattern_damage);
  printf("Minimum reflectance, %d, %d,\n",
         (int)grid->minimum_reflectance_grade, (int)grid->minimum_reflectance);
  printf("Overall symbol grade, %d.0, \"%c\",\n", (int)grade, grade_letter[grade]);
  printf("Angle of distortion,, \"%.1f\",\n", grid->angle_of_distortion);
  printf("Contrast uniformity,, %d,\n", (int)grid->contrast_uniformity);
  printf("Dots per element,, %d,\n", grid->dots_per_element);
  printf("Elongation,, \"%.1f\",\n", grid->elongation);
  printf("Quiet zone,, %d,\n", (int)grid->quiet_zone);
  printf("Distributed damage,, %d,\n", (int)grid->distributed_damage);
  printf("Cell fill,, %d\n", (int)grid->cell_fill);
}

/**
 * \brief displays quality metrics in json format
 * \param grid grid object
 */
static void show_quality_metrics_json(struct grid_2d * grid)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("{\n");
  printf("  \"timestamp\": \"%d-%02d-%02d %02d:%02d:%02d\",\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  printf("  \"symbol_contrast\": { \"grade\": %d, \"value\": %d},\n",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("  \"axial_non_uniformity\": { \"grade\": %d, \"value\": %.1f},\n",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("  \"grid_non_uniformity\": { \"grade\": %d, \"value\": %.1f},\n",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("  \"modulation\": { \"grade\": %d, \"value\": %d},\n",
         (int)grid->modulation_grade, (int)grid->modulation);
  printf("  \"unused_error_correction\": { \"grade\": %d, \"value\": %d},\n",
         (int)grid->unused_error_correction_grade, (int)grid->unused_error_correction);
  printf("  \"clock_track_regularity\": { \"grade\": %d, \"value\": %d},\n",
         (int)grid->clock_track_regularity_grade, (int)grid->clock_track_regularity);
  printf("  \"fixed_pattern_damage\": { \"grade\": %d, \"value\": %d},\n",
         (int)grid->fixed_pattern_damage_grade, (int)grid->fixed_pattern_damage);
  printf("  \"minimum_reflectance\": { \"grade\": %d, \"value\": %d},\n",
         (int)grid->minimum_reflectance_grade, (int)grid->minimum_reflectance);
  printf("  \"overall_symbol_grade\": { \"grade\": %d.0, \"value\": \"%c\"},\n", (int)grade, grade_letter[grade]);
  printf("  \"angle_of_distortion\": %.1f,\n", grid->angle_of_distortion);
  printf("  \"contrast_uniformity\": %d,\n", (int)grid->contrast_uniformity);
  printf("  \"dots_per_element\": %d,\n", grid->dots_per_element);
  printf("  \"elongation\": %.1f,\n", grid->elongation);
  printf("  \"quiet_zone\": %d,\n", (int)grid->quiet_zone);
  printf("  \"distributed_damage\": %d,\n", (int)grid->distributed_damage);
  printf("  \"cell_fill\": %d,\n", (int)grid->cell_fill);
}

/**
 * \brief displays quality metrics
 * \param grid grid object
 */
void show_quality_metrics(struct grid_2d * grid,
                          unsigned char csv,
                          unsigned char json)
{
  if (csv == 1) {
    show_quality_metrics_csv(grid);
    return;
  }
  else if (json == 1) {
    show_quality_metrics_json(grid);
    return;
  }
  show_quality_metrics_human_readable(grid);
}
