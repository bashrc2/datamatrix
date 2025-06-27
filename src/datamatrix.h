/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  datamatrix
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


#ifndef VISIONUTILS_HEADERS_H
#define VISIONUTILS_HEADERS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <omp.h>
#include <float.h>

#define GPR_MISSING_VALUE         9999
#define PARALLEL_LINES            -2
#define UNKNOWN_DISTANCE          999999
#define NO_LINE_FIT               999998
#define PI                        3.1415927
#define FREQ_DIMENSION            256
#define MAX_DECODE_LENGTH         1024
/* max perimeter side length in pixels */
#define MAX_PERIMETER_SIDE_LENGTH 4096
#define MAX_GRID_DIMENSION        144
#define MAX_CODEWORDS             (MAX_GRID_DIMENSION*MAX_GRID_DIMENSION)

/* patterns used when sampling grid cell pixels */
#define SAMPLING_PATTERN_SOLID    0
#define SAMPLING_PATTERN_RING     1

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif


struct key_value_pair_int
{
  int Key;
  int Value;
};

struct perimeter_points {
  float x0, y0;
  float x1, y1;
  float x2, y2;
  float x3, y3;
};

struct grid_2d {
  struct perimeter_points perimeter;
  unsigned char ** occupancy;
  unsigned char * damage;
  int ** codeword_pattern;
  unsigned char * codeword;
  unsigned char * corrected_codewords;
  int dimension_x, dimension_y;
  int unused_error_correction;
  int no_of_errors;
  int no_of_erasures;
  int * erasures;
  unsigned char fixed_pattern_damage;
  float angle_of_distortion;

  unsigned char* data_bytes;

  int * m_Pp;
  int * m_alpha_to;
  int * m_index_of;
  int * m_Gg;
  unsigned char * m_taltab;
  unsigned char * m_tal1tab;
  int * data;
  int * lambda;
  int * s;
  int * b;
  int * t;
  int * omega;
  int * root;
  int * reg;
  int * loc;
};

struct line_segments {
  int image_border;
  int ignore_periphery;
  int max_members;
  int * members;
  int * no_of_members;
  int max_segments;
  int minimum_segment_length;
  int no_of_segments;
  unsigned char * joins;
  int * joined_length;
  int * joins_sorted;
  unsigned char * selected;
  int * perimeter_left;
  int * perimeter_right;
  int * perimeter_top;
  int * perimeter_bottom;
  int * orientation_histogram;
  int ** orientation_histogram_edges;
  int ** side_edges;
  int * side_edges_count;
  int edge_centre_x;
  int edge_centre_y;
  int edge_centre_hits;
  int * linefit;
  int * linefit2;
};


/* histogram.c */

int histogram_mean_reflectance(int * histogram);

void region_histogram(
                      unsigned char * img, int width, int height,
                      int tx, int ty, int bx, int by,
                      int bitsperpixel,
                      int * histogram);

/* threshold.c */

void adaptive_threshold(
                        unsigned char * img, int width, int height,
                        int bitsperpixel,
                        int radius,
                        unsigned char * thresholded);

int meanlight_threshold(unsigned char * img, int width, int height,
                        int bitsperpixel, int threshold,
                        unsigned char * thresholded);

int proximal_threshold(unsigned char * img, int width, int height,
                       int r, int g, int b, int radius,
                       unsigned char * result);
int proximal_erase(unsigned char * img, int width, int height,
                   int radius, int min_coverage_percent);
int proximal_fill(unsigned char * img, int width, int height,
                  unsigned char * result,
                  int max_width, int max_height,
                  int max_variance,
                  int * polygon_id,
                  int * polygon_vertices,
                  int * polygons,
                  int max_total_polygon_points);
int show_polygons(unsigned char * img, int width, int height,
                  int no_of_polygons,
                  int * polygon_id,
                  int * polygon_vertices,
                  int * polygons);
int point_in_polygon(int x, int y, int * points, int no_of_points);
int save_polygons_json(int no_of_polygons,
                       int * polygon_id,
                       int * polygon_vertices,
                       int * polygons,
                       char * filename);

