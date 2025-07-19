/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils: Obtaining dark and light thresholds
 *  Copyright (c) 2017, Bob Mottram
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
 * \brief returns the mean light and mean dark thresholds for the given
 *        reflectance histogram
 * \param histogram reflectance histogram with 256 buckets
 * \param mean_dark returned mean dark threshold
 * \param mean_light returned mean light threshold
 */
static void darklight_thresholds(unsigned int histogram[],
                                 float * mean_dark,
                                 float * mean_light)
{
  float min_variance = 999999.0f;
  float curr_mean_dark = 0.0f;
  float curr_mean_light = 0.0f;
  float variance_dark = 0.0f;
  float variance_light = 0.0f;
  float dark_hits = 0.0f;
  float light_hits = 0.0f;
  float histogram_sqr_mag[256] = {0};
  int h = 0;
  int bucket = 0;
  float sqr_mag = 0.0f;
  float variance = 0.0f;
  float divisor= 0.0f;
  int grey_level;
  *mean_dark = 0;
  *mean_light = 0;

  /* Calculate squared magnitudes -
     avoids unneccessary multiplies later on */
  for (grey_level = 255; grey_level >=0; grey_level--) {
    histogram_sqr_mag[grey_level] =
      histogram[grey_level] * histogram[grey_level];
  }

  /* Evaluate all possible thresholds */
  for (grey_level = 255; grey_level >= 0; grey_level--) {
    dark_hits = 0;
    light_hits = 0;
    curr_mean_dark = 0;
    curr_mean_light = 0;
    variance_dark = 0;
    variance_light = 0;

    bucket = grey_level;

    for(h = 255; h >= 0; h--) {
      sqr_mag = histogram_sqr_mag[h];
      if (h < bucket) {
        curr_mean_dark += h * sqr_mag;
        variance_dark += (bucket - h) * sqr_mag;
        dark_hits += sqr_mag;
      }
      else {
        curr_mean_light += h * sqr_mag;
        variance_light += (bucket - h) * sqr_mag;
        light_hits += sqr_mag;
      }
    }

    if (dark_hits > 0) {
      /* Rescale into 0-255 range */
      divisor = dark_hits * 256;
      curr_mean_dark = (curr_mean_dark * 255) / divisor;
      variance_dark = (variance_dark * 255) / divisor;
    }

    if (light_hits > 0) {
      /* Rescale into 0-255 range */
      divisor = light_hits * 256;
      curr_mean_light = (curr_mean_light * 255) / divisor;
      variance_light = (variance_light * 255) / divisor;
    }

    variance = variance_dark + variance_light;
    if (variance < 0)
      variance = -variance;

    if (variance < min_variance) {
      min_variance = variance;
      *mean_dark = curr_mean_dark;
      *mean_light = curr_mean_light;
    }

    if ((int)(variance * 1000) == (int)(min_variance * 1000))
      *mean_light = curr_mean_light;
  }
}

/**
 * \brief calculates mean light and mean dark thresholds within an image
 * \param img image array
 * \param width width of the image
 * \param height height of the image
 * \param sample_step subsampling step
 * \param sampling_radius_percent Radius to sample within as a percentage of image width
 * \param dark returned mean dark threshold
 * \param light returned mean light threshold
 */
void darklight(unsigned char img[],
               int width, int height,
               int sample_step,
               int sampling_radius_percent,
               unsigned char * dark, unsigned char * light)
{
  unsigned int x,y,n2;
  unsigned int histogram[256] = {0};
  unsigned int tx =
    (unsigned int)((width * sampling_radius_percent / 100)/2);
  unsigned int ty =
    (unsigned int)((height * sampling_radius_percent / 100)/2);
  unsigned int bx = width - 1 - tx;
  unsigned int by = height - 1 - ty;
  unsigned int n = (ty * (unsigned int)width) + tx;
  unsigned int vertical_increment =
    (unsigned int)(width * sample_step);
  float mean_dark=0, mean_light=0;

  for (y = ty; y <= by; y += sample_step, n += vertical_increment) {
    n2 = n;

    for (x = tx; x <= bx; x += sample_step, n2++)
      histogram[img[n2]]++;
  }

  darklight_thresholds(histogram, &mean_dark, &mean_light);
  *dark = (unsigned char)mean_dark;
  *light = (unsigned char)mean_light;
}
