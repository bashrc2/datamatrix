/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Grid fitting functions
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
float get_cell_width(struct grid_2d * grid)
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
 * \brief returns a probability that a given perimeter side described by a
 *        line is the timing border at a given frequency
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param tx start x coordinate for the perimeter side
 * \param ty start y coordinate for the perimeter side
 * \param bx end x coordinate for the perimeter side
 * \param by end y coordinate for the perimeter side
 * \param frequency the timing border frequency to be checked
 * \param sampling_radius radius of pixels to be checked at each location in the frequency
 * \param debug set to 1 if in debug mode
 * \param image_data colour image array
 * \param debug_frequency set to 1 to show the pixels being checked within the colour image
 * \return probability that a timing border exists at this frequency
 */
static int get_timing_prob_side(unsigned char mono_img[],
                                int width, int height,
                                float tx, float ty,
                                float bx, float by,
                                int frequency,
                                int sampling_radius,
                                unsigned char debug,
                                unsigned char image_data[],
                                int debug_frequency)
{
  float dx = bx - tx;
  float dy = by - ty;
  int i, x, y, n, expected, prob=0, samples=0;
  int cell_prob, previous_cell_prob=0;
  int xx, yy, min_x, min_y, max_x, max_y;

  for (i = 0; i < frequency; i++) {
    expected = (i % 2) * 255;
    x = (int)(tx + ((i+0.5f) * dx / frequency));
    y = (int)(ty + ((i+0.5f) * dy / frequency));
    cell_prob = 0;
    /* sample a few pixels around this point */
    min_x = x - sampling_radius;
    max_x = x + sampling_radius;
    min_y = y - sampling_radius;
    max_y = y + sampling_radius;
    if (min_x < 0) min_x = 0;
    if (max_x >= width) max_x = width-1;
    if (min_y < 0) min_y = 0;
    if (max_y >= height) max_y = height-1;

    /* sample pixels around (x,y) */
    for (yy = min_y; yy <= max_y; yy++) {
      n = yy*width + min_x;
      for (xx = min_x; xx <= max_x; xx++, n++) {
        if (mono_img[n] == expected) cell_prob++;
        if ((debug == 1) && (frequency == debug_frequency)) {
          image_data[n*3] = 0;
          image_data[n*3+1] = 255;
          image_data[n*3+2] = 0;
        }
        samples++;
      }
    }
    prob += cell_prob*previous_cell_prob;
    previous_cell_prob = cell_prob;
  }

  if (samples == 0) return 0;
  return (int)(prob * 5000 / (samples*frequency));
}

/**
 * \brief returns a probability in the range 0-10000 for the existence of
 *        timing border at the given frequency.
 *        Here we check three corners - an "L" shape - which the timing borders
 *        are expected to appear in
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param corner_x x coordinate of the corner to be checked (mid point of the "L" shape)
 * \param corner_y y coordinate of the corner to be checked (mid point of the "L" shape)
 * \param prev_corner_x x coordinate of the previous corner to be checked
 *                      (first point of the "L" shape)
 * \param prev_corner_y y coordinate of the previous corner to be checked
 *                      (first point of the "L" shape)
 * \param next_corner_x x coordinate of the next corner to be checked
 *                      (last point of the "L" shape)
 * \param next_corner_y y coordinate of the next corner to be checked
 *                      (last point of the "L" shape)
 * \param frequency the timing border frequency to be checked
 * \param frequency2 the second timing border frequency to be checked
 * \param sampling_radius radius of pixels to be checked at each location in the frequency
 * \param debug set to 1 if in debug mode
 * \param image_data colour image array
 * \param debug_frequency set to 1 to show the pixels being checked within the colour image
 * \return probability that the given corner is the timing border
 */
static int get_timing_prob(unsigned char mono_img[],
                           int width, int height,
                           float corner_x, float corner_y,
                           float prev_corner_x, float prev_corner_y,
                           float next_corner_x, float next_corner_y,
                           int frequency, int frequency_shortest,
                           int sampling_radius,
                           unsigned char debug,
                           unsigned char image_data[],
                           int debug_frequency)
{
  int frequency1 = frequency;
  int frequency2 = frequency;

  if (frequency_shortest < frequency) {
    /* rectangular shape */
    float dx1 = corner_x - prev_corner_x;
    float dy1 = corner_y - prev_corner_y;
    float dist1_sqr = dx1*dx1 + dy1*dy1;
    float dx2 = corner_x - next_corner_x;
    float dy2 = corner_y - next_corner_y;
    float dist2_sqr = dx2*dx2 + dy2*dy2;
    if (dist1_sqr > dist2_sqr) {
      frequency1 = frequency;
      frequency2 = frequency_shortest;
    }
    else {
      frequency1 = frequency_shortest;
      frequency2 = frequency;
    }
  }

  int prob = get_timing_prob_side(mono_img, width, height,
                                  corner_x, corner_y,
                                  prev_corner_x, prev_corner_y,
                                  frequency1, sampling_radius,
                                  debug, image_data,
                                  debug_frequency);
  prob += get_timing_prob_side(mono_img, width, height,
                               corner_x, corner_y,
                               next_corner_x, next_corner_y,
                               frequency2, sampling_radius,
                               debug, image_data,
                               debug_frequency);
  return prob;
}

/**
 * \brief detects the timing border for a square NxN datamatrix
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param minimum_grid_dimension minimum grid dimension
 * \param maximum_grid_dimension maximum grid dimension
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param threshold minimum threshold for probable timing border
 * \param side_length length of perimeter sides in the square
 * \param sampling_radius radius of pixels to be checked at each location in the frequency
 * \param debug set to 1 if in debug mode
 * \param image_data colour image array
 * \param debug_frequency set to 1 to show the pixels being checked within the colour image
 * \return the most likely timing border frequency
 */
