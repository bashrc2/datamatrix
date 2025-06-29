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
  int i,ch;
  int bytesperpixel = bitsperpixel/8;

  for (i = 0; i < width*height; i++)
    for (ch = 0; ch < bytesperpixel; ch++)
      colour[i*bytesperpixel + ch] = img[i];
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
  int i,ch,v;
  int bytesperpixel = bitsperpixel/8;

  for (i = 0; i < width*height; i++) {
    v = 0;
    for (ch = 0; ch < bytesperpixel; ch++) {
      v += img[i*bytesperpixel + ch];
    }
    mono[i] = v/bytesperpixel;
  }
}
