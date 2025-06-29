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
#include "png2.h"
#include "datamatrix.h"

/**
 * \brief read a datamatrix from an image
 * \param image_data image array
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_bitsperpixel Number of bits per pixel
 * \param debug set to 1 if in debug mode
 * \param output_filename array containing output image with detected datamatrix
 * \param test_ml_threshold test with a specific mean light value
 * \param test_erode test with a specific erosion value
 * \param test_dilate test with a specific dilation value
 * \param test_edge_threshold test with a specific edge threshold
 * \param test_frequency test using a known grid dimension
 * \param verify set to 1 if symbol quality metrics are to be calculated
 * \param minimum_grid_dimension minimum grid dimension
 * \param maximum_grid_dimension maximum grid dimension
 * \param decode_result returned decode text
 * \return zero on decode success, -1 otherwise
 */
int read_datamatrix(unsigned char image_data[],
                    int image_width, int image_height,
                    int image_bitsperpixel,
                    unsigned char debug,
                    char output_filename[],
                    int test_ml_threshold,
                    int test_erode, int test_dilate,
                    float test_edge_threshold,
                    int test_frequency,
                    unsigned char verify,
                    int minimum_grid_dimension,
                    int maximum_grid_dimension,
                    char * decode_result)
{
  struct line_segments segments;
  struct grid_2d grid;
  int original_image_width = image_width;
  int original_image_height = image_height;
  int image_bytesperpixel = image_bitsperpixel/8;
  int try_config;
  float edge_radius = 25;
  int min_segment_length=20;
  int segment_join_radius=6;
  const int no_of_configs = 6;

  /* the magic numbers */
  int ml_threshold_configs[] = {0, 0, 50, 10, 10, 20};
  int erosion_itterations_configs[] = {1, 0, 1, 0, 0, 0};
  int dilate_itterations_configs[] = {9, 5, 4, 6, 5, 2};
  float edge_threshold_configs[] = {5, 1, 1, 1, 5, 5};

  int resized_thresholded_width = 320;
  int resized_thresholded_height = image_height * resized_thresholded_width / image_width;
  int sampling_radius = 1;
  int most_probable_frequency = 0;
  int aspect_ratio_percent;
  unsigned char test_specific_config_settings = 0;
  int timing_pattern_sampling_radius = 1;
  int curr_sampling_radius;
  float corner_radians, angle_degrees;

  unsigned char * original_image_data =
    (unsigned char*)malloc(image_width*image_height*image_bytesperpixel);
  unsigned char * meanlight_image_data =
    (unsigned char*)malloc(image_width*image_height*sizeof(unsigned char));
  unsigned char * original_thresholded_image_data =
    (unsigned char*)malloc(image_width*image_height*image_bytesperpixel);
  unsigned char * thresholded =
    (unsigned char*)malloc(resized_thresholded_width*resized_thresholded_height);
  unsigned char * mono_img = (unsigned char*)malloc(image_width*image_height);
  unsigned char * buffer_img = (unsigned char*)malloc(image_width*image_height);
  unsigned char * resized_image_data =
    (unsigned char*)malloc(resized_thresholded_width*resized_thresholded_height*
                           image_bytesperpixel);

  decode_result[0] = 0;

  if (meanlight_image_data == NULL) {
    printf("Unable to create meanlight buffer\n");
    return -2;
  }
  if (original_thresholded_image_data == NULL) {
    printf("Unable to create original thresholded image data\n");
    return -3;
  }
  if (mono_img == NULL) {
    printf("Unable to create mono image data\n");
    return -4;
  }
  if (buffer_img == NULL) {
    printf("Unable to create buffer image data\n");
    return -5;
  }
  if (resized_image_data == NULL) {
    printf("Unable to create resized image data\n");
    return -6;
  }
  if (original_image_data == NULL) {
    printf("Unable to create original image data\n");
    return -7;
  }
  if (thresholded == NULL) {
    printf("Unable to create thresholded image data\n");
    return -8;
  }

  /* keep a copy of the original image */
  memcpy(original_image_data, image_data, image_width*image_height*image_bytesperpixel);

  /* try a few different configurations of erosion/dilation, binary threshold
     and edge threshold */
  for (try_config = 0; try_config < no_of_configs; try_config++) {
    int ml_threshold = ml_threshold_configs[try_config];
    int erosion_itterations = erosion_itterations_configs[try_config];
    int dilate_itterations = dilate_itterations_configs[try_config];
    float edge_threshold = edge_threshold_configs[try_config];

    if (try_config > 0) {
      /* restore the image data after failed try */
      memcpy(image_data, original_image_data, image_width*image_height*image_bytesperpixel);
    }

    if ((test_ml_threshold > 0) ||
        (test_erode > 0) || (test_dilate > 0) ||
        (test_edge_threshold > 0)) {
      /* if we are only trying one combination of settings */
      test_specific_config_settings = 1;
      ml_threshold = test_ml_threshold;
      erosion_itterations = test_erode;
      dilate_itterations = test_dilate;
      edge_threshold = test_edge_threshold;
    }


    /* convert to meanlight */
    meanlight_threshold(image_data, image_width, image_height,
                        image_bitsperpixel, ml_threshold,
                        meanlight_image_data);
    if (debug == 1) {
      write_png_file("debug_02_meanlight.png", image_width, image_height, 24, image_data);
    }
    /* keep a copy of the thresholded data for subsequent decoding */
    memcpy(original_thresholded_image_data, image_data, image_width*image_height*image_bitsperpixel/8);


    /* erosion or dilation */
    if ((erosion_itterations > 0) || (dilate_itterations > 0)) {
      /* make a mono image with one byte per pixel */
      colour_to_mono(image_data,
                     image_width,image_height,image_bitsperpixel,mono_img);
      /* apply morphology to the mono image */
      if (erosion_itterations > 0) {
        erode(mono_img, image_width, image_height,
              buffer_img,
              erosion_itterations,
              mono_img);
      }
      if (dilate_itterations > 0) {
        dilate(mono_img, image_width, image_height,
               buffer_img,
               dilate_itterations,
               mono_img);
      }
      /* convert the mono image back to colour */
      mono_to_colour(mono_img, image_width, image_height,
                     image_bitsperpixel, image_data);
      if (debug == 1) {
        write_png_file("debug_03_erode_dilate.png",
                       image_width, image_height, 24, image_data);
      }
    }

    /* resize the image */
    if (resize_thresholded_image(image_data, image_width, image_height,
                                 image_bitsperpixel,
                                 resized_image_data,
                                 resized_thresholded_width,
                                 resized_thresholded_height) != 0) {
      printf("Failed to resize thresholded image\n");
      return -724;
    }
    if (debug == 1) {
      write_png_file("debug_04_thresholded.png",
                     resized_thresholded_width, resized_thresholded_height, 24,
                     resized_image_data);
    }

    if (edge_threshold > 0) {
      unsigned char perimeter_found = 0;
      float perimeter_x0=0, perimeter_y0=0;
      float perimeter_x1=0, perimeter_y1=0;
      float perimeter_x2=0, perimeter_y2=0;
      float perimeter_x3=0, perimeter_y3=0;

      colour_to_mono(resized_image_data,
                     resized_thresholded_width,resized_thresholded_height,
                     image_bitsperpixel,thresholded);

      detect_edges(thresholded,resized_thresholded_width,resized_thresholded_height,
                   edge_threshold, edge_radius);

      /* convert the mono image back to colour */
      mono_to_colour(thresholded, resized_thresholded_width, resized_thresholded_height,
                     image_bitsperpixel, resized_image_data);
      if (debug == 1) {
        write_png_file("debug_05_edges.png", resized_thresholded_width,
                       resized_thresholded_height, 24, resized_image_data);
      }

      get_line_segments(thresholded, resized_thresholded_width,
                        resized_thresholded_height, &segments, min_segment_length);

      if (debug == 1) {
        show_line_segments(&segments, resized_image_data,
                           resized_thresholded_width, resized_thresholded_height,
                           image_bitsperpixel);
        write_png_file("debug_06a_line_segments.png",
                       resized_thresholded_width, resized_thresholded_height,
                       24, resized_image_data);
      }

      segment_edges_within_roi(&segments, resized_thresholded_width,
                               resized_thresholded_height, 50);

      if (debug == 1) {
        show_line_segments(&segments, resized_image_data,
                           resized_thresholded_width, resized_thresholded_height,
                           image_bitsperpixel);
        write_png_file("debug_06b_line_segments_with_roi.png",
                       resized_thresholded_width, resized_thresholded_height,
                       24, resized_image_data);
      }

      join_line_segments(&segments, segment_join_radius);

      if (debug == 1) {
        show_line_segments(&segments, resized_image_data,
                           resized_thresholded_width, resized_thresholded_height,
                           image_bitsperpixel);
        write_png_file("debug_06c_joined_line_segments.png",
                       resized_thresholded_width, resized_thresholded_height,
                       24, resized_image_data);
      }

      if (debug == 1) {
        show_square_line_segments(&segments, resized_image_data,
                                  resized_thresholded_width,
                                  resized_thresholded_height, image_bitsperpixel);
        write_png_file("debug_07_square_joined_line_segments.png",
                       resized_thresholded_width, resized_thresholded_height, 24,
                       resized_image_data);
        show_rectangular_line_segments(&segments, resized_image_data,
                                       resized_thresholded_width,
                                       resized_thresholded_height, image_bitsperpixel);
        write_png_file("debug_08_rectangular_joined_line_segments.png",
                       resized_thresholded_width, resized_thresholded_height, 24,
                       resized_image_data);
      }

      if (segments.no_of_segments == 0) {
        free_line_segments(&segments);
        return 1;
      }

      /* this must count upwards */
      for (int seg_idx = 0; seg_idx < segments.no_of_segments; seg_idx++) {
        int segment_index = segments.joins_sorted[seg_idx];
        get_peripheral_edges(&segments, segment_index, resized_thresholded_width, resized_thresholded_height);

        int quantization_degrees = 5;
        get_segments_orientation(&segments, resized_thresholded_width, resized_thresholded_height,
                                 quantization_degrees);

        if (fit_perimeter_to_sides(&segments, resized_thresholded_width, resized_thresholded_height,
                                   &perimeter_x0, &perimeter_y0,
                                   &perimeter_x1, &perimeter_y1,
                                   &perimeter_x2, &perimeter_y2,
                                   &perimeter_x3, &perimeter_y3) != 0) {
          show_perimeter(&segments, resized_image_data, resized_thresholded_width, resized_thresholded_height, image_bitsperpixel);
          continue;
        }

        aspect_ratio_percent =
          get_shape_aspect_ratio(perimeter_x0, perimeter_y0,
                                 perimeter_x1, perimeter_y1,
                                 perimeter_x2, perimeter_y2,
                                 perimeter_x3, perimeter_y3);
        /* TODO allow for rectangular aspects */
        if ((aspect_ratio_percent < 90) || (aspect_ratio_percent > 110)) {
          continue;
        }

        /* check that the corners are approximately square */
        /* first corner */
        corner_radians = corner_angle(perimeter_x0, perimeter_y0,
                                      perimeter_x1, perimeter_y1,
                                      perimeter_x2, perimeter_y2);
        if (corner_radians < 0) corner_radians = -corner_radians;
        if (corner_radians > PI)
          corner_radians = (2 * (float)PI) - corner_radians;
        angle_degrees = corner_radians / (float)PI * 180;
        if ((angle_degrees < 70) || (angle_degrees > 110)) continue;

        /* second corner */
        corner_radians = corner_angle(perimeter_x1, perimeter_y1,
                                      perimeter_x2, perimeter_y2,
                                      perimeter_x3, perimeter_y3);
        if (corner_radians < 0) corner_radians = -corner_radians;
        if (corner_radians > PI)
          corner_radians = (2 * (float)PI) - corner_radians;
        angle_degrees = corner_radians / (float)PI * 180;
        if ((angle_degrees < 70) || (angle_degrees > 110)) continue;

        /* shrink the perimeter according to the amount of dilation */
        shrinkwrap_shape_perimeter(erosion_itterations, dilate_itterations,
                                   &perimeter_x0, &perimeter_y0,
                                   &perimeter_x1, &perimeter_y1,
                                   &perimeter_x2, &perimeter_y2,
                                   &perimeter_x3, &perimeter_y3);

        if (debug == 1) {
          show_peripheral_edges(&segments, resized_image_data, resized_thresholded_width, resized_thresholded_height,
                                image_bitsperpixel);
          write_png_file("debug_09_peripheral_edges.png", resized_thresholded_width, resized_thresholded_height,
                         24, resized_image_data);
        }
        if (debug == 1) {
          show_perimeter(&segments, resized_image_data, resized_thresholded_width, resized_thresholded_height,
                         image_bitsperpixel);
          write_png_file("debug_10_perimeter.png", resized_thresholded_width, resized_thresholded_height,
                         24, resized_image_data);
        }

        if (debug == 1) {
          show_shape_perimeter(&segments,
                               resized_image_data, resized_thresholded_width, resized_thresholded_height,
                               image_bitsperpixel,
                               perimeter_x0, perimeter_y0,
                               perimeter_x1, perimeter_y1,
                               perimeter_x2, perimeter_y2,
                               perimeter_x3, perimeter_y3);
          write_png_file("debug_11_shape_perimeter_small.png", resized_thresholded_width, resized_thresholded_height, 24, resized_image_data);
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

        memcpy(image_data, original_image_data, image_width*image_height*image_bytesperpixel);

        show_shape_perimeter(&segments,
                             image_data, image_width, image_height,
                             image_bitsperpixel,
                             perimeter_x0, perimeter_y0,
                             perimeter_x1, perimeter_y1,
                             perimeter_x2, perimeter_y2,
                             perimeter_x3, perimeter_y3);
        if (debug == 1) {
          write_png_file("debug_12_shape_perimeter.png",
                         image_width, image_height, 24, image_data);
        }
        perimeter_found = 1;
        break;
      }

      /* we have a perimeter, now find the timing border frequency */
      if (perimeter_found == 1) {
        colour_to_mono(original_thresholded_image_data,
                       image_width,image_height,image_bitsperpixel,mono_img);

        if (debug == 1) {
          memcpy(image_data, original_thresholded_image_data,
                 image_width*image_height*image_bytesperpixel);
        }

        /* expand the perimeter */
        if (expand_perimeter_sides(mono_img, image_width, image_height,
                                   &perimeter_x0, &perimeter_y0,
                                   &perimeter_x1, &perimeter_y1,
                                   &perimeter_x2, &perimeter_y2,
                                   &perimeter_x3, &perimeter_y3,
                                   120, debug, image_data,
                                   image_bitsperpixel) == 1) {
          if ((debug == 1) || (strlen(output_filename) > 0)) {
            show_shape_perimeter(&segments,
                                 image_data, image_width, image_height,
                                 image_bitsperpixel,
                                 perimeter_x0, perimeter_y0,
                                 perimeter_x1, perimeter_y1,
                                 perimeter_x2, perimeter_y2,
                                 perimeter_x3, perimeter_y3);
          }
          if (debug == 1) {
            write_png_file("debug_13_expand_perimeter_sides.png",
                           image_width, image_height, 24, image_data);
          }
        }

        if (debug == 1) {
          memcpy(image_data, original_thresholded_image_data,
                 image_width*image_height*image_bytesperpixel);
        }

        /* contract the perimeter */
        if (contract_perimeter_sides(mono_img, image_width, image_height,
                                     &perimeter_x0, &perimeter_y0,
                                     &perimeter_x1, &perimeter_y1,
                                     &perimeter_x2, &perimeter_y2,
                                     &perimeter_x3, &perimeter_y3,
                                     20, debug, image_data,
                                     image_bitsperpixel) == 1) {
          if (debug == 1) {
            show_shape_perimeter(&segments,
                                 image_data, image_width, image_height,
                                 image_bitsperpixel,
                                 perimeter_x0, perimeter_y0,
                                 perimeter_x1, perimeter_y1,
                                 perimeter_x2, perimeter_y2,
                                 perimeter_x3, perimeter_y3);
            write_png_file("debug_14_contract_perimeter_sides.png",
                           image_width, image_height, 24, image_data);
          }
        }

        /* -o option: output image showing perimeter */
        if (strlen(output_filename) > 0) {
          memcpy(image_data, original_image_data,
                 image_width*image_height*image_bytesperpixel);
          show_shape_perimeter(&segments,
                               image_data, image_width, image_height,
                               image_bitsperpixel,
                               perimeter_x0, perimeter_y0,
                               perimeter_x1, perimeter_y1,
                               perimeter_x2, perimeter_y2,
                               perimeter_x3, perimeter_y3);
          write_png_file(output_filename,
                         image_width, image_height, 24, image_data);
        }

        /* find the timing border frequency */
        if (test_frequency > 0) {
          most_probable_frequency = test_frequency;
        }
        else {
          most_probable_frequency =
            detect_timing_pattern(mono_img, image_width, image_height,
                                  minimum_grid_dimension,
                                  maximum_grid_dimension,
                                  perimeter_x0, perimeter_y0,
                                  perimeter_x1, perimeter_y1,
                                  perimeter_x2, perimeter_y2,
                                  perimeter_x3, perimeter_y3,
                                  timing_pattern_sampling_radius,
                                  0, image_data, 0);
        }
        if ((most_probable_frequency > 0) && (debug == 1)) {
          memcpy(image_data, original_thresholded_image_data,
                 image_width*image_height*(image_bitsperpixel/8));
          detect_timing_pattern(mono_img, image_width, image_height,
                                minimum_grid_dimension,
                                maximum_grid_dimension,
                                perimeter_x0, perimeter_y0,
                                perimeter_x1, perimeter_y1,
                                perimeter_x2, perimeter_y2,
                                perimeter_x3, perimeter_y3, 1,
                                debug, image_data,
                                most_probable_frequency);
          write_png_file("debug_15_detect_timing_pattern.png",
                         image_width, image_height, 24, image_data);
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
                        mono_img,
                        image_width, image_height,
                        curr_sampling_radius, curr_sampling_pattern,
                        &grid);
            if (debug == 1) {
              mono_to_colour(mono_img, image_width, image_height,
                             image_bitsperpixel, image_data);
              show_grid_image(&grid, image_data, image_width, image_height, image_bitsperpixel, curr_sampling_radius, curr_sampling_pattern);
              show_grid(&grid);
              write_png_file("debug_16_grid_sampling.png", image_width, image_height, 24, image_data);
            }
            datamatrix_decode(&grid, debug, decode_result);
            free_grid(&grid);
            if (strlen(decode_result) > 0) {
              break;
            }
          }
        }

        if (strlen(decode_result) > 0) {
          /* decode achieved */
          break;
        }

        if (test_specific_config_settings == 1) continue;

        /* if timing border frequency detection fails then try to decode using
           all possible grids */
        int no_of_valid_squares = 24;
        int IEC16022_valid_squares[] = {
          10,  12,  14,  16,  18,  20,  22,  24,  26,  32, 36,  40,  44,  48,
          52,  64,  72,  80,  88,  96, 104, 120, 132, 144
        };
        for (int frequency_index = 0; frequency_index < no_of_valid_squares;
             frequency_index++) {
          most_probable_frequency = IEC16022_valid_squares[frequency_index];
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
                        mono_img,
                        image_width, image_height,
                        curr_sampling_radius, curr_sampling_pattern,
                        &grid);
            datamatrix_decode(&grid, debug, decode_result);
            free_grid(&grid);
            if (strlen(decode_result) > 0) {
              if (debug == 1) {
                printf("Frequency: %d\n", most_probable_frequency);
                mono_to_colour(mono_img, image_width, image_height,
                               image_bitsperpixel, image_data);
                show_grid_image(&grid, image_data, image_width, image_height, image_bitsperpixel, curr_sampling_radius, curr_sampling_pattern);
                write_png_file("debug_17_grid_sampling.png", image_width, image_height, 24, image_data);
              }
              break;
            }
          }
        }
        if (strlen(decode_result) > 0) {
          break;
        }

      }

      free_line_segments(&segments);
    }

    /* if we are only trying one combination of settings
       then exit the try loop */
    if (test_specific_config_settings == 1) break;
  }

  /* quality metrics */
  if ((verify == 1) && (strlen(decode_result) > 0)) {
    calculate_quality_metrics(&grid,
                              original_image_data,
                              original_thresholded_image_data,
                              image_width, image_height,
                              image_bitsperpixel);
    show_quality_metrics(&grid);
  }

  /* free allocated memory */
  free(original_image_data);
  free(meanlight_image_data);
  free(original_thresholded_image_data);
  free(mono_img);
  free(buffer_img);
  free(resized_image_data);
  free(thresholded);
  if (strlen(decode_result) > 0) {
    return 0;
  }
  return -1;
}