static int detect_timing_pattern_square(unsigned char mono_img[],
                                        int width, int height,
                                        int minimum_grid_dimension,
                                        int maximum_grid_dimension,
                                        float perimeter_x0, float perimeter_y0,
                                        float perimeter_x1, float perimeter_y1,
                                        float perimeter_x2, float perimeter_y2,
                                        float perimeter_x3, float perimeter_y3,
                                        int threshold, float side_length,
                                        int sampling_radius,
                                        unsigned char debug,
                                        unsigned char image_data[],
                                        int debug_frequency)
{
  float pitch, half_pitch, centre_x, centre_y, vertex_x, vertex_y, dx, dy;
  int side, corner, index, freq, prob, max_prob=0, probable_frequency=-1;
  float timing_perimeter_x0, timing_perimeter_y0;
  float timing_perimeter_x1, timing_perimeter_y1;
  float timing_perimeter_x2, timing_perimeter_y2;
  float timing_perimeter_x3, timing_perimeter_y3;
  float x0, y0, x1, y1, x2, y2, fraction;
  int * valid_squares = get_valid_squares();

  get_centroid(perimeter_x0, perimeter_y0,
               perimeter_x1, perimeter_y1,
               perimeter_x2, perimeter_y2,
               perimeter_x3, perimeter_y3,
               &centre_x, &centre_y);

  for (index = 0; index < NO_OF_VALID_SQUARES; index++) {
    freq = valid_squares[index];
    if ((freq < minimum_grid_dimension) ||
        (freq > maximum_grid_dimension)) continue;
    pitch = side_length / freq;
    half_pitch = pitch/2;
    /* make a shrunken perimeter half the pitch smaller */
    for (side = 0; side < 4; side++) {
      switch(side) {
      case 0: {
        vertex_x = perimeter_x0;
        vertex_y = perimeter_y0;
        break;
      }
      case 1: {
        vertex_x = perimeter_x1;
        vertex_y = perimeter_y1;
        break;
      }
      case 2: {
        vertex_x = perimeter_x2;
        vertex_y = perimeter_y2;
        break;
      }
      case 3: {
        vertex_x = perimeter_x3;
        vertex_y = perimeter_y3;
        break;
      }
      }
      dx = vertex_x - centre_x;
      dy = vertex_y - centre_y;
      fraction = half_pitch / (float)sqrt(dx*dx + dy*dy);
      switch(side) {
      case 0: {
        timing_perimeter_x0 = vertex_x - (dx*fraction);
        timing_perimeter_y0 = vertex_y - (dy*fraction);
        break;
      }
      case 1: {
        timing_perimeter_x1 = vertex_x - (dx*fraction);
        timing_perimeter_y1 = vertex_y - (dy*fraction);
        break;
      }
      case 2: {
        timing_perimeter_x2 = vertex_x - (dx*fraction);
        timing_perimeter_y2 = vertex_y - (dy*fraction);
        break;
      }
      case 3: {
        timing_perimeter_x3 = vertex_x - (dx*fraction);
        timing_perimeter_y3 = vertex_y - (dy*fraction);
        break;
      }
      }
    }
    /* test each corner for a timing pattern */
    for (corner = 0; corner < 4; corner++) {
      switch(corner) {
      case 0: {
        x0 = timing_perimeter_x3;
        y0 = timing_perimeter_y3;
        x1 = timing_perimeter_x0;
        y1 = timing_perimeter_y0;
        x2 = timing_perimeter_x1;
        y2 = timing_perimeter_y1;
        break;
      }
      case 1: {
        x0 = timing_perimeter_x0;
        y0 = timing_perimeter_y0;
        x1 = timing_perimeter_x1;
        y1 = timing_perimeter_y1;
        x2 = timing_perimeter_x2;
        y2 = timing_perimeter_y2;
        break;
      }
      case 2: {
        x0 = timing_perimeter_x1;
        y0 = timing_perimeter_y1;
        x1 = timing_perimeter_x2;
        y1 = timing_perimeter_y2;
        x2 = timing_perimeter_x3;
        y2 = timing_perimeter_y3;
        break;
      }
      case 3: {
        x0 = timing_perimeter_x2;
        y0 = timing_perimeter_y2;
        x1 = timing_perimeter_x3;
        y1 = timing_perimeter_y3;
        x2 = timing_perimeter_x0;
        y2 = timing_perimeter_y0;
        break;
      }
      }
      prob = get_timing_prob(mono_img, width, height,
                             x1, y1, x0, y0, x2, y2,
                             freq, freq, sampling_radius,
                             debug, image_data,
                             debug_frequency);
      if ((prob > threshold) && (prob > max_prob)) {
        max_prob = prob;
        probable_frequency = freq;
      }
    }
  }
  return probable_frequency;
}

/**
 * \brief detects the timing border for a rectangular datamatrix
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param minimum_grid_dimension minimum grid dimension
 * \param maximum_grid_dimension maximum grid dimension
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param threshold minimum threshold for probable timing border
 * \param side_length length of perimeter sides in the square
 * \param sampling_radius radius of pixels to be checked at each location in the frequency
 * \param debug set to 1 if in debug mode
 * \param image_data colour image array
 * \param debug_frequency set to 1 to show the pixels being checked within the colour image
 * \return the most likely timing border frequency
 */
