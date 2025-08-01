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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "png2.h"
#include "datamatrix.h"

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
 * \brief saves a reflectance histogram
 * \param image_data original image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bitsperpixel Number of bits per pixel
 * \param grid grib object
 * \param histogram_image_width width of the histogram image
 * \param histogram_image_height height of the histogram image
 * \param r red
 * \param g green
 * \param b blue
 * \param module_centres Sample only the module centres
 * \param filename filename for the histogram image
 */
static void save_reflectance_histogram(unsigned char image_data[],
                                       int image_width, int image_height,
                                       int image_bitsperpixel,
                                       struct grid_2d * grid,
                                       int histogram_image_width,
                                       int histogram_image_height,
                                       int r, int g, int b,
                                       unsigned char module_centres,
                                       char filename[])
{
  const int border_percent = 5;
  const int sampling_radius = 2;
  float xi=0, yi=0, grid_pos_x, grid_pos_y;
  float horizontal_x1, horizontal_y1, horizontal_x2, horizontal_y2;
  float vertical_x1, vertical_y1, vertical_x2, vertical_y2;
  int axes_width = 1;
  int axes_r = 0;
  int axes_g = 0;
  int axes_b = 0;
  int image_bytesperpixel = image_bitsperpixel/8;
  int grid_x, grid_y, x, y, n, bb, reflectance, tx, ty, bx, by;
  int border_tx, border_ty, border_bx, border_by;
  unsigned int mean_reflectance = 0;
  unsigned int mean_reflectance_hits = 0;
  unsigned int max=0;
  unsigned int histogram[256];
  unsigned char * histogram_image =
    (unsigned char*)safemalloc(histogram_image_width*histogram_image_height*3*
                           sizeof(unsigned char));

  /* clear the histogram */
  memset(&histogram[0], 0, 256*sizeof(unsigned int));

  if (module_centres == 0) {
    /* calculate the histogram for all pixels inside the quiet zone perimeter */
    for (y = 0; y < image_height; y++) {
      for (x = 0; x < image_width; x++) {
        if (point_in_quiet_zone(grid, x, y) == 0) continue;
        n = (y*image_width + x)*image_bytesperpixel;
        reflectance = 0;
        for (bb = 0; bb < image_bytesperpixel; bb++, n++) {
          reflectance += image_data[n];
        }
        reflectance /= image_bytesperpixel;
        histogram[reflectance]++;
      }
    }
  }
  else {
    /* calculate the histogram by sampling only the module centres */
    float horizontal_dx1 = grid->perimeter.x3 - grid->perimeter.x0;
    float horizontal_dy1 = grid->perimeter.y3 - grid->perimeter.y0;
    float horizontal_dx2 = grid->perimeter.x2 - grid->perimeter.x1;
    float horizontal_dy2 = grid->perimeter.y2 - grid->perimeter.y1;

    float vertical_dx1 = grid->perimeter.x1 - grid->perimeter.x0;
    float vertical_dy1 = grid->perimeter.y1 - grid->perimeter.y0;
    float vertical_dx2 = grid->perimeter.x2 - grid->perimeter.x3;
    float vertical_dy2 = grid->perimeter.y2 - grid->perimeter.y3;

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
          if ((xi >= sampling_radius) && (xi <= image_width - 1 - sampling_radius) &&
              (yi >= sampling_radius) && (yi <= image_height - 1 - sampling_radius)) {
            tx = (int)xi - sampling_radius;
            ty = (int)yi - sampling_radius;
            bx = (int)xi + sampling_radius;
            by = (int)yi + sampling_radius;
            for (y = ty; y <= by; y++) {
              for (x = tx; x <= bx; x++) {
                n = (y * image_width + x)*image_bytesperpixel;

                reflectance = 0;
                for (bb = 0; bb < image_bytesperpixel; bb++, n++) {
                  reflectance += image_data[n];
                }
                reflectance /= image_bytesperpixel;
                histogram[reflectance]++;
              }
            }
          }
        }
      }
    }
  }

  /* find maximum histogram response */
  max = 1;
  for (reflectance = 0; reflectance < 256; reflectance++) {
    if (histogram[reflectance] > 0) {
      mean_reflectance += histogram[reflectance];
      mean_reflectance_hits++;
    }
  }
  if (mean_reflectance_hits > 0) {
    max = (mean_reflectance / mean_reflectance_hits) * 25 / 10;
  }

  /* clear the image */
  memset(histogram_image, 255,
         histogram_image_width*histogram_image_height*3*sizeof(unsigned char));

  /* calculate border inside image */
  border_tx = histogram_image_width*border_percent/100;
  border_ty = histogram_image_height*border_percent/100;
  border_bx = histogram_image_width - 1 - border_tx;
  border_by = histogram_image_height - 1 - border_ty;

  /* grey background */
  for (y = border_ty; y < border_by; y++) {
    for (x = border_tx; x < border_bx; x++) {
      n = (y*histogram_image_width + x)*image_bytesperpixel;
      for (bb = 0; bb < image_bytesperpixel; bb++, n++) {
        histogram_image[n] = 210;
      }
    }
  }

  /* draw the histogram */
  for (x = border_tx; x <= border_bx; x++) {
    reflectance = (x - border_tx) * 255 / (border_bx - border_tx);
    reflectance = histogram[reflectance];
    if (reflectance > max) reflectance = max;
    y = border_by - (reflectance * (border_by - border_ty) / max);
    draw_line(histogram_image,
              histogram_image_width, histogram_image_height, 24,
              x, y, x, border_by, 1, r, g, b);
  }

  /* draw axes */
  draw_line(histogram_image,
            histogram_image_width, histogram_image_height, 24,
            border_tx, border_ty, border_tx, border_by,
            axes_width,
            axes_r, axes_g, axes_b);
  draw_line(histogram_image,
            histogram_image_width, histogram_image_height, 24,
            border_bx, border_ty, border_bx, border_by,
            axes_width,
            axes_r, axes_g, axes_b);
  draw_line(histogram_image,
            histogram_image_width, histogram_image_height, 24,
            border_tx, border_by, border_bx, border_by,
            axes_width,
            axes_r, axes_g, axes_b);
  draw_line(histogram_image,
            histogram_image_width, histogram_image_height, 24,
            border_tx, border_ty, border_bx, border_ty,
            axes_width,
            axes_r, axes_g, axes_b);

  /* save PNG file */
  write_png_file(filename,
                 histogram_image_width, histogram_image_height, 24,
                 histogram_image);
  free(histogram_image);
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
 *        also return cell fill percentage
 * \param thresholded_image_data thresholded (binary) image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bytesperpixel Number of bytes per pixel
 * \param x theoretical/ideal x coordinate of the centre of the grid cell
 * \param y theoretical/ideal y coordinate of the centre of the grid cell
 * \param radius radius of the grid cell
 * \param offset_x returned x offset of the cell centre
 * \param offset_y returned y offset of the cell centre
 * \param elongation returned elongation of the cell
 * \param no_of_pixels returned number of pixels within the cell
 * \param cell_fill returned percentage of the cell which is filled
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
 * \brief updates an image showing cell shape for the given cell
 * \param thresholded_image_data thresholded (binary) image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bytesperpixel Number of bytes per pixel
 * \param x theoretical/ideal x coordinate of the centre of the grid cell
 * \param y theoretical/ideal y coordinate of the centre of the grid cell
 * \param radius radius of the grid cell
 * \param cell_shape_image image showing cell shape
 * \param cell_shape_image_width width of the cell shape image
 * \param cell_shape_image_height height of the cell shape image
 * \param cell_shape_bytesperpixel Number of bytes per pixel in cell shape image
 */
