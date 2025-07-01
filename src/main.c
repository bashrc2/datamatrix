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
  char * filename = NULL;
  char output_filename[255];
  unsigned char * image_data = NULL;
  unsigned char * resized_image_data = NULL;
  unsigned int image_width=0;
  unsigned int image_height=0;
  unsigned int resized_image_width=800;
  unsigned int resized_image_height=0;
  unsigned int image_bitsperpixel=0;
  unsigned char debug = 0;
  unsigned char csv = 0;
  unsigned char json = 0;
  int minimum_grid_dimension = MIN_GRID_DIMENSION;
  int maximum_grid_dimension = MAX_GRID_DIMENSION;
  int test_ml_threshold = 0;
  int test_erode = 0;
  int test_dilate = 0;
  float test_edge_threshold = 0;
  int test_frequency = 0;
  unsigned char verify = 0;

  /* no output image by default */
  output_filename[0] = 0;

  for (i=1;i<argc;i+=2) {
    if ((strcmp(argv[i],"-f")==0) ||
        (strcmp(argv[i],"--filename")==0)) {
      filename = argv[i+1];
    }
    if ((strcmp(argv[i],"-e")==0) ||
        (strcmp(argv[i],"--edgethresh")==0)) {
      test_edge_threshold = atof(argv[i+1]);
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
    if ((strcmp(argv[i],"-o")==0) ||
        (strcmp(argv[i],"--output")==0)) {
      sprintf((char*)output_filename,"%s",argv[i+1]);
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
    if (strcmp(argv[i],"--csv")==0) {
      csv = 1;
      i--;
    }
    if (strcmp(argv[i],"--json")==0) {
      json = 1;
      i--;
    }
    if (strcmp(argv[i],"--debug")==0) {
      debug = 1;
      i--;
    }
    if ((strcmp(argv[i],"--verify")==0) ||
        (strcmp(argv[i],"--quality")==0) ||
        (strcmp(argv[i],"--verification")==0)) {
      verify = 1;
      i--;
    }
    if (strcmp(argv[i],"--tests")==0) {
      run_all_tests();
      return 0;
    }
  }

  /* was a file specified */
  if (filename == NULL) {
    printf("No image file specified\n");
    return 0;
  }

  image_data = read_png_file(filename, &image_width, &image_height, &image_bitsperpixel);
  if (image_data == NULL) {
    printf("Couldn't load image %s\n", filename);
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
    printf("Image: %s\n", filename);
    printf("Resolution: %dx%d\n", image_width, image_height);
    printf("Depth: %d\n", image_bitsperpixel);
  }

  if (resized_image_width > 0) {
    if (resized_image_height == 0) {
      resized_image_height = image_height * resized_image_width / image_width;
    }
    /* create an array to store the enlarged image */
    resized_image_data =
      (unsigned char*)malloc(resized_image_width*resized_image_height*
                             image_bitsperpixel/8);
    if (!resized_image_data) return -653;
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


  char * decode_result = (char*)malloc(MAX_DECODE_LENGTH*sizeof(char));
  read_datamatrix(image_data, image_width, image_height,
                  image_bitsperpixel, debug,
                  output_filename,
                  test_ml_threshold,
                  test_erode,
                  test_dilate,
                  test_edge_threshold,
                  test_frequency,
                  verify, csv, json,
                  minimum_grid_dimension,
                  maximum_grid_dimension,
                  decode_result);
  if (strlen(decode_result) > 0) {
    if (verify == 0) {
      printf("%s\n", decode_result);
    }
    else {
      if ((csv == 0) && (json == 0)) {
        printf("Decode: PASS (%s)\n", decode_result);
      }
      else if (csv == 1) {
        printf("Decode, PASS, \"%s\"\n", decode_result);
      }
      else if (json == 1) {
        printf("  \"decode\": { \"grade\": \"PASS\", \"text\": \"%s\" }\n", decode_result);
        printf("}\n");
      }
    }
  }

  free(decode_result);
  return 0;
}
