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
  int xx, yy;

  for (i = 0; i < frequency; i++) {
    expected = (i % 2) * 255;
    x = (int)(tx + ((i+0.5f) * dx / frequency));
    y = (int)(ty + ((i+0.5f) * dy / frequency));
    cell_prob = 0;
    /* sample a few pixels around this point */
    for (yy = y - sampling_radius; yy <= y + sampling_radius; yy++) {
      if ((yy < 0) || (yy >= height)) continue;
      n = yy*width;
      for (xx = x - sampling_radius; xx <= x + sampling_radius; xx++) {
        if ((xx < 0) || (xx >= width)) continue;
        if (mono_img[n+xx] == expected) cell_prob++;
        if ((debug == 1) && (frequency == debug_frequency)) {
          image_data[(n+xx)*3] = 0;
          image_data[(n+xx)*3+1] = 255;
          image_data[(n+xx)*3+2] = 0;
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

/* returns a probability in the range 0-10000 for the existence of timing border at the given frequency */
static int get_timing_prob(unsigned char mono_img[],
                           int width, int height,
                           float corner_x, float corner_y,
                           float prev_corner_x, float prev_corner_y,
                           float next_corner_x, float next_corner_y,
                           int frequency, int sampling_radius,
                           unsigned char debug,
                           unsigned char image_data[],
                           int debug_frequency)
{
  int prob = get_timing_prob_side(mono_img, width, height,
                                  corner_x, corner_y,
                                  prev_corner_x, prev_corner_y,
                                  frequency, sampling_radius,
                                  debug, image_data,
                                  debug_frequency);
  prob += get_timing_prob_side(mono_img, width, height,
                               corner_x, corner_y,
                               next_corner_x, next_corner_y,
                               frequency, sampling_radius,
                               debug, image_data,
                               debug_frequency);
  return prob;
}

static int detect_timing_pattern_square(unsigned char mono_img[],
                                        int width, int height,
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
  int no_of_valid_squares = 24;
  int IEC16022_valid_squares[] = {
    10,  12,  14,  16,  18,  20,  22,  24,  26,  32, 36,  40,  44,  48,
    52,  64,  72,  80,  88,  96, 104, 120, 132, 144
  };

  get_centroid(perimeter_x0, perimeter_y0,
               perimeter_x1, perimeter_y1,
               perimeter_x2, perimeter_y2,
               perimeter_x3, perimeter_y3,
               &centre_x, &centre_y);

  for (index = 0; index < no_of_valid_squares; index++) {
    freq = IEC16022_valid_squares[index];
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
                             freq, sampling_radius,
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

/* returns the probable frequency of the timing border */
int detect_timing_pattern(unsigned char mono_img[],
                          int width, int height,
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
  if ((aspect_ratio > 80) && (aspect_ratio < 120)) {
    /* square */
    return detect_timing_pattern_square(mono_img, width, height,
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
    /* TODO rectangle */
    /*
      int no_of_valid_rectangles = 6;
      int IEC16022_valid_rectangles[] = {
      8, 18,
      8, 32,
      12, 26,
      12, 36,
      16, 36,
      16, 48
      };
    */
  }
  return -1;
}

/* fills in any missing fixed pattern after orientation */
static void complete_fixed_pattern(struct grid_2d * grid)
{
  int grid_x, grid_y;

  /* solid border */
  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    grid->occupancy[0][grid_y] = 1;
  }
  for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
    grid->occupancy[grid_x][grid->dimension_y-1] = 1;
  }

  /* timing border */
  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    grid->occupancy[grid->dimension_x-1][grid_y] = grid_y % 2;
  }
  for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
    grid->occupancy[grid_x][0] = 1 - (grid_x % 2);
  }
}

/* flips and/or mirrors the grid to get it into a standard orientation for decoding */
static void orient_grid(struct grid_2d * grid)
{
  int n, grid_x, grid_y, left_hits=0, right_hits=0, top_hits=0, bottom_hits=0;
  unsigned char * temp;

  for (grid_y = 0; grid_y < grid->dimension_y; grid_y++) {
    if (grid->occupancy[0][grid_y]) left_hits++;
    if (grid->occupancy[grid->dimension_x-1][grid_y]) right_hits++;
  }
  if (right_hits > left_hits) {
    /* mirror */
    temp = (unsigned char*)malloc(grid->dimension_x*sizeof(unsigned char));
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
    temp = (unsigned char*)malloc(grid->dimension_y*sizeof(unsigned char));
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

static void create_grid_base(int dimension_x, int dimension_y,
                             struct grid_2d * grid)
{
  /* m_NN */
  const int max_bits = 256;
  int grid_x;

  grid->dimension_x = dimension_x;
  grid->dimension_y = dimension_y;

  /* generate the grid cells and initialise them to zero */
  grid->occupancy = (unsigned char**)malloc(dimension_x*sizeof(unsigned char*));
  assert(grid->occupancy != NULL);
  for (grid_x = 0; grid_x < dimension_x; grid_x++) {
    grid->occupancy[grid_x] = (unsigned char *)malloc(dimension_y*sizeof(unsigned char));
    assert(grid->occupancy[grid_x] != NULL);
    memset(grid->occupancy[grid_x], 0, dimension_y * sizeof(unsigned char));
  }

  /* generate the damaged cells and initialise them to zero */
  grid->damage = (unsigned char*)malloc(dimension_x*dimension_x*sizeof(unsigned char));
  assert(grid->damage != NULL);
  memset(grid->damage, 0, dimension_x*dimension_y * sizeof(unsigned char));

  /* erasures */
  grid->erasures = (int*)malloc(MAX_GRID_DIMENSION*MAX_GRID_DIMENSION*sizeof(int));
  assert(grid->erasures != NULL);

  /* codeword array, cleared to zero */
  grid->codeword = (unsigned char*)malloc(MAX_CODEWORDS*sizeof(unsigned char));
  assert(grid->codeword != NULL);
  memset(grid->codeword, 0, MAX_CODEWORDS * sizeof(unsigned char));

  /* codeword pattern array, cleared to zero */
  grid->codeword_pattern = (int**)malloc(dimension_x*sizeof(int*));
  assert(grid->codeword_pattern != NULL);
  for (grid_x = 0; grid_x < dimension_x; grid_x++) {
    grid->codeword_pattern[grid_x] = (int *)malloc(dimension_y*sizeof(int));
    assert(grid->codeword_pattern[grid_x] != NULL);
    memset(grid->codeword_pattern[grid_x], 0, dimension_y * sizeof(int));
  }

  grid->corrected_codewords = (unsigned char*)malloc(MAX_CODEWORDS*sizeof(unsigned char));
  assert(grid->corrected_codewords != NULL);
  memset(grid->corrected_codewords, 0, MAX_CODEWORDS * sizeof(unsigned char));

  grid->data_bytes = (unsigned char*)malloc(8*sizeof(unsigned char));
  assert(grid->data_bytes != NULL);

  grid->m_Pp = (int*)malloc(max_bits*sizeof(int));
  assert(grid->m_Pp != NULL);

  /* index->polynomial form conversion table */
  grid->m_alpha_to = (int*)malloc(max_bits*sizeof(int));
  assert(grid->m_alpha_to != NULL);

  /* Polynomial->index form conversion table */
  grid->m_index_of = (int*)malloc(max_bits*sizeof(int));
  assert(grid->m_index_of != NULL);

  /* Generator polynomial g(x)  index form */
  grid->m_Gg = (int*)malloc(max_bits*sizeof(int));
  assert(grid->m_Gg != NULL);

  grid->m_taltab = (unsigned char*)malloc(max_bits*sizeof(unsigned char));
  assert(grid->m_taltab != NULL);

  grid->m_tal1tab = (unsigned char*)malloc(max_bits*sizeof(unsigned char));
  assert(grid->m_tal1tab != NULL);

  grid->data = (int*)malloc(max_bits*sizeof(int));
  assert(grid->data != NULL);

  grid->lambda = (int*)malloc(max_bits*sizeof(int));
  assert(grid->lambda != NULL);

  grid->s = (int*)malloc(max_bits*sizeof(int));
  assert(grid->s != NULL);

  grid->b = (int*)malloc(max_bits*sizeof(int));
  assert(grid->b != NULL);

  grid->t = (int*)malloc(max_bits*sizeof(int));
  assert(grid->t != NULL);

  grid->omega = (int*)malloc(max_bits*sizeof(int));
  assert(grid->omega != NULL);

  grid->root = (int*)malloc(max_bits*sizeof(int));
  assert(grid->root != NULL);

  grid->reg = (int*)malloc(max_bits*sizeof(int));
  assert(grid->reg != NULL);

  grid->loc = (int*)malloc(max_bits*sizeof(int));
  assert(grid->loc != NULL);
}

/* create a grid from an occupancy pattern */
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

/* samples a number of pixels for a grid cell within a thresholded image
   and returns the number of white pixels in the cell */
static int cell_sample_solid(unsigned char mono_img[],
                             int image_width, int image_height,
                             int x, int y, int sampling_radius)
{
  int sample_x, sample_y, n, hits = 0;

  for (sample_x = x - sampling_radius;
       sample_x <= x + sampling_radius; sample_x++) {
    if ((sample_x < 0) || (sample_x >= image_width)) continue;
    for (sample_y = y - sampling_radius;
         sample_y <= y + sampling_radius; sample_y++) {
      if ((sample_y < 0) || (sample_y >= image_height)) continue;
      /* sample some pixels at this location */
      n = sample_y*image_width + sample_x;
      if (mono_img[n] > 0) hits++;
    }
  }
  return hits;
}

/* samples a number of pixels for a grid cell within a thresholded image
   and returns the number of white pixels in the cell */
static int cell_sample_ring(unsigned char mono_img[],
                            int image_width, int image_height,
                            int x, int y, int sampling_radius)
{
  int sample_x, sample_y, n, hits = 0;
  int tx = x - sampling_radius;
  int ty = y - sampling_radius;
  int bx = x + sampling_radius;
  int by = y + sampling_radius;

  if (tx < 0) tx = 0;
  if (ty < 0) ty = 0;
  if (bx >= image_width-1) bx = image_width-1;
  if (by >= image_height-1) by = image_height-1;

  for (sample_x = tx; sample_x <= bx; sample_x++) {
    n = ty*image_width + sample_x;
    if (mono_img[n] > 0) hits++;
    n = by*image_width + sample_x;
    if (mono_img[n] > 0) hits++;
  }
  for (sample_y = ty; sample_y <= by; sample_y++) {
    n = sample_y*image_width + tx;
    if (mono_img[n] > 0) hits++;
    n = sample_y*image_width + bx;
    if (mono_img[n] > 0) hits++;
  }

  return hits;
}

/* create a new grid with the given dimensions */
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
  int grid_x, grid_y, hits, samples;
  int occupancy_threshold, damage_threshold;
  float horizontal_dx1, horizontal_dy1, horizontal_dx2, horizontal_dy2, horizontal_x1, horizontal_y1, horizontal_x2, horizontal_y2;
  float vertical_dx1, vertical_dy1, vertical_dx2, vertical_dy2, vertical_x1, vertical_y1, vertical_x2, vertical_y2;
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
  if (sampling_pattern == SAMPLING_PATTERN_SOLID) {
    /* solid cell sampling */
    samples = (sampling_radius*2 + 1) * (sampling_radius*2 + 1);
  }
  else {
    /* ring cell sampling */
    samples = (sampling_radius*2 + 1) * 4;
  }
  occupancy_threshold = samples/4;
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

void free_grid(struct grid_2d * grid)
{
  int x;

  /* free the grid cells */
  for (x = 0; x < grid->dimension_x; x++) {
    free(grid->occupancy[x]);
    free(grid->codeword_pattern[x]);
  }
  free(grid->occupancy);
  free(grid->damage);
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

void show_grid_image(struct grid_2d * grid,
                     unsigned char image_data[],
                     int image_width, int image_height,
                     int image_bitsperpixel,
                     int sampling_radius, int sampling_pattern)
{
  const int cross_radius = sampling_radius;
  int grid_x, grid_y;
  float grid_pos_x, grid_pos_y, xi=0, yi=0;

  draw_line(image_data, image_width, image_height,
            image_bitsperpixel,
            (int)grid->perimeter.x0, (int)grid->perimeter.y0,
            (int)grid->perimeter.x3, (int)grid->perimeter.y3,
            1,
            0, 255, 0);
  draw_line(image_data, image_width, image_height,
            image_bitsperpixel,
            (int)grid->perimeter.x1, (int)grid->perimeter.y1,
            (int)grid->perimeter.x2, (int)grid->perimeter.y2,
            1,
            255, 0, 0);

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
    float horizontal_x1 = grid->perimeter.x0 + (horizontal_dx1 * grid_pos_y / grid->dimension_y);
    float horizontal_y1 = grid->perimeter.y0 + (horizontal_dy1 * grid_pos_y / grid->dimension_y);
    float horizontal_x2 = grid->perimeter.x1 + (horizontal_dx2 * grid_pos_y / grid->dimension_y);
    float horizontal_y2 = grid->perimeter.y1 + (horizontal_dy2 * grid_pos_y / grid->dimension_y);

    for (grid_x = 0; grid_x < grid->dimension_x; grid_x++) {
      /* vertical line */
      grid_pos_x = grid_x + 0.5f;
      float vertical_x1 = grid->perimeter.x0 + (vertical_dx1 * grid_pos_x / grid->dimension_x);
      float vertical_y1 = grid->perimeter.y0 + (vertical_dy1 * grid_pos_x / grid->dimension_x);
      float vertical_x2 = grid->perimeter.x3 + (vertical_dx2 * grid_pos_x / grid->dimension_x);
      float vertical_y2 = grid->perimeter.y3 + (vertical_dy2 * grid_pos_x / grid->dimension_x);
      intersection(horizontal_x1, horizontal_y1,
                   horizontal_x2, horizontal_y2,
                   vertical_x1, vertical_y1,
                   vertical_x2, vertical_y2,
                   &xi, &yi);
      if (xi != PARALLEL_LINES) {
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
