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
 * \param meanDark returned mean dark threshold
 * \param meanLight returned mean light threshold
 */
static void darklight_thresholds(unsigned int histogram[],
                                 float * meanDark,
                                 float * meanLight)
{
  float minVariance = 999999.0f;
  float currMeanDark = 0.0f;
  float currMeanLight = 0.0f;
  float varianceDark = 0.0f;
  float varianceLight = 0.0f;
  float darkHits = 0.0f;
  float lightHits = 0.0f;
  float histogramSquaredMagnitude[256] = {0};
  int h = 0;
  int bucket = 0;
  float magnitudeSqr = 0.0f;
  float variance = 0.0f;
  float divisor= 0.0f;
  int greyLevel;
  *meanDark = 0;
  *meanLight = 0;

  /* Calculate squared magnitudes -
     avoids unneccessary multiplies later on */
  for (greyLevel = 255; greyLevel >=0; greyLevel--) {
    histogramSquaredMagnitude[greyLevel] =
      histogram[greyLevel] * histogram[greyLevel];
  }

  /* Evaluate all possible thresholds */
  for (greyLevel = 255; greyLevel >= 0; greyLevel--) {
    darkHits = 0;
    lightHits = 0;
    currMeanDark = 0;
    currMeanLight = 0;
    varianceDark = 0;
    varianceLight = 0;

    bucket = (int)greyLevel;

    for(h = 255; h >= 0; h--) {
      magnitudeSqr = histogramSquaredMagnitude[h];
      if (h < bucket) {
        currMeanDark += h * magnitudeSqr;
        varianceDark += (bucket - h) * magnitudeSqr;
        darkHits += magnitudeSqr;
      }
      else {
        currMeanLight += h * magnitudeSqr;
        varianceLight += (bucket - h) * magnitudeSqr;
        lightHits += magnitudeSqr;
      }
    }

    if (darkHits > 0) {
      /* Rescale into 0-255 range */
      divisor = darkHits * 256;
      currMeanDark = (currMeanDark * 255) / divisor;
      varianceDark = (varianceDark * 255) / divisor;
    }

    if (lightHits > 0) {
      /* Rescale into 0-255 range */
      divisor = lightHits * 256;
      currMeanLight = (currMeanLight * 255) / divisor;
      varianceLight = (varianceLight * 255) / divisor;
    }

    variance = varianceDark + varianceLight;
    if (variance < 0)
      variance = -variance;

    if (variance < minVariance) {
      minVariance = variance;
      *meanDark = currMeanDark;
      *meanLight = currMeanLight;
    }

    if ((int)(variance * 1000) == (int)(minVariance * 1000))
      *meanLight = currMeanLight;
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
  float meanDark=0, meanLight=0;

  for (y = ty; y <= by; y += sample_step, n += vertical_increment) {
    n2 = n;

    for (x = tx; x <= bx; x += sample_step, n2++)
      histogram[img[n2]]++;
  }

  darklight_thresholds(histogram, &meanDark, &meanLight);
  *dark = (unsigned char)meanDark;
  *light = (unsigned char)meanLight;
}