static void grid_cell_shape_test_cell(unsigned char thresholded_image_data[],
                                      int image_width, int image_height,
                                      int image_bytesperpixel,
                                      int x, int y, int radius,
                                      unsigned char cell_shape_image[],
                                      int cell_shape_image_width,
                                      int cell_shape_image_height,
                                      int cell_shape_bytesperpixel)
{
  int n2, n = (y*image_width + x)*image_bytesperpixel;
  int half_cell_shape_image_width = cell_shape_image_width/2;
  int half_cell_shape_image_height = cell_shape_image_height/2;
  int cell_shape_cx = cell_shape_image_width/2;
  int cell_shape_cy = cell_shape_image_height/2;
  int search_x, search_y, xx, yy, b;
  int cell_shape_x0, cell_shape_y0;
  int cell_shape_x1, cell_shape_y1;

  if (thresholded_image_data[n] == 0) return;

  for (search_y = y - radius; search_y <= y + radius; search_y++) {
    if ((search_y < 0) || (search_y >= image_height)) continue;
    for (search_x = x - radius; search_x <= x + radius; search_x++) {
      if ((search_x < 0) || (search_x >= image_width)) continue;
      n = (search_y*image_width + search_x)*image_bytesperpixel;
      if (thresholded_image_data[n] != 0) {
        /* top left and bottom right coordinates of the pixel in the cell shape image */
        cell_shape_x0 =
          cell_shape_cx + ((search_x - x)*half_cell_shape_image_width/radius);
        if ((cell_shape_x0 < 0) || (cell_shape_x0 >= cell_shape_image_width)) continue;
        cell_shape_y0 =
          cell_shape_cy + ((search_y - y)*half_cell_shape_image_height/radius);
        if ((cell_shape_y0 < 0) || (cell_shape_y0 >= cell_shape_image_height)) continue;
        cell_shape_x1 =
          cell_shape_cx + ((search_x + 1 - x)*half_cell_shape_image_width/radius);
        if ((cell_shape_x1 < 0) || (cell_shape_x1 >= cell_shape_image_width)) continue;
        cell_shape_y1 =
          cell_shape_cy + ((search_y + 1 - y)*half_cell_shape_image_height/radius);
        if ((cell_shape_y1 < 0) || (cell_shape_y1 >= cell_shape_image_height)) continue;

        /* update the pixel */
        for (yy = cell_shape_y0; yy < cell_shape_y1; yy++) {
          for (xx = cell_shape_x0; xx < cell_shape_x1; xx++) {
            n2 = (yy*cell_shape_image_width + xx)*cell_shape_bytesperpixel;
            if (cell_shape_image[n2] < 255) {
              for (b = cell_shape_bytesperpixel-1; b >= 0; b--) {
                cell_shape_image[n2+b]++;
              }
            }
          }
        }
      }
    }
  }
}

