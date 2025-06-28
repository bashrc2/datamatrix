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

/* returns 1 if the given point is in the grid quiet zone perimeter */
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

/* returns 1 if the given point is within the grid perimeter */
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

/* get the offset of a grid cell from its theoretical centre */
static void grid_nonuniformity_test_cell(unsigned char thresholded_image_data[],
                                         int image_width, int image_height,
                                         int image_bytesperpixel,
                                         int x, int y, int radius,
                                         int * offset_x, int * offset_y,
                                         float * elongation,
                                         int * no_of_pixels)
{
  int search_x, search_y, av_x=0, av_y=0, hits=0;
  int cell_width, cell_height;
  int min_x=image_width, min_y=image_height, max_x=0, max_y=0;
  int n = (y*image_width + x)*image_bytesperpixel;

  *offset_x = NO_OFFSET;
  *elongation = 0;
  *no_of_pixels = 0;
  if (thresholded_image_data[n] == 0) return;

  for (search_y = y - radius; search_y <= y + radius; search_y++) {
    if ((search_y < 0) || (search_y >= image_height)) continue;
    for (search_x = x - radius; search_x <= x + radius; search_x++) {
      if ((search_x < 0) || (search_x >= image_width)) continue;
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

static void quality_metric_grid_nonuniformity(struct grid_2d * grid,
                                              unsigned char thresholded_image_data[],
                                              int image_width, int image_height,
                                              int image_bitsperpixel)
{
  int image_bytesperpixel = image_bitsperpixel/8;
  int grid_x, grid_y, offset_x=0, offset_y=0;
  int av_offset_x=0, av_offset_y=0, offset_hits=0;
  int cell_no_of_pixels, av_cell_no_of_pixels=0;
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
                                     &cell_no_of_pixels);
        if (offset_x != NO_OFFSET) {
          av_offset_x += abs(offset_x);
          av_offset_y += abs(offset_y);
          av_elongation += cell_elongation;
          av_cell_no_of_pixels += cell_no_of_pixels;
          offset_hits++;
        }
      }
    }
  }

  grid->grid_non_uniformity = 0;
  grid->grid_non_uniformity_grade = 0;
  grid->elongation = 0;
  grid->dots_per_element = 0;
  if (offset_hits > 0) {
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

/* calculate axial non-uniformity as the percent difference between
   cell width and height */
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
  grid->axial_non_uniformity = (1.0f - (cell_width_shortest/cell_width_longest))*100;
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

/* returns the bounding box for the grid, including the quiet zone */
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

/* calculate grid cell modulation */
static void quality_metric_modulation(struct grid_2d * grid,
                                      unsigned char image_data[],
                                      unsigned char thresholded_image_data[],
                                      int image_width, int image_height,
                                      int image_bitsperpixel)
{
  int min_x=image_width,min_y=image_height,max_x=0,max_y=0;
  int image_bytesperpixel = image_bitsperpixel/8;
  int x, y, n, b, cell_modulation, occupied_modulation=0, empty_modulation=0;
  int occupied_reflectance=0, occupied_reflectance_hits=0;
  int empty_reflectance=0, empty_reflectance_hits=0;
  int occupied_modulation_variance=0,empty_modulation_variance=0;
  int light_dark_variance, quiet_zone_pixels=0, quiet_zone_occupancy=0;
  float empty_modulation_variance_fraction=0,occupied_modulation_variance_fraction=0;

  get_grid_bounding_box(grid, image_width, image_height,
                        &min_x, &min_y, &max_x, &max_y);

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
  /* variance between average reflectance of occupied and empty cells */
  light_dark_variance = abs(occupied_reflectance - empty_reflectance);

  /* how much does each cell differ from the expected average reflectance? */
  for (y = min_y; y <= max_y; y++) {
    for (x = min_x; x <= max_x; x++) {
      if (point_in_quiet_zone(grid, x, y) == 0) continue;

      n = (y*image_width + x)*image_bytesperpixel;
      cell_modulation = 0;
      if (thresholded_image_data[n] != 0) {
        /* occupied cell */
        for (b = 0; b < image_bytesperpixel; b++) {
          cell_modulation += image_data[n+b];
        }
        occupied_modulation += abs(cell_modulation - occupied_reflectance);
      }
      else {
        /* empty cell */
        for (b = 0; b < image_bytesperpixel; b++) {
          cell_modulation += image_data[n+b];
        }
        empty_modulation += abs(cell_modulation - empty_reflectance);
      }
    }
  }

  if (light_dark_variance > 0) {
    if (occupied_reflectance_hits > 0) {
      occupied_modulation_variance = occupied_modulation / occupied_reflectance_hits;
      occupied_modulation_variance_fraction =
        occupied_modulation_variance / (float)(light_dark_variance * image_bytesperpixel);
    }
    if (empty_reflectance_hits > 0) {
      empty_modulation_variance = empty_modulation / empty_reflectance_hits;
      empty_modulation_variance_fraction =
        empty_modulation_variance / (float)(light_dark_variance * image_bytesperpixel);
    }
  }

  grid->modulation =
    (unsigned char)((occupied_modulation_variance_fraction + empty_modulation_variance_fraction) * 50);
}

/* contrast between highest and lowest reflectance */
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

  /* symbol contrast as a percentage */
  grid->symbol_contrast = (unsigned char)((max_reflectance - min_reflectance) * 100 / (255*image_bytesperpixel));
  /* calculate grade as per GS1 2D Barcode Verification Process Implementation Guideline
     table 7-3 */
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

/* AS9132 calculate angle of distortion */
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
}

void show_quality_metrics(struct grid_2d * grid)
{
  printf("Symbol contrast: %d (%d%%)\n",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("Axial non-uniformity: %d (%.1f%%)\n",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("Grid non-uniformity: %d (%.1f%%)\n",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("Modulation: %d%%\n", (int)grid->modulation);
  printf("Unused error correction: %d%%\n", (int)grid->unused_error_correction);
  printf("Fixed pattern damage: %d%%\n", (int)grid->fixed_pattern_damage);
  printf("Angle of distortion: %.1f degrees\n", grid->angle_of_distortion);
  printf("Elongation: %.1f%%\n", grid->elongation);
  printf("Dots per element: %d\n", grid->dots_per_element);
  printf("Quiet zone: %d%%\n", (int)grid->quiet_zone);
}
