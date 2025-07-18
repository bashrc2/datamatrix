/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - example computer vision functions
 *  Copyright (c) 2011-2015, Bob Mottram
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

int main(int argc, char* argv[])
{
  int i;
  unsigned char * image_data = NULL;
  unsigned char * resized_image_data = NULL;
  unsigned int image_width=0;
  unsigned int image_height=0;
  unsigned int resized_image_width=800;
  unsigned int resized_image_height=0;
  int resized_thresholded_width = 150;
  unsigned int image_bitsperpixel=0;
  unsigned char debug = 0;
  unsigned char csv = 0;
  unsigned char json = 0;
  unsigned char yaml = 0;
  int minimum_grid_dimension = MIN_GRID_DIMENSION;
  int maximum_grid_dimension = MAX_GRID_DIMENSION;
  int test_ml_threshold = 0;
  int test_erode = 0;
  int test_dilate = 0;
  int test_frequency = 0;
  unsigned char verify = 0;
  unsigned char raw_decode = 0;
  unsigned char histogram_module_centres = 0;
  int loop_incr = 2;
  int sampling_radius = 1;
  int min_segment_length = 40;
  char gs1_url[MAX_DECODE_LENGTH];
  char filename[MAX_DECODE_LENGTH];
  char output_filename[MAX_DECODE_LENGTH];
  char grid_filename[MAX_DECODE_LENGTH];
  char histogram_filename[MAX_DECODE_LENGTH];
  char cell_shape_filename[MAX_DECODE_LENGTH];
  char report_template[MAX_DECODE_LENGTH];
  char report_filename[MAX_DECODE_LENGTH];
  char logo_filename[MAX_DECODE_LENGTH];
  char address_line1[MAX_DECODE_LENGTH];
  char address_line2[MAX_DECODE_LENGTH];
  char address_line3[MAX_DECODE_LENGTH];
  char phone[MAX_DECODE_LENGTH];
  char email[MAX_DECODE_LENGTH];
  char website[MAX_DECODE_LENGTH];
  char footer[MAX_DECODE_LENGTH];
  float aperture = 0;
  int light_nm = 670;
  int light_angle_degrees = 90;

  /* only looks for squares or rectangles */
  unsigned char is_square = 0;
  unsigned char is_rectangle = 0;

  /* no filename specified */
  filename[0] = 0;

  /* no verification report filename specified */
  report_filename[0] = 0;

  /* logo which appears on verification report */
  logo_filename[0] = 0;

  /* footer text on verification report */
  footer[0] = 0;

  /* contact details to appear on verification report */
  address_line1[0] = 0;
  address_line2[0] = 0;
  address_line3[0] = 0;
  phone[0] = 0;
  email[0] = 0;
  website[0] = 0;
  decode_strcat(&website[0], "https://gitlab.com/bashrc2/datamatrix");

  /* default verification report template */
  report_template[0] = 0;
  decode_strcat(&report_template[0], "report_template.tex");

  /* no output or grid image by default */
  output_filename[0] = 0;
  grid_filename[0] = 0;
  histogram_filename[0] = 0;
  cell_shape_filename[0] = 0;

  /* no GS1 digital link specified */
  gs1_url[0] = 0;

  for (i = 1; i < argc; i += loop_incr) {
    loop_incr = 2;
    if ((strcmp(argv[i],"-f")==0) ||
        (strcmp(argv[i],"--filename")==0)) {
      filename[0] = 0;
      decode_strcat(&filename[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"-r")==0) ||
        (strcmp(argv[i],"--report")==0)) {
      report_filename[0] = 0;
      decode_strcat(&report_filename[0], argv[i+1]);
      verify = 1;
      histogram_module_centres = 0;
    }
    if ((strcmp(argv[i],"-l")==0) ||
        (strcmp(argv[i],"--logo")==0)) {
      logo_filename[0] = 0;
      decode_strcat(&logo_filename[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"-t")==0) ||
        (strcmp(argv[i],"--template")==0)) {
      report_template[0] = 0;
      decode_strcat(&report_template[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--footer")==0) {
      footer[0] = 0;
      decode_strcat(&footer[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--address1")==0) {
      address_line1[0] = 0;
      decode_strcat(&address_line1[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--address2")==0) {
      address_line2[0] = 0;
      decode_strcat(&address_line2[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--address3")==0) {
      address_line3[0] = 0;
      decode_strcat(&address_line3[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"--phone")==0) ||
        (strcmp(argv[i],"--tel")==0)) {
      phone[0] = 0;
      decode_strcat(&phone[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--email")==0) {
      email[0] = 0;
      decode_strcat(&email[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"--website")==0) ||
        (strcmp(argv[i],"--web")==0)) {
      website[0] = 0;
      decode_strcat(&website[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--aperture")==0) {
      aperture = atof(argv[i+1]);
    }
    if (strcmp(argv[i],"--light")==0) {
      light_nm = atoi(argv[i+1]);
    }
    if (strcmp(argv[i],"--angle")==0) {
      light_angle_degrees = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--freq")==0) ||
        (strcmp(argv[i],"--frequency")==0)) {
      test_frequency = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--erode")==0) ||
        (strcmp(argv[i],"--erosion")==0)) {
      test_erode = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--dilate")==0) ||
        (strcmp(argv[i],"--dilation")==0)) {
      test_dilate = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--ml")==0) ||
        (strcmp(argv[i],"--meanlight")==0)) {
      test_ml_threshold = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--sample")==0) ||
        (strcmp(argv[i],"--samplingradius")==0) ||
        (strcmp(argv[i],"--sampleradius")==0)) {
      sampling_radius = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--minsegmentlength")==0) ||
        (strcmp(argv[i],"--minsegment")==0)) {
      min_segment_length = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"-o")==0) ||
        (strcmp(argv[i],"--output")==0)) {
      decode_strcat(&output_filename[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"--hist")==0) ||
        (strcmp(argv[i],"--histogram")==0)) {
      verify = 1;
      decode_strcat(&histogram_filename[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"--shape")==0) ||
        (strcmp(argv[i],"--cellshape")==0)) {
      verify = 1;
      decode_strcat(&cell_shape_filename[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"-g")==0) ||
        (strcmp(argv[i],"--grid")==0)) {
      decode_strcat(&grid_filename[0], argv[i+1]);
    }
    if ((strcmp(argv[i],"--url")==0) ||
        (strcmp(argv[i],"--resolver")==0) ||
        (strcmp(argv[i],"--gs1link")==0) ||
        (strcmp(argv[i],"--link")==0)) {
      decode_strcat(&gs1_url[0], argv[i+1]);
    }
    if (strcmp(argv[i],"--mingrid")==0) {
      minimum_grid_dimension = atoi(argv[i+1]);
      if (minimum_grid_dimension < MIN_GRID_DIMENSION) {
        minimum_grid_dimension = MIN_GRID_DIMENSION;
      }
    }
    if (strcmp(argv[i],"--maxgrid")==0) {
      maximum_grid_dimension = atoi(argv[i+1]);
      if (maximum_grid_dimension > MAX_GRID_DIMENSION) {
        maximum_grid_dimension = MAX_GRID_DIMENSION;
      }
    }
    if (strcmp(argv[i],"--resizewidth")==0) {
      resized_image_width = atoi(argv[i+1]);
    }
    if (strcmp(argv[i],"--resizeheight")==0) {
      resized_image_height = atoi(argv[i+1]);
    }
    if ((strcmp(argv[i],"--thresholdedwidth")==0) ||
        (strcmp(argv[i],"--binwidth")==0)) {
      resized_thresholded_width = atoi(argv[i+1]);
    }
    if (strcmp(argv[i],"--raw")==0) {
      raw_decode = 1;
      loop_incr = 1;
    }
    if (strcmp(argv[i],"--csv")==0) {
      verify = 1;
      csv = 1;
      json = 0;
      yaml = 0;
      loop_incr = 1;
    }
    if (strcmp(argv[i],"--json")==0) {
      verify = 1;
      csv = 0;
      json = 1;
      yaml = 0;
      loop_incr = 1;
    }
    if ((strcmp(argv[i],"--yaml")==0) ||
        (strcmp(argv[i],"--yml")==0)) {
      verify = 1;
      csv = 0;
      json = 0;
      yaml = 1;
      loop_incr = 1;
    }
    if (strcmp(argv[i],"--debug")==0) {
      debug = 1;
      loop_incr = 1;
    }
    if (strcmp(argv[i],"--square")==0) {
      is_square = 1;
      loop_incr = 1;
    }
    if ((strcmp(argv[i],"--rectangle")==0) ||
        (strcmp(argv[i],"--rectangular")==0)) {
      is_rectangle = 1;
      loop_incr = 1;
    }
    if ((strcmp(argv[i],"-h")==0) ||
        (strcmp(argv[i],"--help")==0)) {
      show_help();
      return 0;
    }
    if ((strcmp(argv[i],"--histcentres")==0) ||
        (strcmp(argv[i],"--histcenters")==0) ||
        (strcmp(argv[i],"--modulecentres")==0) ||
        (strcmp(argv[i],"--modulecenters")==0)) {
      histogram_module_centres = 1;
      loop_incr = 1;
    }
    if ((strcmp(argv[i],"--verify")==0) ||
        (strcmp(argv[i],"--quality")==0) ||
        (strcmp(argv[i],"--verification")==0)) {
      verify = 1;
      loop_incr = 1;
    }
    if (strcmp(argv[i],"--tests")==0) {
      run_all_tests();
      return 0;
    }
  }

  /* was a file specified */
  if (strlen(&filename[0]) == 0) {
    printf("No image file specified\n");
    return 0;
  }

  /* if a verification report is requested then ensure that an output image is produced */
  if (report_filename[0] != 0) {
    if (output_filename[0] == 0) {
      decode_strcat(&output_filename[0], "report.png");
    }
    if (histogram_filename[0] == 0) {
      decode_strcat(&histogram_filename[0], "report_histogram.png");
    }
  }

  image_data = read_png_file(&filename[0], &image_width, &image_height, &image_bitsperpixel);
  if (image_data == NULL) {
    printf("Couldn't load image %s\n", &filename[0]);
    return 0;
  }
  if ((image_width == 0) || (image_height==0)) {
    printf("Couldn't load image size %dx%d\n", image_width, image_height);
    return 0;
  }
  if (image_bitsperpixel == 0) {
    printf("Couldn't load image depth\n");
    return 0;
  }

  if (debug == 1) {
    printf("Image: %s\n", &filename[0]);
    printf("Resolution: %dx%d\n", image_width, image_height);
    printf("Depth: %d\n", image_bitsperpixel);
  }

  if (resized_image_width > 0) {
    if (resized_image_height == 0) {
      resized_image_height = image_height * resized_image_width / image_width;
    }
    /* create an array to store the enlarged image */
    resized_image_data =
      (unsigned char*)safemalloc(resized_image_width*resized_image_height*
                             image_bitsperpixel/8);
    if (resized_image_data == NULL) return -653;
    /* resize the image */
    if (resize_image(image_data, image_width, image_height,
                     image_bitsperpixel,
                     resized_image_data,
                     resized_image_width, resized_image_height) != 0) {
      printf("Failed to resize\n");
      return -724;
    }
    /* resized image becomes the output */
    free(image_data);
    image_data = resized_image_data;
    image_width = resized_image_width;
    image_height = resized_image_height;
    resized_image_data = NULL;

    if (debug == 1) {
      write_png_file("debug_01_resize.png", image_width, image_height, 24, image_data);
      printf("Resized resolution: %dx%d\n", image_width, image_height);
    }
  }


  char * decode_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  read_datamatrix(image_data, image_width, image_height,
                  image_bitsperpixel, debug,
                  &output_filename[0],
                  &grid_filename[0],
                  test_ml_threshold,
                  test_erode,
                  test_dilate,
                  test_frequency,
                  verify, csv, json, yaml,
                  minimum_grid_dimension,
                  maximum_grid_dimension,
                  &gs1_url[0], raw_decode,
                  histogram_module_centres,
                  &histogram_filename[0],
                  resized_thresholded_width,
                  sampling_radius,
                  min_segment_length,
                  aperture, light_nm, light_angle_degrees,
                  is_square, is_rectangle,
                  cell_shape_filename,
                  &report_template[0],
                  &report_filename[0],
                  &logo_filename[0],
                  &address_line1[0],
                  &address_line2[0],
                  &address_line3[0],
                  &phone[0],
                  &email[0],
                  &website[0],
                  &footer[0],
                  decode_result);
  if (strlen(decode_result) > 0) {
    if (verify == 0) {
      printf("%s\n", decode_result);
    }
    else {
      if ((csv == 0) && (json == 0) && (yaml == 0)) {
        printf("Decode: PASS (%s)\n", decode_result);
      }
      else if (csv == 1) {
        printf("\"PASS\", \"%s\",\n", decode_result);
      }
      else if (json == 1) {
        printf("  \"decode\": { \"grade\": \"PASS\", \"text\": \"%s\" }\n", decode_result);
        printf("}\n");
      }
      else if (yaml == 1) {
        printf("\n# Result\n");
        printf("decode:\n- grade: PASS\n  text: %s\n", decode_result);
        printf("---\n");
      }
    }
  }

  free(decode_result);
  return 0;
}