/**
 * \brief creates an image showing the shape of cells win the grid
 * \param grid grid object
 * \param thresholded_image_data thresholded (binary) image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bitsperpixel Number of bits per pixel
 * \param cell_shape_image_width width of the cell shape image
 * \param cell_shape_image_height height of the cell shape image
 * \param cell_shape_bitsperpixel Number of bits per pixel in the cell shape image
 * \param filename filename to save as
 */
static void save_grid_cell_shape(struct grid_2d * grid,
                                 unsigned char thresholded_image_data[],
                                 int image_width, int image_height,
                                 int image_bitsperpixel,
                                 int cell_shape_image_width,
                                 int cell_shape_image_height,
                                 char filename[])
{
  const int cell_shape_bitsperpixel = 24;
  float xi, yi, grid_pos_x, grid_pos_y;
  float horizontal_x1, horizontal_y1, horizontal_x2, horizontal_y2;
  float vertical_x1, vertical_y1, vertical_x2, vertical_y2;
  int i, b, max, grid_x, grid_y;
  unsigned char value;
  int image_bytesperpixel = image_bitsperpixel/8;
  int cell_shape_bytesperpixel = cell_shape_bitsperpixel/8;
  float elongation = 0;
  int no_of_pixels = 0;
  int cell_fill = 0;

  if (strlen(filename) == 0) return;

  /* create the image */
  unsigned char * cell_shape_image =
    (unsigned char*)safemalloc(cell_shape_image_width*
                               cell_shape_image_height*
                               cell_shape_bytesperpixel*
                               sizeof(unsigned char));

  /* clear the image */
  memset(cell_shape_image, 0,
         cell_shape_image_width*cell_shape_image_height*
         cell_shape_bytesperpixel*sizeof(unsigned char));

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
  if (cell_radius < 1) cell_radius = 1;

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
        if (((int)xi >= 0) && ((int)yi > 0) &&
            ((int)xi < image_width) && ((int)yi < image_height)) {
          int offset_x=0, offset_y=0;
          grid_nonuniformity_test_cell(thresholded_image_data,
                                       image_width, image_height,
                                       image_bytesperpixel,
                                       (int)xi, (int)yi, cell_radius,
                                       &offset_x, &offset_y,
                                       &elongation,
                                       &no_of_pixels,
                                       &cell_fill);
          if (((int)xi-offset_x >= 0) && ((int)yi-offset_y > 0) &&
              ((int)xi-offset_x < image_width) && ((int)yi-offset_y < image_height)) {
            grid_cell_shape_test_cell(thresholded_image_data,
                                      image_width, image_height,
                                      image_bytesperpixel,
                                      (int)xi-offset_x, (int)yi-offset_y, cell_radius,
                                      cell_shape_image,
                                      cell_shape_image_width,
                                      cell_shape_image_height,
                                      cell_shape_bytesperpixel);
          }
        }
      }
    }
  }

  /* find the maximum values */
  max = 1;
  for (i = cell_shape_image_width*cell_shape_image_height-1; i >= 0; i--) {
    if (cell_shape_image[i*cell_shape_bytesperpixel] > max) {
      max = cell_shape_image[i*cell_shape_bytesperpixel];
    }
  }
  max = max * 110 / 100;
  for (i = cell_shape_image_width*cell_shape_image_height-1; i >= 0; i--) {
    value = (unsigned char)(255-(cell_shape_image[i*cell_shape_bytesperpixel] * 255 / max));
    for (b = cell_shape_bytesperpixel-1; b >= 0; b--) {
      cell_shape_image[i*cell_shape_bytesperpixel+b] = value;
    }
  }

  /* save PNG file */
  write_png_file(filename,
                 cell_shape_image_width, cell_shape_image_height,
                 cell_shape_bitsperpixel, cell_shape_image);
  free(cell_shape_image);
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
  if (cell_radius < 1) cell_radius = 1;

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
        if (((int)xi >= 0) && ((int)yi > 0) &&
            ((int)xi < image_width) && ((int)yi < image_height)) {
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
  grid->distributed_damage = 0;
  if (cells > 0) {
    grid->distributed_damage = (unsigned char)(damage * 100 / cells);
  }
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
  grid->angle_of_distortion = 0;
  if (!isnan(angle_degrees)) {
    grid->angle_of_distortion = 90 - angle_degrees;
  }
}

