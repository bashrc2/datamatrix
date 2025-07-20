/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix reader
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
 * \brief returns 1 if any decode has been achieved within any thread
 * \param thr_decode_result array of decode strings, one for each thread
 * \param max_config the number of threads
 * \return 1 if any decode string has non-zero length, 0 otherwise
 */
unsigned char any_decode(char * thr_decode_result[], int max_config)
{
  for (int i = 0; i < max_config; i++) {
    if (strlen(thr_decode_result[i]) > 0) return 1;
  }
  return 0;
}

/**
 * \brief read a datamatrix from an image
 * \param image_data image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bitsperpixel Number of bits per pixel
 * \param debug set to 1 if in debug mode
 * \param output_filename array containing output image with detected datamatrix
 * \param grid_filename array containing grid image with detected datamatrix
 * \param test_ml_threshold test with a specific mean light value
 * \param test_erode test with a specific erosion value
 * \param test_dilate test with a specific dilation value
 * \param test_frequency test using a known grid dimension
 * \param verify set to 1 if symbol quality metrics are to be calculated
 * \param csv set to 1 if symbol quality metrics to be in CSV format
 * \param json set to 1 if symbol quality metrics to be in json format
 * \param yaml set to 1 if symbol quality metrics to be in yaml format
 * \param minimum_grid_dimension minimum grid dimension
 * \param maximum_grid_dimension maximum grid dimension
 * \param gs1_url url for GS1 digital link
 * \param raw_decode show the raw decode rather than a human readable one
 * \param histogram_module_centres sample only module centres when creating histogram
 * \param histogram_filename optionally save a reflectance histogram
 * \param resized_thresholded_width width of the smaller binary image for perimeter detection
 * \param sampling_radius radius of pixels to be checked at each location in the grid
 * \param min_segment_length minimum edge segment length in pixels
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 * \param cell_shape_filename optional image showing cell shape
 * \param report_template filename of a template used to produce a verification report
 * \param report_filename filename of the verification report to be saved
 * \param logo_filename filename of logo to display at top of verification report
 * \param address_line1 First line of address to display at top of verification report
 * \param address_line2 Second line of address to display at top of verification report
 * \param address_line3 Third line of address to display at top of verification report
 * \param phone Phone number to display at top of verification report
 * \param email Email address to display at top of verification report
 * \param website Web URL to display at top of verification report
 * \param footer Footer text on verification report
 * \param darklight_sampling_step Step size for dark/light peaks calculation
 * \param max_high_pixels_percent Maximum percentage of high pixels after thresholding
 * \param decode_result returned decode text
 * \return zero on decode success, -1 otherwise
 */
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
                    unsigned char yaml,
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
                    int darklight_sampling_step,
                    int max_high_pixels_percent,
                    char * decode_result)
{
  int original_image_width = image_width;
  int original_image_height = image_height;
  int image_bytesperpixel = image_bitsperpixel/8;
  int try_config, best_config = -1;
  unsigned char human_readable = 1;
  const int segment_join_radius=6;

  /* the magic numbers
     Note that these are doubled sets so that we can also vary the
     meanlight sampling radius*/
  const int no_of_configs = 6*2;
  int max_config = no_of_configs;
  int ml_threshold_configs[] = {
    0, 0, 50, 10, 10, 20,
    0, 0, 50, 10, 10, 20
  };
  int erosion_itterations_configs[] = {
    1, 0, 1, 0, 0, 0,
    1, 0, 1, 0, 0, 0
  };
  int dilate_itterations_configs[] = {
    9, 5, 4, 6, 5, 2,
    9, 5, 4, 6, 5, 2};
  int ml_sampling_radius[] = {
    50, 50, 50, 50, 50, 50,
    20, 20, 20, 20, 20, 20
  };

  struct grid_2d grid[no_of_configs];
  struct line_segments segments[no_of_configs];
  char * thr_decode_result[no_of_configs];
  char * debug_filename[no_of_configs];
  int resized_thresholded_height =
    image_height * resized_thresholded_width / image_width;
  int timing_pattern_sampling_radius = sampling_radius;

  /* keep the original image data, for parallelization and later visualisations
     such as showing the detected perimeter */
  unsigned char * original_image_data =
    (unsigned char*)safemalloc(image_width*image_height*image_bytesperpixel);

  float best_perimeter_x0=-1, best_perimeter_y0=0;
  float best_perimeter_x1=0, best_perimeter_y1=0;
  float best_perimeter_x2=0, best_perimeter_y2=0;
  float best_perimeter_x3=0, best_perimeter_y3=0;

  /* in verification reports show the raw decode */
  if ((verify == 1) || (raw_decode == 1)) human_readable = 0;

  decode_result[0] = 0;

  if (original_image_data == NULL) {
    printf("Unable to create original image data\n");
    return -7;
  }

  /* keep a copy of the original image */
  memcpy(original_image_data, image_data,
         image_width*image_height*image_bytesperpixel);

  if ((test_ml_threshold > 0) ||
      (test_erode > 0) || (test_dilate > 0)) {
    max_config = 1;
  }

  /* create threaded decode results */
  for (try_config = 0; try_config < no_of_configs; try_config++) {
    thr_decode_result[try_config] = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
    /* initialise as empty string */
    thr_decode_result[try_config][0] = 0;
    debug_filename[try_config] = (char*)safemalloc(256*sizeof(char));
  }

  /* try a few different configurations of erosion/dilation, binary threshold
     and edge threshold */
#pragma omp parallel for
  for (try_config = 0; try_config < max_config; try_config++) {
    int meanlight_sampling_radius_percent = ml_sampling_radius[try_config];
    int segment_roi_percent = meanlight_sampling_radius_percent + 20;
    float corner_radians, angle_degrees;
    int curr_sampling_radius;
    int most_probable_frequency=0;
    int most_probable_frequency_x=0, most_probable_frequency_y=0;
    int aspect_ratio_percent;
    unsigned char test_specific_config_settings = 0;

    int ml_threshold = ml_threshold_configs[try_config];
    int erosion_itterations = erosion_itterations_configs[try_config];
    int dilate_itterations = dilate_itterations_configs[try_config];

    unsigned char * thr_image_data =
      (unsigned char*)safemalloc(image_width*image_height*image_bytesperpixel);
    /* thresholded/binary image */
    unsigned char * thr_meanlight_image_data =
      (unsigned char*)safemalloc(image_width*image_height*sizeof(unsigned char));
    /* the original thresholded/binary image, retained for parallelization */
    unsigned char * thr_original_meanlight_image_data =
      (unsigned char*)safemalloc(image_width*image_height*image_bytesperpixel);
    /* full size mono image used for perimeter fitting */
    unsigned char * thr_mono_img =
      (unsigned char*)safemalloc(image_width*image_height*sizeof(unsigned char));
    /* small binary image used for perimeter detection */
    unsigned char * thr_binary_image =
      (unsigned char*)safemalloc(resized_thresholded_width *
                                 resized_thresholded_height * sizeof(unsigned char));
    /* buffer images, full size and small */
    unsigned char * thr_buffer_img =
      (unsigned char*)safemalloc(image_width*image_height*sizeof(unsigned char));
    unsigned char * thr_binary_image_buffer =
      (unsigned char*)safemalloc(resized_thresholded_width *
                                 resized_thresholded_height * sizeof(unsigned char));
    /* array used to store edges and edge segments */
    unsigned char * thr_edges_image_data =
      (unsigned char*)safemalloc(resized_thresholded_width*resized_thresholded_height*
                                 image_bytesperpixel);

    /* make an image which will be used by this thread */
    memcpy(thr_image_data, original_image_data,
           image_width*image_height*image_bytesperpixel);

    if ((test_ml_threshold > 0) ||
        (test_erode > 0) || (test_dilate > 0)) {
      /* if we are only trying one combination of settings */
      test_specific_config_settings = 1;
      ml_threshold = test_ml_threshold;
      erosion_itterations = test_erode;
      dilate_itterations = test_dilate;
    }
    else if (test_frequency > 0) {
      test_specific_config_settings = 1;
    }

    /* convert to meanlight */
    meanlight_threshold(thr_image_data, image_width, image_height,
                        image_bitsperpixel, ml_threshold,
                        meanlight_sampling_radius_percent,
                        darklight_sampling_step,
                        thr_meanlight_image_data);
    if (debug == 1) {
      sprintf(debug_filename[try_config], "debug_%d_02_meanlight.png", try_config);
      write_png_file(debug_filename[try_config],
                     image_width, image_height, 24, thr_image_data);
    }
    /* keep a copy of the thresholded data for subsequent decoding */
    memcpy(thr_original_meanlight_image_data, thr_image_data,
           image_width*image_height*image_bitsperpixel/8);


    /* erosion or dilation */
    if ((erosion_itterations > 0) || (dilate_itterations > 0)) {
      /* make a mono image with one byte per pixel */
      colour_to_mono(thr_image_data,
                     image_width,image_height,image_bitsperpixel,thr_mono_img);
      /* apply morphology to the mono image */
      if (erosion_itterations > 0) {
        erode(thr_mono_img, image_width, image_height,
              thr_buffer_img,
              erosion_itterations,
              thr_mono_img);
      }
      if (dilate_itterations > 0) {
        dilate(thr_mono_img, image_width, image_height,
               thr_buffer_img,
               dilate_itterations,
               thr_mono_img);
      }
      /* convert the mono image back to colour */
      mono_to_colour(thr_mono_img, image_width, image_height,
                     image_bitsperpixel, thr_image_data);
      if (debug == 1) {
        sprintf(debug_filename[try_config], "debug_%d_03_erode_dilate.png", try_config);
        write_png_file(debug_filename[try_config],
                       image_width, image_height, 24, thr_image_data);
      }
    }

    /* resize the image to something small which will later be used for
       edge detection and edge segments */
    if (resize_thresholded_image(thr_image_data, image_width, image_height,
                                 image_bitsperpixel,
                                 thr_edges_image_data,
                                 resized_thresholded_width,
                                 resized_thresholded_height) != 0) {
      printf("Failed to resize thresholded image\n");
      free(thr_image_data);
      free(thr_meanlight_image_data);
      free(thr_original_meanlight_image_data);
      free(thr_mono_img);
      free(thr_binary_image);
      free(thr_binary_image_buffer);
      free(thr_buffer_img);
      free(thr_edges_image_data);
      continue;
    }
    if (debug == 1) {
      sprintf(debug_filename[try_config], "debug_%d_04_thresholded.png", try_config);
      write_png_file(debug_filename[try_config],
                     resized_thresholded_width, resized_thresholded_height, 24,
                     thr_edges_image_data);
    }

    unsigned char rectangular = 0;
    unsigned char perimeter_found = 0;
    float perimeter_x0=0, perimeter_y0=0;
    float perimeter_x1=0, perimeter_y1=0;
    float perimeter_x2=0, perimeter_y2=0;
    float perimeter_x3=0, perimeter_y3=0;

    colour_to_mono(thr_edges_image_data,
                   resized_thresholded_width,resized_thresholded_height,
                   image_bitsperpixel,thr_binary_image);

    detect_edges_binary(thr_binary_image,resized_thresholded_width,
                        resized_thresholded_height,
                        thr_binary_image_buffer);

    unsigned char high_pixels =
      get_percent_high(thr_binary_image_buffer,
                       resized_thresholded_width, resized_thresholded_height);
    if ((any_decode(&thr_decode_result[0], max_config) == 1) ||
        ((high_pixels < 5) || (high_pixels > max_high_pixels_percent))) {
      /* Too many high pixels */
      free(thr_image_data);
      free(thr_meanlight_image_data);
      free(thr_original_meanlight_image_data);
      free(thr_mono_img);
      free(thr_binary_image);
      free(thr_binary_image_buffer);
      free(thr_buffer_img);
      free(thr_edges_image_data);
      continue;
    }

    /* convert the mono image back to colour */
    mono_to_colour(thr_binary_image,
                   resized_thresholded_width, resized_thresholded_height,
                   image_bitsperpixel, thr_edges_image_data);
    if (debug == 1) {
      sprintf(debug_filename[try_config], "debug_%d_05_edges.png", try_config);
      write_png_file(debug_filename[try_config], resized_thresholded_width,
                     resized_thresholded_height, 24, thr_edges_image_data);
    }

    get_line_segments(thr_binary_image, resized_thresholded_width,
                      resized_thresholded_height, &segments[try_config],
                      min_segment_length);

    if (debug == 1) {
      show_line_segments(&segments[try_config], thr_edges_image_data,
                         resized_thresholded_width, resized_thresholded_height,
                         image_bitsperpixel);
      sprintf(debug_filename[try_config], "debug_%d_06a_line_segments.png", try_config);
      write_png_file(debug_filename[try_config],
                     resized_thresholded_width, resized_thresholded_height,
                     24, thr_edges_image_data);
    }

    unsigned char segments_percent =
      segment_edges_within_roi(&segments[try_config], resized_thresholded_width,
                               resized_thresholded_height, segment_roi_percent);
    if ((any_decode(&thr_decode_result[0], max_config) == 1) ||
        (segments_percent < 1)) {
      /* not enough line segments */
      free(thr_image_data);
      free(thr_meanlight_image_data);
      free(thr_original_meanlight_image_data);
      free(thr_mono_img);
      free(thr_binary_image);
      free(thr_binary_image_buffer);
      free(thr_buffer_img);
      free(thr_edges_image_data);
      continue;
    }

    if (debug == 1) {
      show_line_segments(&segments[try_config], thr_edges_image_data,
                         resized_thresholded_width, resized_thresholded_height,
                         image_bitsperpixel);
      sprintf(debug_filename[try_config],
              "debug_%d_06b_line_segments_with_roi.png", try_config);
      write_png_file(debug_filename[try_config],
                     resized_thresholded_width, resized_thresholded_height,
                     24, thr_edges_image_data);
    }

    join_line_segments(&segments[try_config], segment_join_radius);

    if (debug == 1) {
      show_line_segments(&segments[try_config], thr_edges_image_data,
                         resized_thresholded_width, resized_thresholded_height,
                         image_bitsperpixel);
      sprintf(debug_filename[try_config],
              "debug_%d_06c_joined_line_segments.png", try_config);
      write_png_file(debug_filename[try_config],
                     resized_thresholded_width, resized_thresholded_height,
                     24, thr_edges_image_data);
    }

    if (debug == 1) {
      if (is_rectangle == 0) {
        show_square_line_segments(&segments[try_config], thr_edges_image_data,
                                  resized_thresholded_width,
                                  resized_thresholded_height, image_bitsperpixel);
        sprintf(debug_filename[try_config],
                "debug_%d_07_square_joined_line_segments.png", try_config);
        write_png_file(debug_filename[try_config],
                       resized_thresholded_width, resized_thresholded_height, 24,
                       thr_edges_image_data);
      }
      if (is_square == 0) {
        show_rectangular_line_segments(&segments[try_config], thr_edges_image_data,
                                       resized_thresholded_width,
                                       resized_thresholded_height, image_bitsperpixel);
        sprintf(debug_filename[try_config],
                "debug_%d_08_rectangular_joined_line_segments.png", try_config);
        write_png_file(debug_filename[try_config],
                       resized_thresholded_width, resized_thresholded_height, 24,
                       thr_edges_image_data);
      }
    }

    /* check if no line segments found */
    if ((any_decode(&thr_decode_result[0], max_config) == 1) ||
        (segments[try_config].no_of_segments == 0)) {
      free_line_segments(&segments[try_config]);
      free(thr_image_data);
      free(thr_meanlight_image_data);
      free(thr_original_meanlight_image_data);
      free(thr_mono_img);
      free(thr_binary_image);
      free(thr_buffer_img);
      free(thr_edges_image_data);
      continue;
    }

    /* this must count upwards */
    for (int seg_idx = 0; seg_idx < segments[try_config].no_of_segments; seg_idx++) {
      int segment_index = segments[try_config].joins_sorted[seg_idx];
      get_peripheral_edges(&segments[try_config], segment_index,
                           resized_thresholded_width, resized_thresholded_height);

      int quantization_degrees = 5;
      get_segments_orientation(&segments[try_config], resized_thresholded_width,
                               resized_thresholded_height,
                               quantization_degrees);

      if (fit_perimeter_to_sides(&segments[try_config], resized_thresholded_width,
                                 resized_thresholded_height,
                                 &perimeter_x0, &perimeter_y0,
                                 &perimeter_x1, &perimeter_y1,
                                 &perimeter_x2, &perimeter_y2,
                                 &perimeter_x3, &perimeter_y3) != 0) {
        show_perimeter(&segments[try_config], thr_edges_image_data,
                       resized_thresholded_width, resized_thresholded_height,
                       image_bitsperpixel);
        continue;
      }

      aspect_ratio_percent =
        get_shape_aspect_ratio(perimeter_x0, perimeter_y0,
                               perimeter_x1, perimeter_y1,
                               perimeter_x2, perimeter_y2,
                               perimeter_x3, perimeter_y3);
      rectangular = is_rectangle;
      if ((aspect_ratio_percent < 80) || (aspect_ratio_percent > 120)) {
        if (is_square == 1) continue;
        rectangular = rectangular_joined_line_segments(aspect_ratio_percent);
        if (rectangular == 0) continue;
      }
      else if (is_rectangle == 1) continue;

      /* check that the corners are approximately square */
      /* first corner */
      corner_radians = corner_angle(perimeter_x0, perimeter_y0,
                                    perimeter_x1, perimeter_y1,
                                    perimeter_x2, perimeter_y2);
      if (corner_radians < 0) corner_radians = -corner_radians;
      if (corner_radians > PI)
        corner_radians = (2 * (float)PI) - corner_radians;
      angle_degrees = corner_radians / (float)PI * 180;
      if ((angle_degrees < 70) || (angle_degrees > 110)) {
        continue;
      }

      /* second corner */
      corner_radians = corner_angle(perimeter_x1, perimeter_y1,
                                    perimeter_x2, perimeter_y2,
                                    perimeter_x3, perimeter_y3);
      if (corner_radians < 0) corner_radians = -corner_radians;
      if (corner_radians > PI)
        corner_radians = (2 * (float)PI) - corner_radians;
      angle_degrees = corner_radians / (float)PI * 180;
      if ((angle_degrees < 70) || (angle_degrees > 110)) {
        continue;
      }
      /* shrink the perimeter according to the amount of dilation */
      shrinkwrap_shape_perimeter(erosion_itterations, dilate_itterations,
                                 &perimeter_x0, &perimeter_y0,
                                 &perimeter_x1, &perimeter_y1,
                                 &perimeter_x2, &perimeter_y2,
                                 &perimeter_x3, &perimeter_y3);

      if (debug == 1) {
        show_peripheral_edges(&segments[try_config], thr_edges_image_data,
                              resized_thresholded_width, resized_thresholded_height,
                              image_bitsperpixel);
        sprintf(debug_filename[try_config],
                "debug_%d_09_peripheral_edges.png", try_config);
        write_png_file(debug_filename[try_config],
                       resized_thresholded_width, resized_thresholded_height,
                       24, thr_edges_image_data);
      }
      if (debug == 1) {
        show_perimeter(&segments[try_config], thr_edges_image_data,
                       resized_thresholded_width, resized_thresholded_height,
                       image_bitsperpixel);
        sprintf(debug_filename[try_config],
                "debug_%d_10_perimeter.png", try_config);
        write_png_file(debug_filename[try_config],
                       resized_thresholded_width, resized_thresholded_height,
                       24, thr_edges_image_data);
      }

      if (debug == 1) {
        show_shape_perimeter(thr_edges_image_data, resized_thresholded_width,
                             resized_thresholded_height,
                             image_bitsperpixel,
                             perimeter_x0, perimeter_y0,
                             perimeter_x1, perimeter_y1,
                             perimeter_x2, perimeter_y2,
                             perimeter_x3, perimeter_y3);
        sprintf(debug_filename[try_config],
                "debug_%d_11_shape_perimeter_small.png", try_config);
        write_png_file(debug_filename[try_config],
                       resized_thresholded_width, resized_thresholded_height,
                       24, thr_edges_image_data);
      }

      /* convert back to original image resolution */
      perimeter_x0 = (perimeter_x0 * original_image_width) / resized_thresholded_width;
      perimeter_y0 = (perimeter_y0 * original_image_height) / resized_thresholded_height;
      perimeter_x1 = (perimeter_x1 * original_image_width) / resized_thresholded_width;
      perimeter_y1 = (perimeter_y1 * original_image_height) / resized_thresholded_height;
      perimeter_x2 = (perimeter_x2 * original_image_width) / resized_thresholded_width;
      perimeter_y2 = (perimeter_y2 * original_image_height) / resized_thresholded_height;
      perimeter_x3 = (perimeter_x3 * original_image_width) / resized_thresholded_width;
      perimeter_y3 = (perimeter_y3 * original_image_height) / resized_thresholded_height;

      /* because of the left/above oriented method of edge detection within
         detect_edges_binary, shift the perimeter right and down by the
         equivalent of half a pixel */
      float x_adjust = (original_image_width / resized_thresholded_width) * 0.5f;
      float y_adjust = (original_image_height / resized_thresholded_height) * 0.5f;
      perimeter_x0 += x_adjust;
      perimeter_y0 += y_adjust;
      perimeter_x1 += x_adjust;
      perimeter_y1 += y_adjust;
      perimeter_x2 += x_adjust;
      perimeter_y2 += y_adjust;
      perimeter_x3 += x_adjust;
      perimeter_y3 += y_adjust;

      memcpy(thr_image_data, original_image_data,
             image_width*image_height*image_bytesperpixel);

      show_shape_perimeter(thr_image_data, image_width, image_height,
                           image_bitsperpixel,
                           perimeter_x0, perimeter_y0,
                           perimeter_x1, perimeter_y1,
                           perimeter_x2, perimeter_y2,
                           perimeter_x3, perimeter_y3);
      if (debug == 1) {
        sprintf(debug_filename[try_config],
                "debug_%d_12_shape_perimeter.png", try_config);
        write_png_file(debug_filename[try_config],
                       image_width, image_height, 24, thr_image_data);
      }
      perimeter_found = 1;
      break;
    }

    /* we have a perimeter, now find the timing border frequency */
    if (perimeter_found == 1) {
      colour_to_mono(thr_original_meanlight_image_data,
                     image_width,image_height,image_bitsperpixel,thr_mono_img);

      if (debug == 1) {
        memcpy(thr_image_data, thr_original_meanlight_image_data,
               image_width*image_height*image_bytesperpixel);
      }

      /* expand the perimeter */
      if (expand_perimeter_sides(thr_mono_img, image_width, image_height,
                                 &perimeter_x0, &perimeter_y0,
                                 &perimeter_x1, &perimeter_y1,
                                 &perimeter_x2, &perimeter_y2,
                                 &perimeter_x3, &perimeter_y3,
                                 150, debug, thr_image_data,
                                 image_bitsperpixel) == 1) {
        if (debug == 1) {
          show_shape_perimeter(thr_image_data, image_width, image_height,
                               image_bitsperpixel,
                               perimeter_x0, perimeter_y0,
                               perimeter_x1, perimeter_y1,
                               perimeter_x2, perimeter_y2,
                               perimeter_x3, perimeter_y3);
          sprintf(debug_filename[try_config],
                  "debug_%d_13_expand_perimeter_sides.png", try_config);
          write_png_file(debug_filename[try_config],
                         image_width, image_height, 24, thr_image_data);
        }
      }

      if (debug == 1) {
        memcpy(thr_image_data, thr_original_meanlight_image_data,
               image_width*image_height*image_bytesperpixel);
      }

      /* contract the perimeter */
      if (contract_perimeter_sides(thr_mono_img, image_width, image_height,
                                   &perimeter_x0, &perimeter_y0,
                                   &perimeter_x1, &perimeter_y1,
                                   &perimeter_x2, &perimeter_y2,
                                   &perimeter_x3, &perimeter_y3,
                                   20, debug, thr_image_data,
                                   image_bitsperpixel) == 1) {
        if (debug == 1) {
          show_shape_perimeter(thr_image_data, image_width, image_height,
                               image_bitsperpixel,
                               perimeter_x0, perimeter_y0,
                               perimeter_x1, perimeter_y1,
                               perimeter_x2, perimeter_y2,
                               perimeter_x3, perimeter_y3);
          sprintf(debug_filename[try_config],
                  "debug_%d_14_contract_perimeter_sides.png", try_config);
          write_png_file(debug_filename[try_config],
                         image_width, image_height, 24, thr_image_data);
        }
      }

      /* find the timing border frequency */
      if (test_frequency > 0) {
        most_probable_frequency = test_frequency;
      }
      else {
        most_probable_frequency =
          detect_timing_pattern(thr_mono_img, image_width, image_height,
                                minimum_grid_dimension,
                                maximum_grid_dimension,
                                perimeter_x0, perimeter_y0,
                                perimeter_x1, perimeter_y1,
                                perimeter_x2, perimeter_y2,
                                perimeter_x3, perimeter_y3,
                                timing_pattern_sampling_radius,
                                0, thr_image_data, 0);
      }
      if ((most_probable_frequency > 0) && (debug == 1)) {
        memcpy(thr_image_data, thr_original_meanlight_image_data,
               image_width*image_height*(image_bitsperpixel/8));
        detect_timing_pattern(thr_mono_img, image_width, image_height,
                              minimum_grid_dimension,
                              maximum_grid_dimension,
                              perimeter_x0, perimeter_y0,
                              perimeter_x1, perimeter_y1,
                              perimeter_x2, perimeter_y2,
                              perimeter_x3, perimeter_y3, 1,
                              debug, thr_image_data,
                              most_probable_frequency);
        sprintf(debug_filename[try_config],
                "debug_%d_15_detect_timing_pattern.png", try_config);
        write_png_file(debug_filename[try_config],
                       image_width, image_height, 24, thr_image_data);
      }

      if (most_probable_frequency > 0) {
        if (debug == 1) {
          printf("Frequency: %d\n", most_probable_frequency);
        }
        /* sample grid cells in different patterns */
        curr_sampling_radius = sampling_radius;
        for (int curr_sampling_pattern = SAMPLING_PATTERN_SOLID;
             curr_sampling_pattern <= SAMPLING_PATTERN_RING;
             curr_sampling_pattern++) {
          /* increase the radius for ring sampling */
          if (curr_sampling_pattern == SAMPLING_PATTERN_RING) curr_sampling_radius+=2;
          create_grid(most_probable_frequency, most_probable_frequency,
                      perimeter_x0, perimeter_y0,
                      perimeter_x1, perimeter_y1,
                      perimeter_x2, perimeter_y2,
                      perimeter_x3, perimeter_y3,
                      thr_mono_img,
                      image_width, image_height,
                      curr_sampling_radius, curr_sampling_pattern,
                      &grid[try_config]);
          /* check that this looks vaguely like a datamatrix */
          unsigned char occupied_cells_percent =
            get_grid_occupancy_percent(&grid[try_config]);
          if ((occupied_cells_percent < MIN_OCCUPIED_CELLS_PERCENT) ||
              (occupied_cells_percent > MAX_OCCUPIED_CELLS_PERCENT)) {
            free_grid(&grid[try_config]);
            break;
          }
          if (debug == 1) {
            mono_to_colour(thr_mono_img, image_width, image_height,
                           image_bitsperpixel, thr_image_data);
            show_grid_image(&grid[try_config], thr_image_data,
                            image_width, image_height, image_bitsperpixel,
                            curr_sampling_radius, curr_sampling_pattern);
            show_grid(&grid[try_config]);
            sprintf(debug_filename[try_config],
                    "debug_%d_16_grid_sampling.png", try_config);
            write_png_file(debug_filename[try_config],
                           image_width, image_height, 24, thr_image_data);
          }
          datamatrix_decode(&grid[try_config], debug, gs1_url,
                            thr_decode_result[try_config],
                            human_readable);
          if (strlen(thr_decode_result[try_config]) > 0) {
            free_grid(&grid[try_config]);
            best_config = try_config;
            best_perimeter_x0 = perimeter_x0;
            best_perimeter_y0 = perimeter_y0;
            best_perimeter_x1 = perimeter_x1;
            best_perimeter_y1 = perimeter_y1;
            best_perimeter_x2 = perimeter_x2;
            best_perimeter_y2 = perimeter_y2;
            best_perimeter_x3 = perimeter_x3;
            best_perimeter_y3 = perimeter_y3;
            if (strlen(grid_filename) > 0) {
              /* show the grid */
              memcpy(image_data, original_image_data,
                     image_width*image_height*image_bytesperpixel);
              show_grid_image(&grid[try_config],
                              image_data,
                              image_width, image_height,
                              image_bitsperpixel,
                              0, SAMPLING_PATTERN_SOLID);
              write_png_file(grid_filename,
                             image_width, image_height, 24, image_data);
            }
            break;
          }
          /* try again with rotation */
          rotate_grid(&grid[try_config]);
          datamatrix_decode(&grid[try_config], debug,
                            gs1_url, thr_decode_result[try_config],
                            human_readable);
          if (strlen(thr_decode_result[try_config]) > 0) {
            best_config = try_config;
            best_perimeter_x0 = perimeter_x0;
            best_perimeter_y0 = perimeter_y0;
            best_perimeter_x1 = perimeter_x1;
            best_perimeter_y1 = perimeter_y1;
            best_perimeter_x2 = perimeter_x2;
            best_perimeter_y2 = perimeter_y2;
            best_perimeter_x3 = perimeter_x3;
            best_perimeter_y3 = perimeter_y3;
            if (strlen(grid_filename) > 0) {
              /* show the grid */
              memcpy(image_data, original_image_data,
                     image_width*image_height*image_bytesperpixel);
              show_grid_image(&grid[try_config],
                              image_data,
                              image_width, image_height,
                              image_bitsperpixel,
                              0, SAMPLING_PATTERN_SOLID);
              write_png_file(grid_filename,
                             image_width, image_height, 24, image_data);
            }
            free_grid(&grid[try_config]);
            break;
          }
          free_grid(&grid[try_config]);
        }
      }

      if ((any_decode(&thr_decode_result[0], max_config) == 1) ||
          (test_specific_config_settings == 1)) {
        if (verify == 1) {
          calculate_quality_metrics(&grid[try_config],
                                    original_image_data,
                                    thr_original_meanlight_image_data,
                                    image_width, image_height,
                                    image_bitsperpixel,
                                    histogram_module_centres,
                                    histogram_filename,
                                    cell_shape_filename);
        }
        free_line_segments(&segments[try_config]);
        free(thr_image_data);
        free(thr_meanlight_image_data);
        free(thr_original_meanlight_image_data);
        free(thr_mono_img);
        free(thr_binary_image);
        free(thr_buffer_img);
        free(thr_edges_image_data);
        continue;
      }

      /* if timing border frequency detection fails then try to decode using
         all possible grids */
      if (rectangular == 0) {
        int * valid_squares = get_valid_squares();
        for (int frequency_index = 0; frequency_index < NO_OF_VALID_SQUARES;
             frequency_index++) {
          most_probable_frequency = valid_squares[frequency_index];
          if ((most_probable_frequency < minimum_grid_dimension) ||
              (most_probable_frequency > maximum_grid_dimension)) continue;
          /* sample grid cells in different patterns */
          curr_sampling_radius = sampling_radius;
          for (int curr_sampling_pattern = SAMPLING_PATTERN_SOLID;
               curr_sampling_pattern <= SAMPLING_PATTERN_RING;
               curr_sampling_pattern++) {
            /* increase the radius for ring sampling */
            if (curr_sampling_pattern == SAMPLING_PATTERN_RING) curr_sampling_radius+=2;
            create_grid(most_probable_frequency, most_probable_frequency,
                        perimeter_x0, perimeter_y0,
                        perimeter_x1, perimeter_y1,
                        perimeter_x2, perimeter_y2,
                        perimeter_x3, perimeter_y3,
                        thr_mono_img,
                        image_width, image_height,
                        curr_sampling_radius, curr_sampling_pattern,
                        &grid[try_config]);
            /* check that this looks vaguely like a datamatrix */
            unsigned char occupied_cells_percent =
              get_grid_occupancy_percent(&grid[try_config]);
            if ((occupied_cells_percent < MIN_OCCUPIED_CELLS_PERCENT) ||
                (occupied_cells_percent > MAX_OCCUPIED_CELLS_PERCENT)) {
              free_grid(&grid[try_config]);
              break;
            }
            datamatrix_decode(&grid[try_config], debug,
                              gs1_url, thr_decode_result[try_config],
                              human_readable);
            if (strlen(thr_decode_result[try_config]) > 0) {
              free_grid(&grid[try_config]);
              best_config = try_config;
              best_perimeter_x0 = perimeter_x0;
              best_perimeter_y0 = perimeter_y0;
              best_perimeter_x1 = perimeter_x1;
              best_perimeter_y1 = perimeter_y1;
              best_perimeter_x2 = perimeter_x2;
              best_perimeter_y2 = perimeter_y2;
              best_perimeter_x3 = perimeter_x3;
              best_perimeter_y3 = perimeter_y3;
              if (strlen(grid_filename) > 0) {
                /* show the grid */
                memcpy(image_data, original_image_data,
                       image_width*image_height*image_bytesperpixel);
                show_grid_image(&grid[try_config],
                                image_data,
                                image_width, image_height,
                                image_bitsperpixel,
                                0, SAMPLING_PATTERN_SOLID);
                write_png_file(grid_filename,
                               image_width, image_height, 24, image_data);
              }
              if (debug == 1) {
                printf("Frequency: %d\n", most_probable_frequency);
                mono_to_colour(thr_mono_img, image_width, image_height,
                               image_bitsperpixel, thr_image_data);
                show_grid_image(&grid[try_config], thr_image_data,
                                image_width, image_height, image_bitsperpixel,
                                curr_sampling_radius, curr_sampling_pattern);
                sprintf(debug_filename[try_config],
                        "debug_%d_17_grid_sampling.png", try_config);
                write_png_file(debug_filename[try_config],
                               image_width, image_height, 24, thr_image_data);
              }
              break;
            }
            /* try again with rotated grid */
            rotate_grid(&grid[try_config]);
            datamatrix_decode(&grid[try_config], debug,
                              gs1_url, thr_decode_result[try_config],
                              human_readable);
            if (strlen(thr_decode_result[try_config]) > 0) {
              best_config = try_config;
              best_perimeter_x0 = perimeter_x0;
              best_perimeter_y0 = perimeter_y0;
              best_perimeter_x1 = perimeter_x1;
              best_perimeter_y1 = perimeter_y1;
              best_perimeter_x2 = perimeter_x2;
              best_perimeter_y2 = perimeter_y2;
              best_perimeter_x3 = perimeter_x3;
              best_perimeter_y3 = perimeter_y3;
              if (strlen(grid_filename) > 0) {
                /* show the grid */
                memcpy(image_data, original_image_data,
                       image_width*image_height*image_bytesperpixel);
                show_grid_image(&grid[try_config],
                                image_data,
                                image_width, image_height,
                                image_bitsperpixel,
                                0, SAMPLING_PATTERN_SOLID);
                write_png_file(grid_filename,
                               image_width, image_height, 24, image_data);
              }
              if (debug == 1) {
                printf("Frequency: %d\n", most_probable_frequency);
                mono_to_colour(thr_mono_img, image_width, image_height,
                               image_bitsperpixel, thr_image_data);
                show_grid_image(&grid[try_config], thr_image_data,
                                image_width, image_height, image_bitsperpixel,
                                curr_sampling_radius, curr_sampling_pattern);
                sprintf(debug_filename[try_config],
                        "debug_%d_17_grid_sampling.png", try_config);
                write_png_file(debug_filename[try_config],
                               image_width, image_height, 24, thr_image_data);
              }
              free_grid(&grid[try_config]);
              break;
            }
            free_grid(&grid[try_config]);
          }
          if (any_decode(&thr_decode_result[0], max_config) == 1) {
            /* decode achieved */
            break;
          }
        }
      }
      else {
        /* try all rectangles */
        int * valid_rectangles = get_valid_rectangles();
        for (int frequency_index = 0; frequency_index < NO_OF_VALID_RECTANGLES;
             frequency_index++) {
          most_probable_frequency = valid_rectangles[frequency_index*2];

          if (aspect_ratio_percent < 100) {
            most_probable_frequency_x = valid_rectangles[frequency_index*2];
            most_probable_frequency_y = valid_rectangles[frequency_index*2+1];
            most_probable_frequency = most_probable_frequency_y;
          }
          else {
            most_probable_frequency_x = valid_rectangles[frequency_index*2+1];
            most_probable_frequency_y = valid_rectangles[frequency_index*2];
            most_probable_frequency = most_probable_frequency_x;
          }
          if ((most_probable_frequency < minimum_grid_dimension) ||
              (most_probable_frequency > maximum_grid_dimension)) continue;
          /* sample grid cells in different patterns */
          curr_sampling_radius = sampling_radius;
          for (int curr_sampling_pattern = SAMPLING_PATTERN_SOLID;
               curr_sampling_pattern <= SAMPLING_PATTERN_RING;
               curr_sampling_pattern++) {
            /* increase the radius for ring sampling */
            if (curr_sampling_pattern == SAMPLING_PATTERN_RING) curr_sampling_radius+=2;
            create_grid(most_probable_frequency_x, most_probable_frequency_y,
                        perimeter_x0, perimeter_y0,
                        perimeter_x1, perimeter_y1,
                        perimeter_x2, perimeter_y2,
                        perimeter_x3, perimeter_y3,
                        thr_mono_img,
                        image_width, image_height,
                        curr_sampling_radius, curr_sampling_pattern,
                        &grid[try_config]);
            /* check that this looks vaguely like a datamatrix */
            unsigned char occupied_cells_percent =
              get_grid_occupancy_percent(&grid[try_config]);
            if ((occupied_cells_percent < MIN_OCCUPIED_CELLS_PERCENT) ||
                (occupied_cells_percent > MAX_OCCUPIED_CELLS_PERCENT)) {
              free_grid(&grid[try_config]);
              break;
            }
            datamatrix_decode(&grid[try_config], debug,
                              gs1_url, thr_decode_result[try_config],
                              human_readable);
            if (strlen(thr_decode_result[try_config]) > 0) {
              best_config = try_config;
              best_perimeter_x0 = perimeter_x0;
              best_perimeter_y0 = perimeter_y0;
              best_perimeter_x1 = perimeter_x1;
              best_perimeter_y1 = perimeter_y1;
              best_perimeter_x2 = perimeter_x2;
              best_perimeter_y2 = perimeter_y2;
              best_perimeter_x3 = perimeter_x3;
              best_perimeter_y3 = perimeter_y3;
              if (strlen(grid_filename) > 0) {
                /* show the grid */
                memcpy(image_data, original_image_data,
                       image_width*image_height*image_bytesperpixel);
                show_grid_image(&grid[try_config],
                                image_data,
                                image_width, image_height,
                                image_bitsperpixel,
                                0, SAMPLING_PATTERN_SOLID);
                write_png_file(grid_filename,
                               image_width, image_height, 24, image_data);
              }
              if (debug == 1) {
                printf("Frequency: %dx%d\n",
                       most_probable_frequency_x, most_probable_frequency_y);
                mono_to_colour(thr_mono_img, image_width, image_height,
                               image_bitsperpixel, thr_image_data);
                show_grid_image(&grid[try_config], thr_image_data,
                                image_width, image_height, image_bitsperpixel,
                                curr_sampling_radius, curr_sampling_pattern);
                sprintf(debug_filename[try_config],
                        "debug_%d_17_grid_sampling.png", try_config);
                write_png_file(debug_filename[try_config],
                               image_width, image_height, 24, thr_image_data);
              }
              free_grid(&grid[try_config]);
              break;
            }
            free_grid(&grid[try_config]);
          }
          if (any_decode(&thr_decode_result[0], max_config) == 1) {
            /* decode achieved */
            break;
          }
        }
      }
      if (any_decode(&thr_decode_result[0], max_config) == 1) {
        /* decode achieved */
        if (verify == 1) {
          calculate_quality_metrics(&grid[try_config],
                                    original_image_data,
                                    thr_original_meanlight_image_data,
                                    image_width, image_height,
                                    image_bitsperpixel,
                                    histogram_module_centres,
                                    histogram_filename,
                                    cell_shape_filename);
        }
        free_line_segments(&segments[try_config]);
        free(thr_image_data);
        free(thr_meanlight_image_data);
        free(thr_original_meanlight_image_data);
        free(thr_mono_img);
        free(thr_binary_image);
        free(thr_buffer_img);
        free(thr_edges_image_data);
        continue;
      }
    }

    free_line_segments(&segments[try_config]);

    /* quality metrics */
    if (strlen(thr_decode_result[try_config]) > 0) {
      if (verify == 1) {
        calculate_quality_metrics(&grid[try_config],
                                  original_image_data,
                                  thr_original_meanlight_image_data,
                                  image_width, image_height,
                                  image_bitsperpixel,
                                  histogram_module_centres,
                                  histogram_filename,
                                  cell_shape_filename);
      }
    }

    free(thr_image_data);
    free(thr_meanlight_image_data);
    free(thr_original_meanlight_image_data);
    free(thr_mono_img);
    free(thr_binary_image);
    free(thr_buffer_img);
    free(thr_edges_image_data);
  }

  if (best_config > -1) {
    if (strlen(thr_decode_result[best_config]) > 0) {
      /* -o option: output image showing perimeter */
      if ((strlen(output_filename) > 0) &&
          (best_perimeter_x0 > -1)) {
        memcpy(image_data, original_image_data,
               image_width*image_height*image_bytesperpixel);
        show_L_shape_perimeter(&grid[best_config],
                               image_data, image_width, image_height,
                               image_bitsperpixel,
                               best_perimeter_x0, best_perimeter_y0,
                               best_perimeter_x1, best_perimeter_y1,
                               best_perimeter_x2, best_perimeter_y2,
                               best_perimeter_x3, best_perimeter_y3);
        write_png_file(output_filename,
                       image_width, image_height, 24, image_data);
      }

      /* quality metrics */
      if (verify == 1) {
        show_quality_metrics(&grid[best_config], csv, json, yaml,
                             aperture, light_nm, light_angle_degrees);
        if ((strlen(report_template) > 0) &&
            (strlen(report_filename) > 0)) {
          save_verification_report(&grid[best_config],
                                   address_line1,
                                   address_line2,
                                   address_line3,
                                   phone,
                                   email,
                                   website,
                                   output_filename,
                                   histogram_filename,
                                   report_template,
                                   report_filename,
                                   logo_filename,
                                   thr_decode_result[best_config],
                                   aperture,
                                   light_nm,
                                   light_angle_degrees,
                                   footer);
        }
      }
      strcpy(decode_result, thr_decode_result[best_config]);
    }
  }

  /* free all the threaded decode results */
  for (try_config = 0; try_config < no_of_configs; try_config++) {
    free(thr_decode_result[try_config]);
    free(debug_filename[try_config]);
  }

  /* free allocated memory */
  free(original_image_data);
  if (strlen(decode_result) > 0) {
    return 0;
  }
  return -1;
}