static int detect_timing_pattern_rectangular(unsigned char mono_img[],
                                             int width, int height,
                                             int minimum_grid_dimension,
                                             int maximum_grid_dimension,
                                             float perimeter_x0, float perimeter_y0,
                                             float perimeter_x1, float perimeter_y1,
                                             float perimeter_x2, float perimeter_y2,
                                             float perimeter_x3, float perimeter_y3,
                                             int threshold, float side_length,
                                             int sampling_radius,
                                             unsigned char debug,
                                             unsigned char image_data[],
                                             int debug_frequency)
{
  float pitch, half_pitch, centre_x, centre_y, vertex_x, vertex_y, dx, dy;
  int side, corner, index, freq, freq_shortest, prob, max_prob=0, probable_frequency=-1;
  float timing_perimeter_x0, timing_perimeter_y0;
  float timing_perimeter_x1, timing_perimeter_y1;
  float timing_perimeter_x2, timing_perimeter_y2;
  float timing_perimeter_x3, timing_perimeter_y3;
  float x0, y0, x1, y1, x2, y2, fraction;
  int * valid_rectangles = get_valid_rectangles();

  get_centroid(perimeter_x0, perimeter_y0,
               perimeter_x1, perimeter_y1,
               perimeter_x2, perimeter_y2,
               perimeter_x3, perimeter_y3,
               &centre_x, &centre_y);

  for (index = 0; index < NO_OF_VALID_RECTANGLES; index++) {
    /* frequency of the longest side */
    freq = valid_rectangles[index*2+1];
    if ((freq < minimum_grid_dimension) ||
        (freq > maximum_grid_dimension)) continue;
    freq_shortest = valid_rectangles[index*2];
    pitch = side_length / freq;
    half_pitch = pitch/2;
    /* make a shrunken perimeter half the pitch smaller */
    for (side = 0; side < 4; side++) {
      switch(side) {
      case 0: {
        vertex_x = perimeter_x0;
        vertex_y = perimeter_y0;
        break;
      }
      case 1: {
        vertex_x = perimeter_x1;
        vertex_y = perimeter_y1;
        break;
      }
      case 2: {
        vertex_x = perimeter_x2;
        vertex_y = perimeter_y2;
        break;
      }
      case 3: {
        vertex_x = perimeter_x3;
        vertex_y = perimeter_y3;
        break;
      }
      }
      dx = vertex_x - centre_x;
      dy = vertex_y - centre_y;
      fraction = half_pitch / (float)sqrt(dx*dx + dy*dy);
      switch(side) {
      case 0: {
        timing_perimeter_x0 = vertex_x - (dx*fraction);
        timing_perimeter_y0 = vertex_y - (dy*fraction);
        break;
      }
      case 1: {
        timing_perimeter_x1 = vertex_x - (dx*fraction);
        timing_perimeter_y1 = vertex_y - (dy*fraction);
        break;
      }
      case 2: {
        timing_perimeter_x2 = vertex_x - (dx*fraction);
        timing_perimeter_y2 = vertex_y - (dy*fraction);
        break;
      }
      case 3: {
        timing_perimeter_x3 = vertex_x - (dx*fraction);
        timing_perimeter_y3 = vertex_y - (dy*fraction);
        break;
      }
      }
    }
    /* test each corner for a timing pattern */
    for (corner = 0; corner < 4; corner++) {
      switch(corner) {
      case 0: {
        x0 = timing_perimeter_x3;
        y0 = timing_perimeter_y3;
        x1 = timing_perimeter_x0;
        y1 = timing_perimeter_y0;
        x2 = timing_perimeter_x1;
        y2 = timing_perimeter_y1;
        break;
      }
      case 1: {
        x0 = timing_perimeter_x0;
        y0 = timing_perimeter_y0;
        x1 = timing_perimeter_x1;
        y1 = timing_perimeter_y1;
        x2 = timing_perimeter_x2;
        y2 = timing_perimeter_y2;
        break;
      }
      case 2: {
        x0 = timing_perimeter_x1;
        y0 = timing_perimeter_y1;
        x1 = timing_perimeter_x2;
        y1 = timing_perimeter_y2;
        x2 = timing_perimeter_x3;
        y2 = timing_perimeter_y3;
        break;
      }
      case 3: {
        x0 = timing_perimeter_x2;
        y0 = timing_perimeter_y2;
        x1 = timing_perimeter_x3;
        y1 = timing_perimeter_y3;
        x2 = timing_perimeter_x0;
        y2 = timing_perimeter_y0;
        break;
      }
      }
      prob = get_timing_prob(mono_img, width, height,
                             x1, y1, x0, y0, x2, y2,
                             freq, freq_shortest, sampling_radius,
                             debug, image_data,
                             debug_frequency);
      if ((prob > threshold) && (prob > max_prob)) {
        max_prob = prob;
        probable_frequency = freq;
      }
    }
  }
  return probable_frequency;
}

/**
 * \brief returns the probable frequency of the timing border
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param minimum_grid_dimension minimum grid dimension
 * \param maximum_grid_dimension maximum grid dimension
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param sampling_radius radius of pixels to be checked at each location in the frequency
 * \param debug set to 1 if in debug mode
 * \param image_data colour image array
 * \param debug_frequency set to 1 to show the pixels being checked within the colour image
 * \return the most likely timing border frequency
 */