/**
 * \brief Calculates all quality metrics for symbol verification
 * \param grid grid object
 * \param image_data image array
 * \param thresholded_image_data binary image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param mage_bitsperpixel Number of bits per pixel
 * \param histogram_module_centres sample only the module centres to produce the histogram
 * \param histogram_filename optionally save a reflectance histogram
 * \param cell_shape_filename optional cell shape image
 */
void calculate_quality_metrics(struct grid_2d * grid,
                               unsigned char image_data[],
                               unsigned char thresholded_image_data[],
                               int image_width, int image_height,
                               int image_bitsperpixel,
                               unsigned char histogram_module_centres,
                               char histogram_filename[],
                               char cell_shape_filename[])
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

  if (strlen(histogram_filename) > 0) {
    save_reflectance_histogram(image_data,
                               image_width, image_height,
                               image_bitsperpixel,
                               grid, 800, 600, 0,0,0,
                               histogram_module_centres,
                               histogram_filename);
  }

  int cell_shape_image_width = image_width;
  int cell_shape_image_height = image_width;
  save_grid_cell_shape(grid,
                       thresholded_image_data,
                       image_width, image_height,
                       image_bitsperpixel,
                       cell_shape_image_width,
                       cell_shape_image_height,
                       cell_shape_filename);
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
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 */
static void show_quality_metrics_human_readable(struct grid_2d * grid,
                                                float aperture,
                                                int light_nm,
                                                int light_angle_degrees)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("Timestamp: %d-%02d-%02d %02d:%02d:%02d\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  if (grid->gs1_datamatrix == 1) {
    printf("Symbol type: GS1 datamatrix\n");
  }
  else {
    printf("Symbol type: datamatrix\n");
  }

  if (aperture > 0) {
    printf("Aperture: %.1f\n", aperture);
  }
  printf("Light (nm): %d\n", light_nm);
  printf("Light angle: %d°\n", light_angle_degrees);
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
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 */
static void show_quality_metrics_csv(struct grid_2d * grid,
                                     float aperture,
                                     int light_nm,
                                     int light_angle_degrees)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("\"%d-%02d-%02d %02d:%02d:%02d\", ",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  if (grid->gs1_datamatrix == 1) {
    printf("\"GS1 datamatrix\", ");
  }
  else {
    printf("\"datamatrix\", ");
  }

  if (aperture > 0) {
    printf("\"%.1f\", ", aperture);
  }
  printf("%d, ", light_nm);
  printf("%d, ", light_angle_degrees);
  printf("%d, %d, ",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("%d, \"%.1f\", ",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("%d, \"%.1f\", ",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("%d, %d, ", (int)grid->modulation_grade, (int)grid->modulation);
  printf("%d, %d, ",
         (int)grid->unused_error_correction_grade, (int)grid->unused_error_correction);
  printf("%d, %d, ",
         (int)grid->clock_track_regularity_grade, (int)grid->clock_track_regularity);
  printf("%d, %d, ",
         (int)grid->fixed_pattern_damage_grade, (int)grid->fixed_pattern_damage);
  printf("%d, %d, ",
         (int)grid->minimum_reflectance_grade, (int)grid->minimum_reflectance);
  printf("%d.0, \"%c\", ", (int)grade, grade_letter[grade]);
  printf("\"%.1f\", ", grid->angle_of_distortion);
  printf("%d, ", (int)grid->contrast_uniformity);
  printf("%d, ", grid->dots_per_element);
  printf("\"%.1f\", ", grid->elongation);
  printf("%d, ", (int)grid->quiet_zone);
  printf("%d, ", (int)grid->distributed_damage);
  printf("%d, ", (int)grid->cell_fill);
}

/**
 * \brief displays quality metrics in json format
 * \param grid grid object
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 */
static void show_quality_metrics_json(struct grid_2d * grid,
                                      float aperture,
                                      int light_nm,
                                      int light_angle_degrees)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("{\n");
  printf("  \"timestamp\": \"%d-%02d-%02d %02d:%02d:%02d\",\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  printf("  \"issuedate\": \"%d-%02d-%02d\",\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  if (grid->gs1_datamatrix == 1) {
    printf("  \"symbol_type\": \"GS1 datamatrix\",\n");
  }
  else {
    printf("  \"symbol_type\": \"datamatrix\",\n");
  }
  if (aperture > 0) {
    printf("  \"aperture\": %.1f,\n", aperture);
  }
  printf("  \"light_nm\": %d,\n", light_nm);
  printf("  \"light_angle_degrees\": %d,\n", light_angle_degrees);
  printf("  \"symbol_contrast\": { \"grade\": %d, \"value\": %d },\n",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("  \"axial_non_uniformity\": { \"grade\": %d, \"value\": %.1f },\n",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("  \"grid_non_uniformity\": { \"grade\": %d, \"value\": %.1f },\n",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("  \"modulation\": { \"grade\": %d, \"value\": %d },\n",
         (int)grid->modulation_grade, (int)grid->modulation);
  printf("  \"unused_error_correction\": { \"grade\": %d, \"value\": %d },\n",
         (int)grid->unused_error_correction_grade, (int)grid->unused_error_correction);
  printf("  \"clock_track_regularity\": { \"grade\": %d, \"value\": %d },\n",
         (int)grid->clock_track_regularity_grade, (int)grid->clock_track_regularity);
  printf("  \"fixed_pattern_damage\": { \"grade\": %d, \"value\": %d },\n",
         (int)grid->fixed_pattern_damage_grade, (int)grid->fixed_pattern_damage);
  printf("  \"minimum_reflectance\": { \"grade\": %d, \"value\": %d },\n",
         (int)grid->minimum_reflectance_grade, (int)grid->minimum_reflectance);
  printf("  \"overall_symbol_grade\": { \"grade\": %d.0, \"value\": \"%c\" },\n", (int)grade, grade_letter[grade]);
  printf("  \"angle_of_distortion\": %.1f,\n", grid->angle_of_distortion);
  printf("  \"contrast_uniformity\": %d,\n", (int)grid->contrast_uniformity);
  printf("  \"dots_per_element\": %d,\n", grid->dots_per_element);
  printf("  \"elongation\": %.1f,\n", grid->elongation);
  printf("  \"quiet_zone\": %d,\n", (int)grid->quiet_zone);
  printf("  \"distributed_damage\": %d,\n", (int)grid->distributed_damage);
  printf("  \"cell_fill\": %d,\n", (int)grid->cell_fill);
}

/**
 * \brief displays quality metrics in yaml format
 * \param grid grid object
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 */
static void show_quality_metrics_yaml(struct grid_2d * grid,
                                      float aperture,
                                      int light_nm,
                                      int light_angle_degrees)
{
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("---\n");
  printf("issuedate: %d-%02d-%02d\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  printf("timestamp: %d-%02d-%02d %02d:%02d:%02d\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
         tm.tm_hour, tm.tm_min, tm.tm_sec);
  if (grid->gs1_datamatrix == 1) {
    printf("symbol_type: GS1 datamatrix\n");
  }
  else {
    printf("symbol_type: datamatrix\n");
  }
  printf("\n# Illumination\n");
  if (aperture > 0) {
    printf("aperture: %.1f\n", aperture);
  }
  printf("light_nm: %d\n", light_nm);
  printf("light_angle_degrees: %d\n\n", light_angle_degrees);
  printf("# Quality metrics\n");
  printf("symbol_contrast:\n- grade: %d\n  value: %d\n",
         (int)grid->symbol_contrast_grade, (int)grid->symbol_contrast);
  printf("axial_non_uniformity:\n- grade: %d\n  value: %.1f\n",
         (int)grid->axial_non_uniformity_grade, grid->axial_non_uniformity);
  printf("grid_non_uniformity:\n- grade: %d\n- value: %.1f\n",
         (int)grid->grid_non_uniformity_grade, grid->grid_non_uniformity);
  printf("modulation:\n- grade: %d\n  value: %d\n",
         (int)grid->modulation_grade, (int)grid->modulation);
  printf("unused_error_correction:\n- grade: %d\n  value: %d\n",
         (int)grid->unused_error_correction_grade, (int)grid->unused_error_correction);
  printf("clock_track_regularity:\n- grade: %d\n  value: %d\n",
         (int)grid->clock_track_regularity_grade, (int)grid->clock_track_regularity);
  printf("fixed_pattern_damage:\n- grade: %d\n  value: %d\n",
         (int)grid->fixed_pattern_damage_grade, (int)grid->fixed_pattern_damage);
  printf("minimum_reflectance:\n- grade: %d\n  value: %d\n",
         (int)grid->minimum_reflectance_grade, (int)grid->minimum_reflectance);
  printf("overall_symbol_grade:\n- grade: %d.0\n  value: %c\n", (int)grade, grade_letter[grade]);
  printf("angle_of_distortion: %.1f\n", grid->angle_of_distortion);
  printf("contrast_uniformity: %d\n", (int)grid->contrast_uniformity);
  printf("dots_per_element: %d\n", grid->dots_per_element);
  printf("elongation: %.1f\n", grid->elongation);
  printf("quiet_zone: %d\n", (int)grid->quiet_zone);
  printf("distributed_damage: %d\n", (int)grid->distributed_damage);
  printf("cell_fill: %d\n", (int)grid->cell_fill);
}

/**
 * \brief displays quality metrics
 * \param grid grid object
 * \param csv Set to 1 to show in CSV format
 * \param json Set to 1 to show in JSON format
 * \param yaml Set to 1 to show in yaml format
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 */
void show_quality_metrics(struct grid_2d * grid,
                          unsigned char csv,
                          unsigned char json,
                          unsigned char yaml,
                          float aperture,
                          int light_nm,
                          int light_angle_degrees)
{
  if (csv == 1) {
    show_quality_metrics_csv(grid, aperture, light_nm, light_angle_degrees);
    return;
  }
  else if (json == 1) {
    show_quality_metrics_json(grid, aperture, light_nm, light_angle_degrees);
    return;
  }
  else if (yaml == 1) {
    show_quality_metrics_yaml(grid, aperture, light_nm, light_angle_degrees);
    return;
  }
  show_quality_metrics_human_readable(grid, aperture, light_nm,
                                      light_angle_degrees);
}

/**
 * \brief saves a verification report
 * \param grid grid object
 * \param address_line1 First line of address to appear on verification report
 * \param address_line2 Second line of address to appear on verification report
 * \param address_line3 Third line of address to appear on verification report
 * \param phone Phone number to appear on verification report
 * \param email Email address to appear on verification report
 * \param website Web address to appear on verification report
 * \param grid_filename Image to be shown in the report
 * \param histogram_filename Reflectance histogram image to be shown in the report
 * \param report_template Filename for a TEX template
 * \param report_filename Filename to save the report as
 * \param logo_filename Filename for logo to display at top of report
 * \param decode_result decoded text
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 * \param footer Footer text on verification report
 */
void save_verification_report(struct grid_2d * grid,
                              char address_line1[],
                              char address_line2[],
                              char address_line3[],
                              char phone[],
                              char email[],
                              char website[],
                              char grid_filename[],
                              char histogram_filename[],
                              char report_template[],
                              char report_filename[],
                              char logo_filename[],
                              char decode_result[],
                              float aperture,
                              int light_nm,
                              int light_angle_degrees,
                              char footer[])
{
  FILE * fp_template, * fp_report;
  char * line = NULL;
  unsigned char grade = overall_quality_grade(grid);
  char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  fp_template = fopen(report_template, "r");
  if (fp_template == NULL) return;

  fp_report = fopen(report_filename, "w");
  if (fp_report == NULL) return;

  line = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));

  while (getline2(line, fp_template) != -1) {
    if (strstr(line, "\\newcommand{") != NULL) {
      /* address line 1 */
      if (strstr(line, "{\\addressa}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\addressa}{%s}\n", address_line1);
        continue;
      }
      /* address line 2 */
      if (strstr(line, "{\\addressb}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\addressb}{%s}\n", address_line2);
        continue;
      }
      /* address line 3 */
      if (strstr(line, "{\\addressc}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\addressc}{%s}\n", address_line3);
        continue;
      }
      /* phone */
      if (strstr(line, "{\\phone}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\phone}{%s}\n", phone);
        continue;
      }
      /* email */
      if (strstr(line, "{\\email}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\email}{%s}\n", email);
        continue;
      }
      /* website */
      if (strstr(line, "{\\website}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\website}{%s}\n", website);
        continue;
      }
      /* issue date */
      if (strstr(line, "{\\issuedate}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\issuedate}{%d-%02d-%02d}\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
        continue;
      }
      /* symbol type */
      if (strstr(line, "{\\symboltype}") != NULL) {
        if (grid->gs1_datamatrix == 1) {
          fprintf(fp_report, "\\newcommand{\\symboltype}{GS1 datamatrix}\n");
        }
        else {
          fprintf(fp_report, "\\newcommand{\\symboltype}{Datamatrix}\n");
        }
        continue;
      }
      /* matrix size */
      if (strstr(line, "{\\matrixsize}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\matrixsize}{%dx%d}\n",
                grid->dimension_x, grid->dimension_y);
        continue;
      }
      /* overall grade A-F */
      if (strstr(line, "{\\overallgrade}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\overallgrade}{%c}\n",
                grade_letter[grade]);
        continue;
      }
      /* decode result */
      if (strstr(line, "{\\decoderesult}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\decoderesult}{");
        for (int i = 0; i < strlen(decode_result); i++) {
          if (decode_result[i] == 29) {
            fprintf(fp_report, "<GS>");
          }
          else if (decode_result[i] == 30) {
            fprintf(fp_report, "<RS>");
          }
          else if (decode_result[i] == 31) {
            fprintf(fp_report, "<US>");
          }
          else if (decode_result[i] == 28) {
            fprintf(fp_report, "<FS>");
          }
          else if (decode_result[i] == 4) {
            fprintf(fp_report, "<EOT>");
          }
          else if (decode_result[i] == '_') {
            fprintf(fp_report, "\\_");
          }
          else if (decode_result[i] == '\\') {
            fprintf(fp_report, "\\textbackslash");
          }
          else if (decode_result[i] == '%') {
            fprintf(fp_report, "\\%%");
          }
          else {
            fprintf(fp_report, "%c", decode_result[i]);
          }
        }
        fprintf(fp_report, "}\n");
        continue;
      }
      /* ISO Symbol Grade 0.0 - 4.0 */
      if (strstr(line, "{\\isosymbolgrade}") != NULL) {
        if (grade > 0) {
          fprintf(fp_report, "\\newcommand{\\isosymbolgrade}{%d.0 PASS}\n",
                  (int)grade);
        }
        else {
          fprintf(fp_report, "\\newcommand{\\isosymbolgrade}{%d.0 FAIL}\n",
                  (int)grade);
        }
        continue;
      }
      /* symbol contrast */
      if (strstr(line, "{\\symbolcontrast}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\symbolcontrast}{%c (%d\\%%)}\n",
                grade_letter[(int)grid->symbol_contrast_grade],
                (int)grid->symbol_contrast);
        continue;
      }
      /* modulation */
      if (strstr(line, "{\\modulation}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\modulation}{%c (%d\\%%)}\n",
                grade_letter[(int)grid->modulation_grade],
                (int)grid->modulation);
        continue;
      }
      /* axial non-uniformity */
      if (strstr(line, "{\\axialnonuniformity}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\axialnonuniformity}{%c (%.1f\\%%)}\n",
                grade_letter[(int)grid->axial_non_uniformity_grade],
                grid->axial_non_uniformity);
        continue;
      }
      /* grid non-uniformity */
      if (strstr(line, "{\\gridnonuniformity}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\gridnonuniformity}{%c (%.1f\\%%)}\n",
                grade_letter[(int)grid->grid_non_uniformity_grade],
                grid->grid_non_uniformity);
        continue;
      }
      /* unused error correction */
      if (strstr(line, "{\\unusederrorcorrection}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\unusederrorcorrection}{%c (%d\\%%)}\n",
                grade_letter[(int)grid->unused_error_correction_grade],
                (int)grid->unused_error_correction);
        continue;
      }
      /* fixed pattern damage */
      if (strstr(line, "{\\fixedpatterndamage}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\fixedpatterndamage}{%c (%d\\%%)}\n",
                grade_letter[(int)grid->fixed_pattern_damage_grade],
                (int)grid->fixed_pattern_damage);
        continue;
      }
      /* clock track regularity */
      if (strstr(line, "{\\clocktrackregularity}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\clocktrackregularity}{%c (%d\\%%)}\n",
                grade_letter[(int)grid->clock_track_regularity_grade],
                (int)grid->clock_track_regularity);
        continue;
      }
      /* minimum reflectance */
      if (strstr(line, "{\\minreflectance}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\minreflectance}{%c (%d\\%%)}\n",
                grade_letter[(int)grid->minimum_reflectance_grade],
                (int)grid->minimum_reflectance);
        continue;
      }
      /* aperture */
      if (aperture > 0) {
        if (strstr(line, "{\\lightaperture}") != NULL) {
          fprintf(fp_report, "\\newcommand{\\lightaperture}{%.1f}\n",
                  aperture);
          continue;
        }
      }
      /* light wavelength nm */
      if (strstr(line, "{\\lightnm}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\lightnm}{%d}\n",
                light_nm);
        continue;
      }
      /* light angle degrees */
      if (strstr(line, "{\\lightangle}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\lightangle}{%d}\n",
                light_angle_degrees);
        continue;
      }
      /* angle of distortion */
      if (strstr(line, "{\\angleofdistortion}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\angleofdistortion}{%.1f}\n",
                grid->angle_of_distortion);
        continue;
      }
      /* contrast uniformity */
      if (strstr(line, "{\\contrastuniformity}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\contrastuniformity}{%d}\n",
                (int)grid->contrast_uniformity);
        continue;
      }
      /* dots per element */
      if (strstr(line, "{\\dotsperelement}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\dotsperelement}{%d}\n",
                grid->dots_per_element);
        continue;
      }
      /* elongation */
      if (strstr(line, "{\\elongation}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\elongation}{%.1f}\n",
                grid->elongation);
        continue;
      }
      /* quiet zone */
      if (strstr(line, "{\\quietzone}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\quietzone}{%d}\n",
                (int)grid->quiet_zone);
        continue;
      }
      /* distributed damage */
      if (strstr(line, "{\\distributeddamage}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\distributeddamage}{%d}\n",
                (int)grid->distributed_damage);
        continue;
      }
      /* cell fill */
      if (strstr(line, "{\\cellfill}") != NULL) {
        fprintf(fp_report, "\\newcommand{\\cellfill}{%d}\n",
                (int)grid->cell_fill);
        continue;
      }
    }
    /* logo image at top of report */
    if (strlen(logo_filename) > 0) {
      if (strstr(line, "{img/logo_square.png}") != NULL) {
        fprintf(fp_report, "\\includegraphics[height=2.5cm,clip]{%s}\n",
                logo_filename);
        continue;
      }
    }
    /* detected symbol image */
    if (strstr(line, "{img/datamatrix.png}") != NULL) {
      fprintf(fp_report, "    \\includegraphics[height=6cm]{%s}\n",
              grid_filename);
      continue;
    }
    /* reflectance histogram image */
    if (strstr(line, "{img/histogram.png}") != NULL) {
      fprintf(fp_report, "    \\includegraphics[height=6cm]{%s}\n",
              histogram_filename);
      continue;
    }
    /* footer text */
    if (strlen(footer) > 0) {
      if (strstr(line, "\\fancyfoot") != NULL) {
        fprintf(fp_report, "\\fancyfoot[C]{%s}\n", footer);
        continue;
      }
    }
    fprintf(fp_report, "%s", line);
  }

  free(line);
  fclose(fp_template);
  fclose(fp_report);
}