/* convert.c */

void bitwise_to_mono(unsigned char * bitwise, int width, int height,
                     unsigned char * img);

void bitwise_to_colour(unsigned char * bitwise, int width, int height,
                       unsigned char * img, int bytesperpixel);

void mono_to_colour(unsigned char * img, int width, int height,
                    int bitsperpixel,
                    unsigned char * colour);

void colour_to_mono(unsigned char * img, int width, int height,
                    int bitsperpixel,
                    unsigned char * mono);

void mono_to_bitwise(unsigned char * img, int width, int height,
                     unsigned char * bitwise);

int save_bitwise(char * filename,
                 unsigned char * bitwise, int width, int height);

int load_bitwise(char * filename,
                 unsigned char * bitwise, int width, int height);

/* erosiondilation.c */

void erosion(
             unsigned char * img,
             int width, int height,
             int itterations);

void dilation(
              unsigned char * img,
              int width, int height,
              int itterations);

void dilate(unsigned char * img, int width, int height,
            unsigned char * buffer,
            int itterations,
            unsigned char * result);

void erode(unsigned char * img, int width, int height,
           unsigned char * buffer,
           int itterations,
           unsigned char * result);


/* dark and light thresholds */

void darklight(unsigned char * img,
               int width, int height,
               int samplingStepSize,
               int sampling_radius_percent,
               unsigned char * dark, unsigned char * light);

/* lines.c */

void remove_lines(unsigned char * img,
                  int width, int height,
                  int tx1, int ty1, int bx1, int by1,
                  int tx2, int ty2, int bx2, int by2);

/* gradient.h */

void correct_background_gradient(unsigned char * img,
                                 int width, int height,
                                 int x1, int y1,
                                 int r1, int g1, int b1,
                                 int x2, int y2,
                                 int r2, int g2, int b2);

/* edges.c */

void detect_edges(
                  unsigned char * img,
                  int width, int height,
                  float threshold, float radius);

void get_line_segments(unsigned char * edgesImage, int width, int height,
                       struct line_segments * segments,
                       int min_segment_length);
void join_line_segments(struct line_segments * segments,
                        int join_radius);

void segment_edges_within_roi(struct line_segments * segments,
                              int width, int height,
                              int roi_radius_percent);

int get_segment_aspect_ratio(struct line_segments * segments,
                             int index);

void get_peripheral_edges(struct line_segments * segments,
                          int index, int width, int height);

float get_segments_orientation(struct line_segments * segments,
                               int width, int height,
                               int quantization_degrees);

unsigned char fit_perimeter_to_sides(struct line_segments * segments,
                                     int width, int height,
                                     float *perimeter_x0, float *perimeter_y0,
                                     float *perimeter_x1, float *perimeter_y1,
                                     float *perimeter_x2, float *perimeter_y2,
                                     float *perimeter_x3, float *perimeter_y3);

int get_shape_aspect_ratio(float perimeter_x0, float perimeter_y0,
                           float perimeter_x1, float perimeter_y1,
                           float perimeter_x2, float perimeter_y2,
                           float perimeter_x3, float perimeter_y3);

void free_line_segments(struct line_segments * segments);

void show_line_segments(struct line_segments * segments,
                        unsigned char * result, int width, int height,
                        int result_bitsperpixel);
void show_square_line_segments(struct line_segments * segments,
                               unsigned char * result, int width, int height,
                               int result_bitsperpixel);
void show_peripheral_edges(struct line_segments * segments,
                           unsigned char * result, int width, int height,
                           int result_bitsperpixel);
void show_perimeter(struct line_segments * segments,
                    unsigned char * result, int width, int height,
                    int result_bitsperpixel);
void show_shape_perimeter(struct line_segments * segments,
                          unsigned char result[], int width, int height,
                          int result_bitsperpixel,
                          float perimeter_x0, float perimeter_y0,
                          float perimeter_x1, float perimeter_y1,
                          float perimeter_x2, float perimeter_y2,
                          float perimeter_x3, float perimeter_y3);

/* filters.c */

