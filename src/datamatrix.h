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

#define GPR_MISSING_VALUE          9999
#define PARALLEL_LINES             -2
#define UNKNOWN_DISTANCE           999999
#define NO_LINE_FIT                999998
#define NO_OFFSET                  999998
#define PI                         3.1415927
#define FREQ_DIMENSION             256
#define MAX_DECODE_LENGTH          1024
/* max perimeter side length in pixels */
#define MAX_PERIMETER_SIDE_LENGTH  4096
#define MIN_GRID_DIMENSION         8
#define MAX_GRID_DIMENSION         144
#define MAX_CODEWORDS              (MAX_GRID_DIMENSION*MAX_GRID_DIMENSION)
#define MIN_OCCUPIED_CELLS_PERCENT 40
#define MAX_OCCUPIED_CELLS_PERCENT 60

/* patterns used when sampling grid cell pixels */
#define SAMPLING_PATTERN_SOLID     0
#define SAMPLING_PATTERN_RING      1

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#ifndef NO_OF_VALID_SQUARES
#define NO_OF_VALID_SQUARES 24
#endif

#ifndef NO_OF_VALID_RECTANGLES
#define NO_OF_VALID_RECTANGLES (6+13)
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
  struct perimeter_points quiet_zone_perimeter;
  unsigned char ** occupancy;
  unsigned char ** occupancy_buffer;
  unsigned char * damage;
  unsigned char * original_damage;
  unsigned char * damage_buffer;
  int ** codeword_pattern;
  unsigned char * codeword;
  unsigned char * corrected_codewords;
  int dimension_x, dimension_y;
  int no_of_errors;
  int no_of_erasures;
  int * erasures;

  /* orientation of occupancy/damage arrays */
  unsigned char rotated, flipped, mirrored;

  /* temporary decoding arrays used during condensation */
  unsigned char ** temp_occupancy;
  unsigned char * temp_damage;

  /* quality metrics */
  unsigned char gs1_datamatrix;
  unsigned char iso15434_datamatrix;
  unsigned char hibc_datamatrix;
  unsigned char minimum_reflectance;
  unsigned char minimum_reflectance_grade;
  unsigned char unused_error_correction;
  unsigned char unused_error_correction_grade;
  unsigned char cell_fill;
  unsigned char distributed_damage;
  unsigned char fixed_pattern_damage;
  unsigned char fixed_pattern_damage_grade;
  unsigned char clock_track_regularity;
  unsigned char clock_track_regularity_grade;
  float angle_of_distortion;
  unsigned char symbol_contrast;
  unsigned char symbol_contrast_grade;
  float axial_non_uniformity;
  unsigned char axial_non_uniformity_grade;
  float grid_non_uniformity;
  unsigned char grid_non_uniformity_grade;
  unsigned char modulation;
  unsigned char modulation_grade;
  unsigned char contrast_uniformity;
  float elongation;
  int dots_per_element;
  unsigned char quiet_zone;

  /* decoding arrays */
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

/* threshold.c */

int meanlight_threshold(unsigned char img[], int width, int height,
                        int bitsperpixel, int threshold,
                        int sampling_radius_percent,
                        unsigned char thresholded[]);

/* convert.c */

void mono_to_colour(unsigned char img[], int width, int height,
                    int bitsperpixel,
                    unsigned char colour[]);

void colour_to_mono(unsigned char img[], int width, int height,
                    int bitsperpixel,
                    unsigned char mono[]);

/* erosiondilation.c */

void dilate(unsigned char img[], int width, int height,
            unsigned char buffer[],
            int itterations,
            unsigned char result[]);

void erode(unsigned char img[], int width, int height,
           unsigned char buffer[],
           int itterations,
           unsigned char result[]);


/* darklight.c */

void darklight(unsigned char img[],
               int width, int height,
               int sample_step,
               int sampling_radius_percent,
               unsigned char * dark, unsigned char * light);

/* edges.c */

void detect_edges_binary(unsigned char img[],
                         int width, int height,
                         unsigned char buffer[]);

void get_line_segments(unsigned char edges_image[], int width, int height,
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
                        unsigned char result[], int width, int height,
                        int result_bitsperpixel);
void show_square_line_segments(struct line_segments * segments,
                               unsigned char result[], int width, int height,
                               int result_bitsperpixel);
void show_rectangular_line_segments(struct line_segments * segments,
                                    unsigned char result[], int width, int height,
                                    int result_bitsperpixel);
void show_peripheral_edges(struct line_segments * segments,
                           unsigned char result[], int width, int height,
                           int result_bitsperpixel);
void show_perimeter(struct line_segments * segments,
                    unsigned char result[], int width, int height,
                    int result_bitsperpixel);
void show_shape_perimeter(unsigned char result[], int width, int height,
                          int result_bitsperpixel,
                          float perimeter_x0, float perimeter_y0,
                          float perimeter_x1, float perimeter_y1,
                          float perimeter_x2, float perimeter_y2,
                          float perimeter_x3, float perimeter_y3);
