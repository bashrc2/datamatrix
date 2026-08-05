/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  datamatrix
 *  Copyright (c) 2025-2026, Bob Mottram
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
#include <limits.h>
#include "lodepng.h"

#define DATAMATRIX_VERSION_STRING  "v1.0.0"

#define GPR_MISSING_VALUE          9999
#define PARALLEL_LINES             -2
#define UNKNOWN_DISTANCE           999999
#define UNKNOWN_VALUE              9999
#define EMPTY_CHAR                 ' '
#define NO_LINE_FIT                999998
#define NO_OFFSET                  999998
#define PI                         3.1415927
#define FREQ_DIMENSION             256
#define MAX_DECODE_LENGTH          1024
#define MAX_DECODE_STRING_LENGTH   20000
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

/* position for formatted description when displaying an encoded
   datamatrix as an image */
#define DESCRIPTION_BELOW 0
#define DESCRIPTION_ABOVE 1
#define DESCRIPTION_RIGHT 2
#define DESCRIPTION_LEFT  3

/* dot peen font character matrix */
#define FONT_WIDTH  5
#define FONT_HEIGHT 10

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#ifndef UINT_TO_INT
#define UINT_TO_INT(i) (((i) <= (unsigned int)INT_MAX) ? ((int)(i)) : (-(int)~(i) - 1))
#endif

#ifndef NO_OF_VALID_SQUARES
#define NO_OF_VALID_SQUARES 24
#endif

#ifndef NO_OF_VALID_RECTANGLES
#define NO_OF_VALID_RECTANGLES (6+13)
#endif

#ifndef SQUARE
#define SQUARE(a) (a*a)
#endif

#ifndef SQUARE_MAG
#define SQUARE_MAG(a,b) ((SQUARE(a)) + (SQUARE(b)))
#endif

#ifndef HYPOT
#define HYPOT(a,b) ((float)sqrt(SQUARE_MAG((a), (b))))
#endif

enum edge_segment_join_types {
    JOIN_NONE, JOIN_START_TO_START, JOIN_START_TO_END,
    JOIN_END_TO_START, JOIN_END_TO_END
};

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
                        int sampling_step,
                        unsigned char thresholded[]);

unsigned char get_percent_high(unsigned char mono_img[],
                               int image_width, int image_height);

/* convert.c */

void mono_to_colour(const unsigned char img[], int width, int height,
                    int bitsperpixel,
                    unsigned char colour[]);

void colour_to_mono(const unsigned char img[], int width, int height,
                    int bitsperpixel,
                    unsigned char mono[]);

/* erosiondilation.c */

void dilate(const unsigned char img[], const int width, const int height,
            unsigned char buffer[],
            const int itterations,
            unsigned char result[]);

void erode(const unsigned char img[], const int width, const int height,
           unsigned char buffer[],
           const int itterations,
           unsigned char result[]);


/* darklight.c */

