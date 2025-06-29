/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - functions to produce thresholded images
 *  Copyright (c) 2011-2022, Bob Mottram
 *  bob@libreserver.org
 *
 *  This code is based on Visionutils by Bob Mottram which was
 *  originally distributed under BSD 2-clause license.
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
 * \brief is this a mostly dark image?
 * \param img_mono mono image array
 * \param width width of the image
 * \param height height of the image
 * \param dark peak dark
 * \param light peak light
 * \return 0 if this is a mostly dark image
 */
static unsigned char is_dark_image(unsigned char img_mono[],
                                   int width, int height,
                                   unsigned char dark,
                                   unsigned char light)
{
  int dark_pixels=0, light_pixels=0;
  int i, diff_dark, diff_light;
  int dark2 = (int)dark;
  int light2 = (int)light;

  for (i = (width*height) - 1; i >= 0 ; i--) {
    diff_dark = (int)img_mono[i] - dark2;
    diff_light = (int)img_mono[i] - light2;
    if (diff_dark*diff_dark < diff_light*diff_light) {
      dark_pixels++;
    }
    else {
      light_pixels++;
    }
  }
  if (dark_pixels > light_pixels) return 0;
  return 1;
}

/**
 * \brief creates a thresholded image
 * \param img image array
 * \param width width of the image
 * \param height height of the image
 * \param bitsperpixel Number of bits per pixel
 * \param threshold the threshold to be applied
 * \param thresholded returned thresholded image array
 * \return
 */
int meanlight_threshold(unsigned char img[], int width, int height,
                        int bitsperpixel, int threshold,
                        unsigned char thresholded[])
{
  unsigned char * img_mono = thresholded;
  unsigned char dark=0,light=0,range;
  unsigned int percent_active=0;
  unsigned char thresh;
  int i, percent;

  colour_to_mono(img, width, height, bitsperpixel, img_mono);

  /* get the dark and light peaks */
  darklight(img_mono, width, height, 1, 50, &dark, &light);

  range = light - dark;
  /* initial threshold is half way between light and dark */
  thresh = (unsigned char)(dark + (range / 2));
  /* adapt the threshold to the dynamic range */
  threshold = threshold * (int)range / 100;
  if (is_dark_image(img_mono, width, height, dark, light)) {
    /* Image is mostly light, count the dark pixels */
    if ((int)dark + threshold > 255)
      thresh = (unsigned char)255;
    else
      thresh = (unsigned char)((int)dark + threshold);

    for (i = (width*height) - 1; i >= 0; i--)
      if (img_mono[i] < thresh) {
        thresholded[i] = 255;
        percent_active++;
      }
      else {
        thresholded[i] = 0;
      }
  }
  else {
    /* Image is mostly dark, count the light pixels */
    if ((int)light - threshold < 0)
      thresh = (unsigned char)0;
    else
      thresh = (unsigned char)((int)light - threshold);

    for (i = (width*height) - 1; i >= 0 ; i--)
      if (img_mono[i] >= thresh) {
        thresholded[i] = 255;
        percent_active++;
      }
      else
        thresholded[i] = 0;
  }
  percent = (int)(percent_active*100/(width*height));
  if (percent > 30) {
    /* invert the thresholded image */
    for (i = (width*height) - 1; i >= 0 ; i--) thresholded[i] = 255 - thresholded[i];
    percent = 100 - percent;
  }
  mono_to_colour(img_mono, width, height, bitsperpixel, img);
  return percent;
}
