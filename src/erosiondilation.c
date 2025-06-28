/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - example computer vision functions
 *  Copyright (c) 2011-2015, Bob Mottram
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

/* \brief perform morphological dilation on the given mono image
 * \param img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param buffer image processing buffer, same size as img
 * \param itterations number of dilation itterations
 * \param result returned dilated image
 */
void dilate(unsigned char img[], int width, int height,
            unsigned char buffer[],
            int itterations,
            unsigned char result[])
{
  int pixels = width * height;

  memcpy(buffer, img, pixels);

  int n;
  unsigned char value;
  int min = width + 1;
  int max = pixels - width - 1;
  for (int itt = 0; itt < itterations; itt++) {
    for (int i = min; i < max; i++) {
      value = buffer[i];
      if (buffer[i - 1] > value) value = buffer[i - 1];
      if (buffer[i + 1] > value) value = buffer[i + 1];

      // above
      n = i - width - 1;
      if (buffer[n] > value) value = buffer[n];
      n++;
      if (buffer[n] > value) value = buffer[n];
      n++;
      if (buffer[n] > value) value = buffer[n];

      // below
      n = i + width - 1;
      if (buffer[n] > value) value = buffer[n];
      n++;
      if (buffer[n] > value) value = buffer[n];
      n++;
      if (buffer[n] > value) value = buffer[n];

      result[i] = value;
    }

    if (itt < itterations - 1)
      memcpy(buffer, result, pixels);
  }
}

/* \brief perform morphological erosion on the given mono image
 * \param img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param buffer image processing buffer, same size as img
 * \param itterations number of erosion itterations
 * \param result returned eroded image
 */
void erode(unsigned char * img, int width, int height,
           unsigned char * buffer,
           int itterations,
           unsigned char * result)
{
	int pixels = width * height;

  memcpy(buffer, img, pixels);

  int n;
  unsigned char value;
  int min = width + 1;
  int max = pixels - width - 1;
  for (int itt = 0; itt < itterations; itt++) {
    for (int i = min; i < max; i++) {
      value = buffer[i];

      if (buffer[i - 1] < value) value = buffer[i - 1];
      if (buffer[i + 1] < value) value = buffer[i + 1];

      // above
      n = i - width - 1;
      if (buffer[n] < value) value = buffer[n];
      n++;
      if (buffer[n] < value) value = buffer[n];
      n++;
      if (buffer[n] < value) value = buffer[n];

      // below
      n = i + width - 1;
      if (buffer[n] < value) value = buffer[n];
      n++;
      if (buffer[n] < value) value = buffer[n];
      n++;
      if (buffer[n] < value) value = buffer[n];

      result[i] = value;
    }

    if (itt < itterations - 1) {
      memcpy(buffer, result, pixels);
    }
  }
}