void darklight(const unsigned char img[],
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

unsigned char segment_edges_within_roi(struct line_segments * segments,
                                       int width, int height,
                                       int roi_radius_percent);

int get_segment_aspect_ratio(const struct line_segments * segments,
                             int index);

int get_peripheral_edges(struct line_segments * segments,
                         const int index, const int width, const int height);

float get_segments_orientation(struct line_segments * segments,
                               const int width, const int height,
                               const int quantization_degrees);

int fit_perimeter_to_sides(const struct line_segments * segments,
                           const int width, const int height,
                           float * perimeter_x0, float * perimeter_y0,
                           float * perimeter_x1, float * perimeter_y1,
                           float * perimeter_x2, float * perimeter_y2,
                           float * perimeter_x3, float * perimeter_y3,
                           const unsigned char debug,
                           const int try_config, const int seg_idx,
                           const int offset,
                           unsigned char thr_edges_image_data[],
                           const int resized_thresholded_width,
                           const int resized_thresholded_height,
                           const int image_bitsperpixel,
                           char * debug_filename);

int get_shape_aspect_ratio(const float perimeter_x0, const float perimeter_y0,
                           const float perimeter_x1, const float perimeter_y1,
                           const float perimeter_x2, const float perimeter_y2,
                           const float perimeter_x3, const float perimeter_y3);

void free_line_segments(struct line_segments * segments);

void show_line_segments(const struct line_segments * segments,
                        unsigned char result[], int width, int height,
                        int result_bitsperpixel);
void show_square_line_segments(const struct line_segments * segments,
                               unsigned char result[],
                               const int width, const int height,
                               const int result_bitsperpixel);
void show_rectangular_line_segments(const struct line_segments * segments,
                                    unsigned char result[],
                                    const int width, const int height,
                                    const int result_bitsperpixel);
void show_peripheral_edges(const struct line_segments * segments,
                           unsigned char result[],
                           const int width, const int height,
                           const int result_bitsperpixel);
void show_perimeter(const struct line_segments * segments,
                    unsigned char result[],
                    const int width, const int height,
                    const int result_bitsperpixel);
void show_perimeter_intersection(const struct line_segments * segments,
                                 unsigned char result[],
                                 const int width, const int height,
                                 const int result_bitsperpixel,
                                 const int side1, const int side2);
void show_shape_perimeter(unsigned char result[],
                          const int width, const int height,
                          const int result_bitsperpixel,
                          const float perimeter_x0, const float perimeter_y0,
                          const float perimeter_x1, const float perimeter_y1,
                          const float perimeter_x2, const float perimeter_y2,
                          const float perimeter_x3, const float perimeter_y3);
void show_L_shape_perimeter(const struct grid_2d * grid,
                            unsigned char image_data[],
                            const int image_width, const int image_height,
                            const int image_bitsperpixel);
unsigned char rectangular_joined_line_segments(const int aspect_ratio);

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
               int width, int height,
               int bitsperpixel,
               int tx, int ty, int bx, int by,
               int line_width,
               int r, int g, int b);

void draw_dot(unsigned char img[],
              int width, int height,
              int bitsperpixel,
              int centre_x, int centre_y, int radius,
              int r, int g, int b);

/* dotpeen_font.c */

void draw_text(unsigned char img[],
               int width, int height,
               int bitsperpixel,
               int text_x, int text_y, int character_width,
               int line_spacing,
               int character_separation,
               int r, int g, int b,
               const char * text);

void draw_text_svg(FILE * fp_image,
                   int width, int height,
                   int text_x, int text_y, int character_width,
                   int line_spacing,
                   int character_separation,
                   const char * text);

int description_text_width(const char * description);
int description_text_lines(const char * description);

/* geometry.c */

int point_in_polygon(const int x, const int y,
                     const int points[], const int no_of_points);

void get_centroid(const float perimeter_x0, const float perimeter_y0,
                  const float perimeter_x1, const float perimeter_y1,
                  const float perimeter_x2, const float perimeter_y2,
                  const float perimeter_x3, const float perimeter_y3,
                  float * centre_x, float * centre_y);

float get_side_length(const int side,
                      const float perimeter_x0, const float perimeter_y0,
                      const float perimeter_x1, const float perimeter_y1,
                      const float perimeter_x2, const float perimeter_y2,
                      const float perimeter_x3, const float perimeter_y3);

float get_longest_side(const float perimeter_x0, const float perimeter_y0,
                       const float perimeter_x1, const float perimeter_y1,
                       const float perimeter_x2, const float perimeter_y2,
                       const float perimeter_x3, const float perimeter_y3);

float get_shortest_side(const float perimeter_x0, const float perimeter_y0,
                        const float perimeter_x1, const float perimeter_y1,
                        const float perimeter_x2, const float perimeter_y2,
                        const float perimeter_x3, const float perimeter_y3);

int intersection(const float x0, const float y0,
                 const float x1, const float y1,
                 const float x2, const float y2,
                 const float x3, const float y3,
                 float * xi, float * yi);

float point_dist_from_line(const float x0, const float y0,
                           const float x1, const float y1,
                           const float point_x, const float point_y);

int ransac_fit(const int edges[], const int no_of_edges,
               int linefit[], const int max_edges,
               const float max_deviation,
               const int no_of_samples,
               int no_of_edge_samples,
               float *x0, float *y0,
               float *x1, float *y1);