void filter_image(unsigned char * img,
                  int img_width, int img_height,
                  int bitsperpixel,
                  float * filter,
                  int filter_width,
                  unsigned char * output_img,
                  int output_img_width,
                  int output_img_height,
                  int output_bitsperpixel);

/* random.c */

unsigned int rand_num(unsigned int * seed);

/* som.c */

struct gpr_som_elem {
  float * vect;
};
typedef struct gpr_som_elem gpr_som_element;


struct gpr_som_struct {
  int dimension;
  int no_of_sensors;
  struct gpr_som_elem ** weight;
  float * state;
  float * sensor_scale;
};
typedef struct gpr_som_struct gpr_som;

struct gpr_som_stack_struct {
  int no_of_layers;
  unsigned char ** img;
  int * img_width;
  int * img_height;
  int * patch_width;
  gpr_som * som;
  int inhibit_radius;
  int excite_radius;
  float learning_rate;
  unsigned int random_seed;
  unsigned char initialised;
};
typedef struct gpr_som_stack_struct gpr_som_stack;

void gpr_som_init(int dimension, int no_of_sensors, gpr_som * som);
void gpr_som_init_image_filters(int filters_dimension,
                                int patch_width,
                                gpr_som * som,
                                float max_pixel_value,
                                int patch_dimensions,
                                unsigned int * random_seed);
void gpr_som_free(gpr_som * som);
float * gpr_som_get_filter(gpr_som * som, int index);
int gpr_som_init_sensor(gpr_som * som,
                        int sensor_index,
                        float min_value, float max_value,
                        unsigned int * random_seed);
void gpr_som_init_sensors(gpr_som * som,
                          float min_value, float max_value,
                          unsigned int * random_seed);
int gpr_som_init_sensor_from_data(gpr_som * som,
                                  int sensor_index,
                                  int training_data_field_index,
                                  float * training_data,
                                  int training_data_fields_per_example,
                                  int no_of_training_examples,
                                  unsigned int * random_seed);
void gpr_som_init_filters_from_image(gpr_som * som,
                                     unsigned char * img,
                                     int img_width, int img_height,
                                     int bitsperpixel,
                                     int patch_dimensions,
                                     unsigned int * random_seed);
void gpr_som_learn_from_data(gpr_som * som,
                             int * data_field_index,
                             float * training_data,
                             int fields_per_sample,
                             int no_of_samples,
                             int learning_itterations,
                             int inhibit_radius, int excite_radius,
                             float learning_rate,
                             unsigned int * random_seed,
                             int show_progress);
void gpr_som_outputs_from_data(gpr_som * som,
                               int * data_field_index,
                               float * data,
                               int fields_per_sample,
                               int no_of_samples,
                               float * result);
void gpr_som_learn(gpr_som * som,
                   float * sensors,
                   int inhibit_radius,
                   int excite_radius,
                   float learning_rate);
void gpr_som_learn_from_image(gpr_som * som,
                              unsigned char * img,
                              int img_width, int img_height,
                              int bitsperpixel,
                              int patch_dimensions,
                              int tx, int ty,
                              int inhibit_radius,
                              int excite_radius,
                              float learning_rate);
void gpr_som_learn_filters_from_image(int no_of_samples,
                                      unsigned char * img,
                                      int img_width, int img_height,
                                      int bitsperpixel,
                                      int patch_dimensions,
                                      gpr_som * som,
                                      unsigned int * random_seed,
                                      int inhibit_radius,
                                      int excite_radius,
                                      float learning_rate);
int gpr_som_update(float * sensors,
                   gpr_som * som,
                   float * x, float * y);
int gpr_som_update_from_image(unsigned char * img,
                              int img_width, int img_height,
                              int bitsperpixel,
                              int tx, int ty,
                              gpr_som * som,
                              int patch_dimensions,
                              float * x, float * y);
int gpr_som_run(float * sensors,
                gpr_som * som,
                float * x, float * y);
int gpr_som_run_from_image(unsigned char * img,
                           int img_width, int img_height,
                           int bitsperpixel,
                           int tx, int ty,
                           gpr_som * som,
                           int patch_dimensions,
                           float * x, float * y);