int detect_timing_pattern(unsigned char mono_img[],
                          int width, int height,
                          int minimum_grid_dimension,
                          int maximum_grid_dimension,
                          float perimeter_x0, float perimeter_y0,
                          float perimeter_x1, float perimeter_y1,
                          float perimeter_x2, float perimeter_y2,
                          float perimeter_x3, float perimeter_y3,
                          int sampling_radius,
                          unsigned char debug,
                          unsigned char image_data[],
                          int debug_frequency)
{
  int threshold = 0;
  float longest_side = get_longest_side(perimeter_x0, perimeter_y0,
                                        perimeter_x1, perimeter_y1,
                                        perimeter_x2, perimeter_y2,
                                        perimeter_x3, perimeter_y3);
  float shortest_side = get_longest_side(perimeter_x0, perimeter_y0,
                                         perimeter_x1, perimeter_y1,
                                         perimeter_x2, perimeter_y2,
                                         perimeter_x3, perimeter_y3);
  if (longest_side < 1) return -1;
  float aspect_ratio = shortest_side * 100 / longest_side;
  if ((aspect_ratio > 90) && (aspect_ratio < 110)) {
    /* square */
    return detect_timing_pattern_square(mono_img, width, height,
                                        minimum_grid_dimension,
                                        maximum_grid_dimension,
                                        perimeter_x0, perimeter_y0,
                                        perimeter_x1, perimeter_y1,
                                        perimeter_x2, perimeter_y2,
                                        perimeter_x3, perimeter_y3,
                                        threshold, longest_side,
                                        sampling_radius,
                                        debug, image_data,
                                        debug_frequency);
  }
  else {
    /* rectangle */
    return detect_timing_pattern_rectangular(mono_img, width, height,
                                             minimum_grid_dimension,
                                             maximum_grid_dimension,
                                             perimeter_x0, perimeter_y0,
                                             perimeter_x1, perimeter_y1,
                                             perimeter_x2, perimeter_y2,
                                             perimeter_x3, perimeter_y3,
                                             threshold, longest_side,
                                             sampling_radius,
                                             debug, image_data,
                                             debug_frequency);
  }
  return -1;
}

/**
 * \brief fills in any missing fixed pattern after orientation
 * \param grid grid object
 */
static void complete_fixed_pattern(struct grid_2d * grid)
{
  int grid_x, grid_y, expected;
  int damage=0, timing_border_damage=0;
  int fixed_pattern_cells=0;
  int timing_border_cells=0;

  /* solid border */
  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    if (grid->occupancy[0][grid_y] == 0) damage++;
    grid->occupancy[0][grid_y] = 1;
    fixed_pattern_cells++;
  }
  for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
    if (grid->occupancy[grid_x][grid->dimension_y-1] == 0) damage++;
    grid->occupancy[grid_x][grid->dimension_y-1] = 1;
    fixed_pattern_cells++;
  }

  /* timing border */
  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    expected = grid_y % 2;
    if (grid->occupancy[grid->dimension_x-1][grid_y] != expected) {
      damage++;
      timing_border_damage++;
    }
    grid->occupancy[grid->dimension_x-1][grid_y] = expected;
    fixed_pattern_cells++;
    timing_border_cells++;
  }
  for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
    expected = 1 - (grid_x % 2);
    if (grid->occupancy[grid_x][0] != expected) {
      damage++;
      timing_border_damage++;
    }
    grid->occupancy[grid_x][0] = expected;
    fixed_pattern_cells++;
    timing_border_cells++;
  }

  /* QUALITY METRIC: fixed pattern damage as a percentage */
  grid->fixed_pattern_damage = (unsigned char)(damage * 100 / fixed_pattern_cells);
  /* QUALITY METRIC: clock track regularity as a percentage */
  grid->clock_track_regularity =
    (unsigned char)(timing_border_damage*100/timing_border_cells);
}

/**
 * \brief square grids can have ambiguous orientation, so rotating sometimes
 *        achieves a decode
 * \param grid grid object
 */
void rotate_grid(struct grid_2d * grid)
{
  int grid_x, grid_y, grid_y2;

  if ((grid->dimension_x != grid->dimension_y)) return;

  for (grid_y = grid->dimension_y-1; grid_y >= 0; grid_y--) {
    grid_y2 = grid->dimension_y-1-grid_y;
    for (grid_x = grid->dimension_x-1; grid_x >= 0; grid_x--) {
      grid->occupancy_buffer[grid_x][grid_y] = grid->occupancy[grid_x][grid_y2];
      grid->damage_buffer[grid_y*grid->dimension_x + grid_x] =
        grid->damage[grid_y2*grid->dimension_x + grid_x];
    }
  }

  for (grid_y = grid->dimension_y-1; grid_y >= 0; grid_y--) {
    grid_y2 = grid->dimension_y-1-grid_y;
    for (grid_x = grid->dimension_x-1; grid_x >= 0; grid_x--) {
      grid->occupancy[grid_x][grid_y2] = grid->occupancy_buffer[grid_y][grid_x];
      grid->damage[grid_y2*grid->dimension_x + grid_x] =
        grid->damage_buffer[grid_x*grid->dimension_x + grid_y];
    }
  }
  grid->rotated = 1;
}

/**
 * \brief flips and/or mirrors the grid to get it into a standard orientation for decoding
 * \param grid grid object
 */
static void orient_grid(struct grid_2d * grid)
{
  int n, grid_x, grid_y, left_hits=0, right_hits=0, top_hits=0, bottom_hits=0;
  unsigned char * temp;

  /* keep a copy of the original damage pattern for use when displaying
     damage in an image */
  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    n = grid_y * grid->dimension_x;
    for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
      grid->original_damage[n+grid_x] = grid->damage[n+grid_x];
    }
  }

  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    if (grid->occupancy[0][grid_y]) left_hits++;
    if (grid->occupancy[grid->dimension_x-1][grid_y]) right_hits++;
  }
  if (right_hits > left_hits) {
    /* mirror */
    grid->mirrored = 1;
    temp = (unsigned char*)safemalloc(grid->dimension_x*sizeof(unsigned char));
    if (temp != NULL) {
      /* mirror occupancy */
      for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
        for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
          temp[grid_x] = grid->occupancy[grid_x][grid_y];
        }
        for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
          grid->occupancy[grid_x][grid_y] = temp[grid->dimension_x - 1 - grid_x];
        }
      }
      /* mirror damage */
      for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
        n = grid_y * grid->dimension_x;
        for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
          temp[grid_x] = grid->damage[n+grid_x];
        }
        for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
          grid->damage[n+grid_x] = temp[grid->dimension_x - 1 - grid_x];
        }
      }
      free(temp);
    }
  }

  for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
    if (grid->occupancy[grid_x][0]) top_hits++;
    if (grid->occupancy[grid_x][grid->dimension_y-1]) bottom_hits++;
  }
  if (top_hits > bottom_hits) {
    /* flip */
    grid->flipped = 1;
    temp = (unsigned char*)safemalloc(grid->dimension_y*sizeof(unsigned char));
    if (temp != NULL) {
      /* flip occupancy */
      for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
        for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
          temp[grid_y] = grid->occupancy[grid_x][grid_y];
        }
        for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
          grid->occupancy[grid_x][grid_y] = temp[grid->dimension_y - 1 - grid_y];
        }
      }
      /* flip damage */
      for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
        for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
          n = grid_y*grid->dimension_x + grid_x;
          temp[grid_y] = grid->damage[n];
        }
        for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
          n = grid_y*grid->dimension_x + grid_x;
          grid->damage[n] = temp[grid->dimension_y - 1 - grid_y];
        }
      }
      free(temp);
    }
  }
}