float corner_angle(const float x0, const float y0,
                   const float x1, const float y1,
                   const float x2, const float y2);

/* grid.c */

float get_cell_width(struct grid_2d * grid);

void create_grid(const int dimension_x, const int dimension_y,
                 const float perimeter_x0,
                 const float perimeter_y0,
                 const float perimeter_x1,
                 const float perimeter_y1,
                 const float perimeter_x2,
                 const float perimeter_y2,
                 const float perimeter_x3,
                 const float perimeter_y3,
                 const unsigned char mono_img[],
                 const int image_width, const int image_height,
                 int sampling_radius,
                 const int sampling_pattern,
                 struct grid_2d * grid);

void create_grid_from_pattern(const int dimension_x, const int dimension_y,
                              struct grid_2d * grid,
                              const unsigned char occupancy[]);

int detect_timing_pattern(unsigned char mono_img[],
                          const int width, const int height,
                          const int minimum_grid_dimension,
                          const int maximum_grid_dimension,
                          const float perimeter_x0, const float perimeter_y0,
                          const float perimeter_x1, const float perimeter_y1,
                          const float perimeter_x2, const float perimeter_y2,
                          const float perimeter_x3, const float perimeter_y3,
                          const int sampling_radius,
                          const unsigned char debug,
                          unsigned char image_data[],
                          const int debug_frequency);

void rotate_grid(struct grid_2d * grid);

unsigned char get_grid_occupancy_percent(const struct grid_2d * grid);

void free_grid(struct grid_2d * grid);

void show_grid(const struct grid_2d * grid);

void show_grid_image(struct grid_2d * grid,
                     unsigned char image_data[],
                     const int image_width, const int image_height,
                     const int image_bitsperpixel,
                     const int sampling_radius, const int sampling_pattern);

/* decode.c */

void datamatrix_decode(struct grid_2d * grid, unsigned char debug,
                       const char gs1_url[], char result[],
                       unsigned char human_readable);
unsigned char condense_data_blocks(struct grid_2d * grid,
                                   unsigned char debug);

/* utils.c */

int decode_strcat(char * result, const char * text);

int decode_strcat_char(char * result, char chr);

void * safemalloc(size_t n);

int getline2(char line[], FILE * fp);

int ends_with(char * text, char * match);

int is_letter_lower(char ch);

int is_letter_upper(char ch);

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

unsigned char overall_quality_grade(const struct grid_2d * grid);

void calculate_quality_metrics(struct grid_2d * grid,
                               const unsigned char image_data[],
                               const unsigned char thresholded_image_data[],
                               const int image_width, const int image_height,
                               const int image_bitsperpixel,
                               const unsigned char histogram_module_centres,
                               const char histogram_filename[],
                               const char cell_shape_filename[]);

void show_quality_metrics(const struct grid_2d * grid,
                          const unsigned char csv,
                          const unsigned char json,
                          const unsigned char yaml,
                          const float aperture,
                          const int light_nm,
                          const int light_angle_degrees);

/* report.c */

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
                              char footer[]);

/* gs1_utils.c */

int get_gtin_check_digit(const char gtin[],
                         const unsigned char includes_check_digit);

float get_temperature(const char data_str[]);

char * get_issn(const char data_str[]);

char * get_package_type(const char package_code[]);

char * get_gs1_company_prefix(const char data_str[]);

char * get_company_prefix_str(char company_prefix_code[],
                              const int start_index, const char data_str[]);

char * get_country(const char data_str[]);

char * get_country_alpha2(const char data_str[]);

char * get_currency_value(const int application_identifier,
                          const char data_str[]);

char * get_decimal_value(const int application_identifier,
                         const char data_str[]);

char * get_meat_cut(const char data_str[]);

char * get_north_american_coupon(char data_str[],
                                 char company_prefix_code[]);

void calc_check_character(const char data_str[], const int check_characters,
                          char check_character_pair[]);

char * get_production_method(const char data_str[]);

char * get_fishing_gear_type(const char data_str[]);

char * get_aquatic_species(const char data_str[]);

char * get_fishing_area(const char data_str[]);

/* gs1.c */