void gpr_som_save(gpr_som * som,
                  FILE * fp);
void gpr_som_load(gpr_som * som,
                  FILE * fp);
void gpr_som_show_state(gpr_som * som,
                        unsigned char * img,
                        int img_width, int img_height,
                        int bitsperpixel);
void gpr_som_show_filters(gpr_som * som,
                          unsigned char * img,
                          int img_width, int img_height,
                          int bitsperpixel,
                          int patch_dimensions);

void gpr_som_stack_init(int no_of_layers,
                        int img_width, int img_height,
                        int bitsperpixel,
                        int patch_width,
                        int * som_dimension,
                        gpr_som_stack * stack,
                        unsigned int random_seed);

void gpr_som_stack_free(gpr_som_stack * stack);

void gpr_som_stack_learn(int no_of_samples,
                         unsigned char * img,
                         int img_width, int img_height,
                         int bitsperpixel,
                         gpr_som_stack * stack);

/* integralimage.c */

void update_integral_image(unsigned char * img,
                           int img_width, int img_height,
                           int bitsperpixel,
                           long * integral_image);

long get_integral(long * integral_image,
                  int tx, int ty, int bx, int by, int img_width);

/* centresurround.c */

void update_centre_surround(unsigned char * img,
                            int img_width, int img_height,
                            int bitsperpixel,
                            long * integral_image,
                            int patch_width,
                            int threshold,
                            unsigned char * output_img,
                            int output_img_width,
                            int output_img_height);

void amplify_image(unsigned char * img,
                   int img_width, int img_height,
                   int bitsperpixel,
                   int amplify,
                   int min_y, int max_y,
                   unsigned char * output_img);

/* resize.h */
int enlarge_image(unsigned char img[], int width, int height,
                  int bitsperpixel,
                  unsigned char enlarged[],
                  int enlarged_width, int enlarged_height);
int resize_image(unsigned char img[], int width, int height,
                 int bitsperpixel,
                 unsigned char resized[],
                 int resized_width, int resized_height);
int resize_thresholded_image(unsigned char img[], int width, int height,
                             int bitsperpixel,
                             unsigned char resized[],
                             int resized_width, int resized_height);

/* centre_of_gravity.c */
void centre_of_gravity(unsigned char * img,
                       int img_width, int img_height,
                       int bitsperpixel,
                       int * cx, int * cy);
/* rotation.c */
void create_rotation_lookup(int img_width, int img_height,
                            int no_of_rotations,
                            short lookup[]);
int match_image_with_rotation(int img_width, int img_height,
                              unsigned char input[],
                              unsigned char template[], short rotation_lookup[],
                              int no_of_rotations,
                              int *offset_x, int *offset_y, float * score);
void learn_image_with_rotation(int img_width, int img_height,
                               unsigned char input[],
                               unsigned char template[], short rotation_lookup[],
                               int no_of_rotations,
                               int offset_x, int offset_y, int rotation_index);

/* draw.c */
void draw_line(unsigned char img[],
               unsigned int width, unsigned int height,
               int bitsperpixel,
               int tx, int ty, int bx, int by,
               int line_width,
               int r, int g, int b);
void draw_curve(unsigned char img[],
                unsigned int width, unsigned int height,
                int bitsperpixel,
                int x1, int y1,
                int x2, int y2,
                int x3, int y3,
                int line_width,
                int r, int g, int b);

/* symmetry.c */
int detect_vertical_symmetry(unsigned char img[],
                             unsigned int width, unsigned int height,
                             int bitsperpixel,
                             int * tx, int * ty, int * bx, int * by);

/* geometry.c */
void get_centroid(float perimeter_x0, float perimeter_y0,
                  float perimeter_x1, float perimeter_y1,
                  float perimeter_x2, float perimeter_y2,
                  float perimeter_x3, float perimeter_y3,
                  float * centre_x, float * centre_y);

float get_side_length(int side,
                      float perimeter_x0, float perimeter_y0,
                      float perimeter_x1, float perimeter_y1,
                      float perimeter_x2, float perimeter_y2,
                      float perimeter_x3, float perimeter_y3);