/**
 * \brief creates a grid object with various arrays used for decoding
 * \param dimension_x x dimension of the grid
 * \param dimension_y y dimension of the grid
 * \param grid grid object
 */
static void create_grid_base(int dimension_x, int dimension_y,
                             struct grid_2d * grid)
{
  /* m_NN */
  const int max_bits = 256;
  int grid_x;

  grid->dimension_x = dimension_x;
  grid->dimension_y = dimension_y;

  /* orientation of the occupancy/damage arrays */
  grid->rotated = 0;
  grid->mirrored = 0;
  grid->flipped = 0;

  /* percent of fixed pattern damage */
  grid->fixed_pattern_damage = 0;

  /* not a GS1 datamatrix */
  grid->gs1_datamatrix = 0;

  /* generate the grid cells and initialise them to zero */
  grid->occupancy = (unsigned char**)safemalloc(dimension_x*sizeof(unsigned char*));
  for (grid_x = 0; grid_x < dimension_x; grid_x++) {
    grid->occupancy[grid_x] = (unsigned char *)safemalloc(dimension_y*sizeof(unsigned char));
    memset(grid->occupancy[grid_x], 0, dimension_y * sizeof(unsigned char));
  }

  /* generate the grid buffer cells and initialise them to zero */
  grid->occupancy_buffer = (unsigned char**)safemalloc(dimension_x*sizeof(unsigned char*));
  for (grid_x = 0; grid_x < dimension_x; grid_x++) {
    grid->occupancy_buffer[grid_x] = (unsigned char *)safemalloc(dimension_y*sizeof(unsigned char));
    memset(grid->occupancy_buffer[grid_x], 0, dimension_y * sizeof(unsigned char));
  }

  /* generate the damaged cells and initialise them to zero */
  grid->damage = (unsigned char*)safemalloc(dimension_x*dimension_x*sizeof(unsigned char));
  memset(grid->damage, 0, dimension_x*dimension_y * sizeof(unsigned char));

  /* generate original damaged cells for use when drawing damage in an image */
  grid->original_damage = (unsigned char*)safemalloc(dimension_x*dimension_x*sizeof(unsigned char));
  memset(grid->original_damage, 0, dimension_x*dimension_y * sizeof(unsigned char));

  /* generate the damaged cells buffer and initialise them to zero */
  grid->damage_buffer = (unsigned char*)safemalloc(dimension_x*dimension_x*sizeof(unsigned char));
  memset(grid->damage_buffer, 0, dimension_x*dimension_y * sizeof(unsigned char));

  /* erasures */
  grid->erasures = (int*)safemalloc(MAX_GRID_DIMENSION*MAX_GRID_DIMENSION*sizeof(int));

  /* codeword array, cleared to zero */
  grid->codeword = (unsigned char*)safemalloc(MAX_CODEWORDS*sizeof(unsigned char));
  memset(grid->codeword, 0, MAX_CODEWORDS * sizeof(unsigned char));

  /* codeword pattern array, cleared to zero */
  grid->codeword_pattern = (int**)safemalloc(dimension_x*sizeof(int*));
  for (grid_x = 0; grid_x < dimension_x; grid_x++) {
    grid->codeword_pattern[grid_x] = (int *)safemalloc(dimension_y*sizeof(int));
    memset(grid->codeword_pattern[grid_x], 0, dimension_y * sizeof(int));
  }

  grid->corrected_codewords = (unsigned char*)safemalloc(MAX_CODEWORDS*sizeof(unsigned char));
  memset(grid->corrected_codewords, 0, MAX_CODEWORDS * sizeof(unsigned char));

  grid->data_bytes = (unsigned char*)safemalloc(8*sizeof(unsigned char));

  grid->m_Pp = (int*)safemalloc(max_bits*sizeof(int));

  /* index->polynomial form conversion table */
  grid->m_alpha_to = (int*)safemalloc(max_bits*sizeof(int));

  /* Polynomial->index form conversion table */
  grid->m_index_of = (int*)safemalloc(max_bits*sizeof(int));

  /* Generator polynomial g(x)  index form */
  grid->m_Gg = (int*)safemalloc(max_bits*sizeof(int));

  grid->m_taltab = (unsigned char*)safemalloc(max_bits*sizeof(unsigned char));

  grid->m_tal1tab = (unsigned char*)safemalloc(max_bits*sizeof(unsigned char));

  grid->data = (int*)safemalloc(max_bits*sizeof(int));

  grid->lambda = (int*)safemalloc(max_bits*sizeof(int));

  grid->s = (int*)safemalloc(max_bits*sizeof(int));

  grid->b = (int*)safemalloc(max_bits*sizeof(int));

  grid->t = (int*)safemalloc(max_bits*sizeof(int));

