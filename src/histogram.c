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

/* return the mean reflectance from a reflectance histogram */
int histogram_mean_reflectance(int * histogram)
{
  unsigned int i, mean_reflectance=0,total=0;

  for (i=0;i<256;i++) {
    total += histogram[i];
    mean_reflectance += i*histogram[i];
  }
  if (total==0) total=1;
  return mean_reflectance / total;
}

/* create a histogram from a region of an image */
void region_histogram(unsigned char * img,
                      int width, int height,
                      int tx, int ty, int bx, int by,
                      int bitsperpixel,
                      int * histogram)
{
  int i,x,y,n,reflectance;
  const int step = 2;
  int bytesperpixel = bitsperpixel/8;

  memset((void*)histogram,'\0',256*sizeof(int));

  if (tx < 0) tx = 0;
  if (ty < 0) ty = 0;
  if (bx >= width) bx = width-1;
  if (by >= height) by = height-1;

  for (y = ty; y <= by; y+=step) {
    for (x = tx; x <= bx; x+=step) {
      n = (y*width*bytesperpixel) + (x*bytesperpixel);
      reflectance = 0;
      for (i = 0; i < bytesperpixel; i++) {
        reflectance += img[n++];
      }
      histogram[reflectance/bytesperpixel]++;
    }
  }
}