float get_longest_side(float perimeter_x0, float perimeter_y0,
                       float perimeter_x1, float perimeter_y1,
                       float perimeter_x2, float perimeter_y2,
                       float perimeter_x3, float perimeter_y3);

float get_shortest_side(float perimeter_x0, float perimeter_y0,
                        float perimeter_x1, float perimeter_y1,
                        float perimeter_x2, float perimeter_y2,
                        float perimeter_x3, float perimeter_y3);

int intersection(float x0, float y0,
                 float x1, float y1,
                 float x2, float y2,
                 float x3, float y3,
                 float * xi, float * yi);

float point_dist_from_line(float x0, float y0,
                           float x1, float y1,
                           float point_x, float point_y,
                           float * line_intersection_x,
                           float * line_intersection_y);

int ransac_fit(int edges[], int no_of_edges,
               int linefit[], int max_edges,
               float max_deviation,
               int no_of_samples,
               int no_of_edge_samples,
               float *x0, float *y0,
               float *x1, float *y1);

float corner_angle(float x0, float y0,
                   float x1, float y1,
                   float x2, float y2);

/* grid.c */

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
                 struct grid_2d * grid);

void create_grid_from_pattern(int dimension_x, int dimension_y,
                              struct grid_2d * grid,
                              unsigned char occupancy[]);

int detect_timing_pattern(unsigned char mono_img[],
                          int width, int height,
                          float perimeter_x0, float perimeter_y0,
                          float perimeter_x1, float perimeter_y1,
                          float perimeter_x2, float perimeter_y2,
                          float perimeter_x3, float perimeter_y3,
                          int sampling_radius,
                          unsigned char debug,
                          unsigned char image_data[],
                          int debug_frequency);

void free_grid(struct grid_2d * grid);

void show_grid(struct grid_2d * grid);

void show_grid_image(struct grid_2d * grid,
                     unsigned char image_data[],
                     int image_width, int image_height,
                     int image_bitsperpixel,
                     int sampling_radius, int sampling_pattern);

/* decode.c */

void datamatrix_decode(struct grid_2d * grid, unsigned char debug,
                       char result[]);

/* utils.c */

int decode_strcat(char * result, char * text);

int decode_strcat_char(char * result, char chr);

/* shrinkgrow.c */

void shrinkwrap_shape_perimeter(int erosion_itterations, int dilate_itterations,
                                float * perimeter_x0, float * perimeter_y0,
                                float * perimeter_x1, float * perimeter_y1,
                                float * perimeter_x2, float * perimeter_y2,
                                float * perimeter_x3, float * perimeter_y3);

unsigned char expand_perimeter_sides(unsigned char mono_img[],
                                     int width, int height,
                                     float * perimeter_x0, float * perimeter_y0,
                                     float * perimeter_x1, float * perimeter_y1,
                                     float * perimeter_x2, float * perimeter_y2,
                                     float * perimeter_x3, float * perimeter_y3,
                                     int max_extension_percent,
                                     unsigned char debug,
                                     unsigned char image_data[],
                                     int image_bitsperpixel);

unsigned char contract_perimeter_sides(unsigned char mono_img[],
                                       int width, int height,
                                       float * perimeter_x0, float * perimeter_y0,
                                       float * perimeter_x1, float * perimeter_y1,
                                       float * perimeter_x2, float * perimeter_y2,
                                       float * perimeter_x3, float * perimeter_y3,
                                       int min_extension_percent,
                                       unsigned char debug,
                                       unsigned char image_data[],
                                       int image_bitsperpixel);

/* quality.c */

void calculate_quality_metrics(struct grid_2d * grid);

/* reader.c */

int read_datamatrix(unsigned char image_data[],
                    int image_width, int image_height,
                    int image_bitsperpixel,
                    unsigned char debug,
                    char output_filename[],
                    int test_ml_threshold,
                    int test_erode, int test_dilate,
                    float test_edge_threshold,
                    int test_frequency,
                    char * decode_result);

/* tests.c */

void run_all_tests();

#endif