  grid->omega = (int*)safemalloc(max_bits*sizeof(int));

  grid->root = (int*)safemalloc(max_bits*sizeof(int));

  grid->reg = (int*)safemalloc(max_bits*sizeof(int));

  grid->loc = (int*)safemalloc(max_bits*sizeof(int));
}

/**
 * \brief create a grid from an occupancy pattern.
 *        This is used by unit testing
 * \param dimension_x x dimension of the grid
 * \param dimension_y y dimension of the grid
 * \param grid grid object
 * \param occupancy grid occupancy array to be inserted
 */
void create_grid_from_pattern(int dimension_x, int dimension_y,
                              struct grid_2d * grid,
                              unsigned char occupancy[])
{
  int grid_x, grid_y;

  create_grid_base(dimension_x, dimension_y, grid);

  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
      grid->occupancy[grid_x][grid_y] = occupancy[grid_y*dimension_x + grid_x];
    }
  }
  orient_grid(grid);
  complete_fixed_pattern(grid);
}

/**
 * \brief samples a number of pixels for a grid cell within a thresholded image
 *        and returns the number of white pixels in the cell
 * \param mono_img mono binary image array
 * \param width width of the image
 * \param height height of the image
 * \param x x coordinate within the image to sample around
 * \param y y coordinate within the image to sample around
 * \param sampling_radius the radius within which to sample
 * \return number of white (occupied) pixels
 */
static int cell_sample_solid(unsigned char mono_img[],
                             int image_width, int image_height,
                             int x, int y, int sampling_radius)
{
  int sample_x, sample_y, n, hits = 0;
  int min_x = x - sampling_radius;
  int min_y = y - sampling_radius;
  int max_x = x + sampling_radius;
  int max_y = y + sampling_radius;

  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x >= image_width) max_x = image_width-1;
  if (max_y >= image_height) max_y = image_height-1;

  for (sample_y = min_y; sample_y <= max_y; sample_y++) {
    n = sample_y*image_width + min_x;
    for (sample_x = min_x; sample_x <= max_x; sample_x++, n++) {
      /* sample some pixels at this location */
      if (mono_img[n] > 0) {
        hits++;
        return hits;
      }
    }
  }
  return hits;
}

/**
 * \brief samples a number of pixels in a ring pattern for a grid cell within
 *        a thresholded image and returns the number of white pixels in the
 *        cell. Ring sampling is useful because under some illumination types
 *        part surface markings may appear as "donuts" or "O" shapes.
 * \param mono_img mono binary image array
 * \param width width of the image
 * \param height height of the image
 * \param x x coordinate within the image to sample around
 * \param y y coordinate within the image to sample around
 * \param sampling_radius the radius within which to sample
 * \return number of white (occupied) pixels
 */
static int cell_sample_ring(unsigned char mono_img[],
                            int image_width, int image_height,
                            int x, int y, int sampling_radius)
{
  int sample_x, sample_y, n1, n2, hits = 0;
  int tx = x - sampling_radius;
  int ty = y - sampling_radius;
  int bx = x + sampling_radius;
  int by = y + sampling_radius;

  /* for computational efficiency we actually sample a square shape */
  if (tx < 0) tx = 0;
  if (ty < 0) ty = 0;
  if (bx >= image_width-1) bx = image_width-1;
  if (by >= image_height-1) by = image_height-1;

  /* horizontal sides of the square */
  n1 = ty*image_width + tx;
  n2 = by*image_width + tx;
  for (sample_x = tx; sample_x <= bx; sample_x++, n1++, n2++) {
    if (mono_img[n1] > 0) {
      hits++;
      return hits;
    }
    if (mono_img[n2] > 0) {
      hits++;
      return hits;
    }
  }

  /* vertical sides of the square */
  for (sample_y = ty; sample_y <= by; sample_y++) {
    n1 = sample_y*image_width + tx;
    if (mono_img[n1] > 0) {
      hits++;
      return hits;
    }
    n1 = sample_y*image_width + bx;
    if (mono_img[n1] > 0) {
      hits++;
      return hits;
    }
  }

  return hits;
}

/**
 * \brief create a new grid with the given dimensions
 * \param dimension_x x dimension of the grid
 * \param dimension_y y dimension of the grid
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param mono_img mono binary image array
 * \param width width of the image
 * \param height height of the image
 * \param sampling_radius the radius of pixels to sample for each grid cell
 * \param sampling_pattern solid or ring sampling
 * \param grid grid object
 */
void create_grid(int dimension_x, int dimension_y,
                 float perimeter_x0,
                 float perimeter_y0,
                 float perimeter_x1,
                 float perimeter_y1,
                 float perimeter_x2,
                 float perimeter_y2,
                 float perimeter_x3,
                 float perimeter_y3,
                 unsigned char mono_img[],
                 int image_width, int image_height,
                 int sampling_radius,
                 int sampling_pattern,
                 struct grid_2d * grid)
{
  int grid_x, grid_y, hits;
  int occupancy_threshold, damage_threshold;
  float horizontal_dx1, horizontal_dy1, horizontal_dx2, horizontal_dy2;
  float horizontal_x1, horizontal_y1, horizontal_x2, horizontal_y2;
  float vertical_dx1, vertical_dy1, vertical_dx2, vertical_dy2;
  float vertical_x1, vertical_y1, vertical_x2, vertical_y2;
  float xi=0, yi=0, grid_pos_x, grid_pos_y;

  grid->perimeter.x0 = perimeter_x0;
  grid->perimeter.y0 = perimeter_y0;
  grid->perimeter.x1 = perimeter_x1;
  grid->perimeter.y1 = perimeter_y1;
  grid->perimeter.x2 = perimeter_x2;
  grid->perimeter.y2 = perimeter_y2;
  grid->perimeter.x3 = perimeter_x3;
  grid->perimeter.y3 = perimeter_y3;
  grid->dimension_x = dimension_x;
  grid->dimension_y = dimension_y;

