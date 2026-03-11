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

/**
 * \brief convert an image from mono to colour
 * \param img image array
 * \param width width of the image
 * \param height height of the image
 * \param bitsperpixel Number of bits per pixel
 * \param colour returned colour image
 */
void mono_to_colour(unsigned char img[], int width, int height,
                    int bitsperpixel,
                    unsigned char colour[])
{
  int bytesperpixel = bitsperpixel/8;
  int idx = width*height - 1;
  int ch = bytesperpixel;

  for (int i = width*height*bytesperpixel - 1; i >= 0; i--, ch--) {
      colour[i] = img[idx];
      if (ch == 0) {
          ch = bytesperpixel;
          idx--;
      }
  }
}

/**
 * \brief convert an image from colour to mono
 * \param width width of the image
 * \param height height of the image
 * \param bitsperpixel Number of bits per pixel
 * \param mono returned mono image array
 */
void colour_to_mono(unsigned char img[], int width, int height,
                    int bitsperpixel,
                    unsigned char mono[])
{
  int bytesperpixel = bitsperpixel/8;
  int idx = width*height - 1;
  int ch = bytesperpixel;
  int sum = 0;

  for (int i = width*height*bytesperpixel - 1; i >= 0; i--, ch--) {
      sum += img[i];
      if (ch == 0) {
          ch = bytesperpixel;
          idx--;
          mono[idx] = sum / bytesperpixel;
          sum = 0;
      }
  }
}
