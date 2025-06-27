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

static void calculate_quiet_zone(struct grid_2d * grid)
{
  int i;
  float centre_x=0, centre_y=0,dx,dy, fraction, side_length, cell_width;
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
  dx = grid->perimeter.x1 - grid->perimeter.x0;
  dy = grid->perimeter.y1 - grid->perimeter.y0;
  side_length = (float)sqrt(dx*dx + dy*dy);
  cell_width = side_length / grid->dimension_x;
  fraction = (side_length + cell_width) / side_length;

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

/* contrast between highest and lowest reflectance */
static void quality_matric_symbol_contrast(struct grid_2d * grid,
                                           unsigned char image_data[],
                                           int image_width, int image_height,
                                           int image_bitsperpixel)
{
  int image_bytesperpixel = image_bitsperpixel/8;
  int x, y, n, b, i, min_x=image_width, min_y=image_height, max_x=0, max_y=0;
  int reflectance, min_reflectance=-1, max_reflectance=0;
  /* NOTE: the perimeter should be expanded to include quiet zone */
  calculate_quiet_zone(grid);
  int points[4*2] = {
    (int)grid->quiet_zone_perimeter.x0, (int)grid->quiet_zone_perimeter.y0,
    (int)grid->quiet_zone_perimeter.x1, (int)grid->quiet_zone_perimeter.y1,
    (int)grid->quiet_zone_perimeter.x2, (int)grid->quiet_zone_perimeter.y2,
    (int)grid->quiet_zone_perimeter.x3, (int)grid->quiet_zone_perimeter.y3
  };
  /* get the bounding box for the perimeter */
  for (i = 0; i < 4; i++) {
    if (points[i*2] < min_x) min_x = points[i*2];
    if (points[i*2+1] < min_y) min_y = points[i*2+1];
    if (points[i*2] > max_x) max_x = points[i*2];
    if (points[i*2+1] > max_y) max_y = points[i*2+1];
  }

  /* ensure that the bounding box is inside of the image */
  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x >= image_width) max_x = image_width-1;
  if (max_y >= image_height) max_y = image_height-1;

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
  quality_metric_angle_of_distortion(grid);
  quality_matric_symbol_contrast(grid, image_data,
                                 image_width, image_height,
                                 image_bitsperpixel);
}

void show_quality_metrics(struct grid_2d * grid)
{
  printf("Fixed pattern damage: %d%%\n", (int)grid->fixed_pattern_damage);
  printf("Angle of distortion: %.1f degrees\n", grid->angle_of_distortion);
  printf("Symbol contrast: %d%%\n", (int)grid->symbol_contrast);
}