  create_grid_base(dimension_x, dimension_y, grid);

  if (sampling_radius < 1) sampling_radius = 1;
  occupancy_threshold = 0;
  damage_threshold = occupancy_threshold;

  horizontal_dx1 = grid->perimeter.x3 - grid->perimeter.x0;
  horizontal_dy1 = grid->perimeter.y3 - grid->perimeter.y0;
  horizontal_dx2 = grid->perimeter.x2 - grid->perimeter.x1;
  horizontal_dy2 = grid->perimeter.y2 - grid->perimeter.y1;

  vertical_dx1 = grid->perimeter.x1 - grid->perimeter.x0;
  vertical_dy1 = grid->perimeter.y1 - grid->perimeter.y0;
  vertical_dx2 = grid->perimeter.x2 - grid->perimeter.x3;
  vertical_dy2 = grid->perimeter.y2 - grid->perimeter.y3;

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
        if (sampling_pattern == SAMPLING_PATTERN_SOLID) {
          hits = cell_sample_solid(mono_img,
                                   image_width, image_height,
                                   (int)xi, (int)yi, sampling_radius);
        }
        else {
          hits = cell_sample_ring(mono_img,
                                  image_width, image_height,
                                  (int)xi, (int)yi, sampling_radius);
        }
        if (hits > occupancy_threshold) {
          grid->occupancy[grid_x][grid_y] = 1;
        }
        if ((hits > 0) && (hits <= damage_threshold)) {
          grid->damage[grid_y*grid->dimension_x + grid_x] = 1;
        }
      }
    }
  }
  orient_grid(grid);
  complete_fixed_pattern(grid);
}

/**
 * \brief returns the percent of occupied cells in the grid
 * \return percent of occupied cells
 */
unsigned char get_grid_occupancy_percent(struct grid_2d * grid)
{
  int grid_x, grid_y, hits=0;

  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
      if (grid->occupancy[grid_x][grid_y] == 1) hits++;
    }
  }
  return (unsigned char)(hits * 100 / (grid->dimension_x * grid->dimension_y));
}

/**
 * \brief frees memory for a grid object
 * \param grid grid object
 */
void free_grid(struct grid_2d * grid)
{
  int x;

  /* free the grid cells */
  for (x = 0; x < grid->dimension_x; x++) {
    free(grid->occupancy[x]);
    free(grid->occupancy_buffer[x]);
    free(grid->codeword_pattern[x]);
  }
  free(grid->occupancy);
  free(grid->original_damage);
  free(grid->damage);
  free(grid->damage_buffer);
  free(grid->erasures);
  free(grid->codeword);
  free(grid->codeword_pattern);
  free(grid->corrected_codewords);
  free(grid->data_bytes);
  free(grid->m_Pp);
  free(grid->m_index_of);
  free(grid->m_Gg);
  free(grid->m_taltab);
  free(grid->m_tal1tab);
  free(grid->data);
  free(grid->lambda);
  free(grid->s);
  free(grid->b);
  free(grid->t);
  free(grid->omega);
  free(grid->root);
  free(grid->reg);
  free(grid->loc);
}

/**
 * \brief shows the grid occupancy, including any detected cell damage
 * \param grid grid object
 */
void show_grid(struct grid_2d * grid)
{
  int n, x, y;

  printf("\n");
  for (y = 0; y < grid->dimension_y; y++) {
    n = y*grid->dimension_x;
    for (x = 0; x < grid->dimension_x; x++) {
      if (grid->damage[n+x] > 0) {
        printf("x");
      }
      else {
        if (grid->occupancy[x][y] == 0) {
          printf(" ");
        }
        else {
          printf("o");
        }
      }
    }
    printf("\n");
  }
  printf("\n");
}

/**
 * \brief shows the detected grid as an image
 * \param grid grid object
 * \param image_data image array to be shown
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bitsperpixel Number of bits per pixel
 * \param sampling_radius the radius of pixels to sample for each grid cell
 * \param sampling_pattern solid or ring sampling
 */