void show_L_shape_perimeter(struct grid_2d * grid,
                            unsigned char image_data[],
                            int image_width, int image_height,
                            int image_bitsperpixel,
                            float perimeter_x0, float perimeter_y0,
                            float perimeter_x1, float perimeter_y1,
                            float perimeter_x2, float perimeter_y2,
                            float perimeter_x3, float perimeter_y3);
unsigned char rectangular_joined_line_segments(int aspect_ratio);

/* resize.c */

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

/* draw.c */

void draw_line(unsigned char img[],
               unsigned int width, unsigned int height,
               int bitsperpixel,
               int tx, int ty, int bx, int by,
               int line_width,
               int r, int g, int b);

void draw_dot(unsigned char img[],
              unsigned int width, unsigned int height,
              int bitsperpixel,
              int centre_x, int centre_y, int radius,
              int r, int g, int b);

/* geometry.c */

int point_in_polygon(int x, int y, int points[], int no_of_points);

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

float get_cell_width(struct grid_2d * grid);

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
                          int minimum_grid_dimension,
                          int maximum_grid_dimension,
                          float perimeter_x0, float perimeter_y0,
                          float perimeter_x1, float perimeter_y1,
                          float perimeter_x2, float perimeter_y2,
                          float perimeter_x3, float perimeter_y3,
                          int sampling_radius,
                          unsigned char debug,
                          unsigned char image_data[],
                          int debug_frequency);

void rotate_grid(struct grid_2d * grid);

unsigned char get_grid_occupancy_percent(struct grid_2d * grid);

void free_grid(struct grid_2d * grid);

void show_grid(struct grid_2d * grid);

void show_grid_image(struct grid_2d * grid,
                     unsigned char image_data[],
                     int image_width, int image_height,
                     int image_bitsperpixel,
                     int sampling_radius, int sampling_pattern);

/* decode.c */

void datamatrix_decode(struct grid_2d * grid, unsigned char debug,
                       char gs1_url[], char result[],
                       unsigned char human_readable);
unsigned char condense_data_blocks(struct grid_2d * grid,
                                   unsigned char debug);

/* utils.c */

int decode_strcat(char * result, char * text);

int decode_strcat_char(char * result, char chr);

void * safemalloc(int n);

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

void calculate_quality_metrics(struct grid_2d * grid,
                               unsigned char image_data[],
                               unsigned char thresholded_image_data[],
                               int image_width, int image_height,
                               int image_bitsperpixel,
                               unsigned char histogram_module_centres,
                               char histogram_filename[],
                               char cell_shape_filename[]);

void show_quality_metrics(struct grid_2d * grid,
                          unsigned char csv, unsigned char json,
                          float aperture,
                          int light_nm,
                          int light_angle_degrees);

/* gs1.c */

void gs1_semantics(char result[],
                   char gs1_result[],
                   char gs1_url[],
                   unsigned char debug,
                   int * application_identifier,
                   unsigned char * application_identifier_length,
                   int * application_data_start,
                   int * application_data_end,
                   unsigned char * application_data_variable);

char * get_currency_value(int application_identifier,
                          char data_str[]);

char * get_decimal_value(int application_identifier,
                         char data_str[]);

char * get_country(char data_str[]);

char * get_coupon(char data_str[]);

char * get_issn(char data_str[]);

/* condense.c */

unsigned char condense_data_blocks(struct grid_2d * grid,
                                   unsigned char debug);

/* iso15434.c */

void iso15434_semantics(char result[],
                        char iso15434_result[],
                        unsigned char debug,
                        unsigned char * is_iso1543,
                        char format_code[],
                        int * iso15434_data_start,
                        char iso15434_uii[]);

char * iso15434_translate_data_qualifier(char result[],
                                         int start_index,
                                         int end_index,
                                         char iso15434_uii[],
                                         char format_code[],
                                         unsigned char debug);

/* hibc.c */

void hibc_semantics(char result[],
                    char hibc_result[],
                    unsigned char debug);

/* data_id.c */

int get_data_identifier(char data_str[],
                        char id[],
                        char identifier[], char value[]);

char * data_id_convert_date(char * id_human_readable, char * id_value);

/* dimensions.c */

int * get_valid_squares();

int * get_valid_rectangles();

/* reader.c */

int read_datamatrix(unsigned char image_data[],
                    int image_width, int image_height,
                    int image_bitsperpixel,
                    unsigned char debug,
                    char output_filename[],
                    char grid_filename[],
                    int test_ml_threshold,
                    int test_erode, int test_dilate,
                    int test_frequency,
                    unsigned char verify,
                    unsigned char csv,
                    unsigned char json,
                    int minimum_grid_dimension,
                    int maximum_grid_dimension,
                    char gs1_url[],
                    unsigned char raw_decode,
                    unsigned char histogram_module_centres,
                    char histogram_filename[],
                    int resized_thresholded_width,
                    int sampling_radius,
                    int min_segment_length,
                    float aperture,
                    int light_nm,
                    int light_angle_degrees,
                    unsigned char is_square,
                    unsigned char is_rectangle,
                    char cell_shape_filename[],
                    char * decode_result);

/* tests.c */

void run_all_tests();

/* help.c */

void show_help();

#endif