void gs1_semantics(char result[],
                   char gs1_result[],
                   const char gs1_url[],
                   unsigned char debug,
                   int * application_identifier,
                   unsigned char * application_identifier_length,
                   int * application_data_start,
                   int * application_data_end,
                   unsigned char * application_data_variable);

/* gs1_encode.c */

int gs1_encode(const int application_identifier, char data_str[],
               char encode_text[], char encode_description[]);

/* hibc_encode.c */

int hibc_encode(const char * application_identifier, const char data_str[],
                char encode_text[]);

char hibc_check_character(const char * encode_text);

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
                    const unsigned char debug);

/* data_id.c */

int get_data_identifier(char data_str[],
                        char id[],
                        char identifier[], char value[]);

char * data_id_convert_date(char * id_human_readable, char * id_value);

char * data_id_convert_company_prefix(char * id_human_readable, char * id_value);

/* dimensions.c */

const int * get_valid_squares();

const int * get_valid_rectangles();

/* reader.c */

int read_datamatrix(unsigned char image_data[],
                    const int image_width, const int image_height,
                    const int image_bitsperpixel,
                    const unsigned char debug,
                    char output_filename[],
                    const char grid_filename[],
                    const int test_ml_threshold,
                    const int test_erode, const int test_dilate,
                    const int test_frequency,
                    const unsigned char verify,
                    const unsigned char csv,
                    const unsigned char json,
                    const unsigned char yaml,
                    const int minimum_grid_dimension,
                    const int maximum_grid_dimension,
                    const char gs1_url[],
                    const unsigned char raw_decode,
                    const unsigned char histogram_module_centres,
                    char histogram_filename[],
                    const int resized_thresholded_width,
                    const int sampling_radius,
                    const int min_segment_length,
                    const float aperture,
                    const int light_nm,
                    const int light_angle_degrees,
                    const unsigned char is_square,
                    const unsigned char is_rectangle,
                    const char cell_shape_filename[],
                    char report_template[],
                    char report_filename[],
                    char logo_filename[],
                    char address_line1[],
                    char address_line2[],
                    char address_line3[],
                    char phone[],
                    char email[],
                    char website[],
                    char footer[],
                    const int darklight_sampling_step,
                    const int max_high_pixels_percent,
                    const int segment_join_radius,
                    const int min_peripheral_edges,
                    char * decode_result);

void decode_as_json(const char * decode_result);

void decode_as_yaml(const char * decode_result);

/* tests.c */

void run_all_tests();

/* encode.c */

void encode_image(unsigned char img[],
                  const int width, const int height,
                  const int bitsperpixel, const unsigned char * grid,
                  const int encode_width, const int encode_height,
                  const unsigned char square_modules,
                  const char * description,
                  const unsigned char description_position,
                  const int character_width,
                  const int line_spacing,
                  const int character_separation);

void encode_svg(const char * image_filename,
                const int width, const int height,
                const unsigned char * grid,
                const int encode_width, const int encode_height,
                const unsigned char square_modules,
                const char * description,
                const unsigned char description_position,
                const int character_width,
                const int line_spacing,
                const int character_separation);

/* decode_text.c */

int decode_datamatrix_from_text(const char * datamatrix_text,
                                const char * gs1_url,
                                char * custom_dot_char,
                                char * custom_empty_char,
                                unsigned char debug);

/* encode_text.c */

int encode_datamatrix_to_text_or_image(const char * text,
                                       const char * description,
                                       const unsigned char description_position,
                                       const int character_width,
                                       const int line_spacing,
                                       const int encode_scale,
                                       const unsigned char is_square,
                                       const unsigned char csv,
                                       const unsigned char show_coords,
                                       const float coords_offset_x,
                                       const float coords_offset_y,
                                       char * image_filename,
                                       int encode_image_width,
                                       const char * dot_char,
                                       const char * empty_char,
                                       const unsigned char square_modules,
                                       const unsigned char debug);

/* png2.c */

unsigned char * read_png_file(const char * filename,
                              unsigned int * width,
                              unsigned int * height,
                              unsigned int * bitsperpixel);

int write_png_file(const char * filename,
                   const int width, const int height,
                   const int bitsperpixel,
                   unsigned char buffer[]);

/* help.c */

void show_help();

#endif