void show_grid_image(struct grid_2d * grid,
                     unsigned char image_data[],
                     int image_width, int image_height,
                     int image_bitsperpixel,
                     int sampling_radius, int sampling_pattern)
{
  int image_bytesperpixel = image_bitsperpixel/8;
  const int cross_radius = sampling_radius;
  int grid_x, grid_y, r, g, b, n;
  float grid_pos_x, grid_pos_y, xi=0, yi=0;
  float cell_width = get_cell_width(grid);
  int cell_radius = (int)(cell_width/3);

  /* show the vertical part of the "L" shape, L1 */
  if (grid->mirrored == 0) {
    draw_line(image_data, image_width, image_height,
              image_bitsperpixel,
              (int)grid->perimeter.x0, (int)grid->perimeter.y0,
              (int)grid->perimeter.x3, (int)grid->perimeter.y3,
              1,
              0, 255, 0);
  }
  else {
    draw_line(image_data, image_width, image_height,
              image_bitsperpixel,
              (int)grid->perimeter.x1, (int)grid->perimeter.y1,
              (int)grid->perimeter.x2, (int)grid->perimeter.y2,
              1,
              0, 255, 0);
  }

  /* show the horizontal part of the "L" shape, L2 */
  if (grid->flipped == 0) {
    draw_line(image_data, image_width, image_height,
              image_bitsperpixel,
              (int)grid->perimeter.x3, (int)grid->perimeter.y3,
              (int)grid->perimeter.x2, (int)grid->perimeter.y2,
              1,
              0, 255, 0);
  }
  else {
    draw_line(image_data, image_width, image_height,
              image_bitsperpixel,
              (int)grid->perimeter.x0, (int)grid->perimeter.y0,
              (int)grid->perimeter.x1, (int)grid->perimeter.y1,
              1,
              0, 255, 0);
  }

  float horizontal_dx1 = grid->perimeter.x3 - grid->perimeter.x0;
  float horizontal_dy1 = grid->perimeter.y3 - grid->perimeter.y0;
  float horizontal_dx2 = grid->perimeter.x2 - grid->perimeter.x1;
  float horizontal_dy2 = grid->perimeter.y2 - grid->perimeter.y1;

  float vertical_dx1 = grid->perimeter.x1 - grid->perimeter.x0;
  float vertical_dy1 = grid->perimeter.y1 - grid->perimeter.y0;
  float vertical_dx2 = grid->perimeter.x2 - grid->perimeter.x3;
  float vertical_dy2 = grid->perimeter.y2 - grid->perimeter.y3;

  for (grid_y = -1; grid_y <= grid->dimension_y; grid_y++) {
    /* horizontal line */
    grid_pos_y = grid_y + 0.5f;
    float horizontal_x1 =
      grid->perimeter.x0 + (horizontal_dx1 * grid_pos_y / grid->dimension_y);
    float horizontal_y1 =
      grid->perimeter.y0 + (horizontal_dy1 * grid_pos_y / grid->dimension_y);
    float horizontal_x2 =
      grid->perimeter.x1 + (horizontal_dx2 * grid_pos_y / grid->dimension_y);
    float horizontal_y2 =
      grid->perimeter.y1 + (horizontal_dy2 * grid_pos_y / grid->dimension_y);

    for (grid_x = -1; grid_x <= grid->dimension_x; grid_x++) {
      /* vertical line */
      grid_pos_x = grid_x + 0.5f;
      float vertical_x1 =
        grid->perimeter.x0 + (vertical_dx1 * grid_pos_x / grid->dimension_x);
      float vertical_y1 =
        grid->perimeter.y0 + (vertical_dy1 * grid_pos_x / grid->dimension_x);
      float vertical_x2 =
        grid->perimeter.x3 + (vertical_dx2 * grid_pos_x / grid->dimension_x);
      float vertical_y2 =
        grid->perimeter.y3 + (vertical_dy2 * grid_pos_x / grid->dimension_x);
      intersection(horizontal_x1, horizontal_y1,
                   horizontal_x2, horizontal_y2,
                   vertical_x1, vertical_y1,
                   vertical_x2, vertical_y2,
                   &xi, &yi);
      if (xi != PARALLEL_LINES) {
        /* dot */
        if (((int)yi >= 0) && ((int)yi < image_height) &&
            ((int)xi >= 0) && ((int)xi < image_width)) {
          n = (((int)yi * image_width) + (int)xi) * image_bytesperpixel;
          if ((grid_x > -1) && (grid_x < grid->dimension_x) &&
              (grid_y > -1) && (grid_y < grid->dimension_y)) {
            if (grid->original_damage[grid_y*grid->dimension_x + grid_x] == 0) {
              /* valid cell */
              r = (int)image_data[n+2] - 30;
              g = (int)image_data[n+1] + 30;
              b = (int)image_data[n] - 30;
            }
            else {
              /* damaged cell */
              r = (int)image_data[n+1] + 30;
              g = (int)image_data[n+2] - 30;
              b = (int)image_data[n] - 30;
            }
          }
          else {
            /* quiet zone */
            r = (int)image_data[n+1] - 30;
            g = (int)image_data[n+2] - 30;
            b = (int)image_data[n] + 30;
          }
          if (r < 0) r = 0;
          if (r > 255) r = 255;
          if (g < 0) g = 0;
          if (g > 255) g = 255;
          if (b < 0) b = 0;
          if (b > 255) b = 255;
          draw_dot(image_data, image_width, image_height,
                   image_bitsperpixel,
                   (int)xi, (int)yi, cell_radius, r, g, b);
        }

        if ((cross_radius > 0) &&
            ((grid_x > -1) && (grid_x < grid->dimension_x) &&
             (grid_y > -1) && (grid_y < grid->dimension_y))) {
          if (sampling_pattern == SAMPLING_PATTERN_SOLID) {
            /* solid sampling */
            draw_line(image_data, image_width, image_height,
                      image_bitsperpixel,
                      (int)xi-cross_radius, (int)yi,
                      (int)xi+cross_radius, (int)yi,
                      1, 0, 255, 0);
            draw_line(image_data, image_width, image_height,
                      image_bitsperpixel,
                      (int)xi, (int)yi-cross_radius,
                      (int)xi, (int)yi+cross_radius,
                      1, 0, 255, 0);
          }
          else {
            /* ring sampling */
            draw_line(image_data, image_width, image_height,
                      image_bitsperpixel,
                      (int)xi-cross_radius, (int)yi-cross_radius,
                      (int)xi+cross_radius, (int)yi-cross_radius,
                      1, 0, 255, 0);
            draw_line(image_data, image_width, image_height,
                      image_bitsperpixel,
                      (int)xi-cross_radius, (int)yi+cross_radius,
                      (int)xi+cross_radius, (int)yi+cross_radius,
                      1, 0, 255, 0);
            draw_line(image_data, image_width, image_height,
                      image_bitsperpixel,
                      (int)xi-cross_radius, (int)yi-cross_radius,
                      (int)xi-cross_radius, (int)yi+cross_radius,
                      1, 0, 255, 0);
            draw_line(image_data, image_width, image_height,
                      image_bitsperpixel,
                      (int)xi+cross_radius, (int)yi-cross_radius,
                      (int)xi+cross_radius, (int)yi+cross_radius,
                      1, 0, 255, 0);
          }
        }
      }
    }
  }
}
