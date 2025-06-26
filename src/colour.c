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

void remove_colour(unsigned char * img,
                   int width, int height,
                   int r, int g, int b, int radius)
{
  int i, dr, dg, db, diff;

  for (i = 0; i < width*height*3; i+=3) {
    dr = r - (int)img[i];
    dg = g - (int)img[i+1];
    db = g - (int)img[i+2];
    diff = dr*dr + dg*dg + db*db;
    if (diff < radius) {
      img[i] = 255;
      img[i+1] = 255;
      img[i+2] = 255;
    }
  }
}

void remove_channel(unsigned char * img,
                    int width, int height, int bitsperpixel,
                    int channel)
{
    int x,y,n,i;
    int bytesperpixel = bitsperpixel/8;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            n = (y*width*bytesperpixel) + (x*bytesperpixel);
            for (i = 0; i < bytesperpixel; i++) {
                if (i != channel) {
                    if (img[n+channel] < img[n+i]) break;
                }
            }
            if (i == bytesperpixel) {
                for (i = 0; i < bytesperpixel; i++) {
                    img[n + i] = 0;
                }
            }
        }
    }
}

void single_channel(unsigned char * img,
                    int width, int height, int bitsperpixel,
                    int channel, int threshold)
{
  int x,y,n,i;
  int bytesperpixel = bitsperpixel/8;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      n = (y*width*bytesperpixel) + (x*bytesperpixel);
      if ((img[n] > threshold) || (img[n+channel] < 50) || (img[n+channel] < img[n])) {
        for (i = 0; i < bytesperpixel; i++) {
          img[n+i] = 255;
        }
      }
      else {
        for (i = 0; i < bytesperpixel; i++) {
          if (i != channel) {
            img[n+i] = img[n+channel];
          }
        }
      }
    }
  }
}

void enhance_channel(unsigned char * img,
                     int width, int height,
                     int channel, int threshold)
{
  int x,y,n,c, diff, dc;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      n = (y*width + x)*3;
      diff = 0;
      for (c = 0; c < 3; c++) {
        if (c == channel) continue;
        dc = img[n+channel] - img[n+c];
        diff += (dc * dc);
      }
      if (diff > threshold) {
        for (c = 0; c < 3; c++) {
          img[n + c] = 255;
        }
      }
    }
  }
}

void enhance_colour(unsigned char * img,
                    int width, int height,
                    int r, int g, int b, int threshold)
{
  int x,y,n,c, diff, dr,dg,db,rr,gg,bb;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      n = (y*width + x)*3;
      rr = img[n];
      gg = img[n+1];
      bb = img[n+2];
      if ((gg > bb) || (rr > bb)) {
        for (c = 0; c < 3; c++) {
          img[n + c] = 255;
        }
        continue;
      }
      if (abs(gg-rr)>50) {
        for (c = 0; c < 3; c++) {
          img[n + c] = 255;
        }
        continue;
      }
      dr = r - rr;
      dg = g - gg;
      db = b - bb;
      diff = dr*dr + dg*dg + db*db;
      if (diff > threshold) {
        for (c = 0; c < 3; c++) {
          img[n + c] = 255;
        }
      }
    }
  }
}

int rgb_min(unsigned char r, unsigned char g, unsigned char b)
{
    int min = r;
    if (g < min) min = g;
    if (b < min) min = b;
    return min;
}

int rgb_max(unsigned char r, unsigned char g, unsigned char b)
{
    int max = r;
    if (g > max) max = g;
    if (b > max) max = b;
    return max;
}

static void rgb_to_hsv_pixel(unsigned char r, unsigned char g, unsigned char b,
                             unsigned char *h, unsigned char *s, unsigned char *v)
{
    int min, max, delta;
    float hue;

    min = rgb_min(r, g, b);
    max = rgb_max(r, g, b);
    *v = (unsigned char)max;

    delta = max - min;

    if (max != 0)
        *s = (unsigned char)(delta / max);
    else {
        *s = 0;
        *h = -1;
        return;
    }

    if ((int)r == max)
        hue = (g - b) / (float)delta;
    else if ((int)g == max)
        hue = 2 + (b - r) / (float)delta;
    else
        hue = 4 + (r - g) / (float)delta;

    hue *= 60;
    if (hue < 0) {
        hue += 360;
    }
    *h = (unsigned char)(hue * 255 / 360);
}

static void rgb_to_hsl_pixel(unsigned char r, unsigned char g, unsigned char b,
                             unsigned char *h, unsigned char *s, unsigned char *l)
{
    int min, max, delta;
    float hue;

    min = rgb_min(r, g, b);
    max = rgb_max(r, g, b);
    *l = (unsigned char)(((299*(unsigned int)r) + (587*(unsigned int)g) + (114*(unsigned int)b))/1024);

    delta = max - min;

    if (max != 0)
        *s = (unsigned char)(delta / max);
    else {
        *s = 0;
        *h = -1;
        return;
    }

    if ((int)r == max)
        hue = (g - b) / (float)delta;
    else if ((int)g == max)
        hue = 2 + (b - r) / (float)delta;
    else
        hue = 4 + (r - g) / (float)delta;

    hue *= 60;
    if (hue < 0) {
        hue += 360;
    }
    *h = (unsigned char)(hue * 255 / 360);
}

int rgb_to_hsv(unsigned char img[],
               int width, int height, int bitsperpixel,
               unsigned char img_hsv[])
{
    int i, bytes_per_pixel = bitsperpixel/8;

    if (bytes_per_pixel < 3) return 1;

    for (i = 0; i < width*height; i++)
        rgb_to_hsv_pixel(img[i*bytes_per_pixel + 2],
                         img[i*bytes_per_pixel + 1],
                         img[i*bytes_per_pixel],
                         &img_hsv[i*bytes_per_pixel],
                         &img_hsv[i*bytes_per_pixel + 1],
                         &img_hsv[i*bytes_per_pixel + 2]);

    return 0;
}

int rgb_to_hsl(unsigned char img[],
               int width, int height, int bitsperpixel,
               unsigned char img_hsl[])
{
    int i, bytes_per_pixel = bitsperpixel/8;

    if (bytes_per_pixel < 3) return 1;

    for (i = 0; i < width*height; i++)
        rgb_to_hsl_pixel(img[i*bytes_per_pixel + 2],
                         img[i*bytes_per_pixel + 1],
                         img[i*bytes_per_pixel],
                         &img_hsl[i*bytes_per_pixel],
                         &img_hsl[i*bytes_per_pixel + 1],
                         &img_hsl[i*bytes_per_pixel + 2]);

    return 0;
}

void rgb_to_cielab_pixel(unsigned char r, unsigned char g, unsigned char b,
                         unsigned char *Cl, unsigned char *Ca, unsigned char *Cb)
{
    int i;
    float var_X, var_Y, var_Z, X, Y, Z;
    float var_R = r / 255.0f;
    float var_G = g / 255.0f;
    float var_B = b / 255.0f;
    float ref_X =  95.047;
    float ref_Y = 100.000;
    float ref_Z = 108.883;
    float result[3];

    /* convery to XYZ */
    if ( var_R > 0.04045f ) {
        var_R = (float)pow(((var_R + 0.055f) / 1.055f), 2.4f);
    }
    else {
        var_R = var_R / 12.92f;
    }

    if ( var_G > 0.04045f ) {
        var_G = (float)pow(((var_G + 0.055f) / 1.055), 2.4f);
    }
    else {
        var_G = var_G / 12.92f;
    }

    if ( var_B > 0.04045f ) {
        var_B = (float)pow(((var_B + 0.055f) / 1.055f), 2.4f);
    }
    else {
        var_B = var_B / 12.92f;
    }

    var_R = var_R * 100.0f;
    var_G = var_G * 100.0f;
    var_B = var_B * 100.0f;

    X = var_R * 0.4124f + var_G * 0.3576f + var_B * 0.1805f;
    Y = var_R * 0.2126f + var_G * 0.7152f + var_B * 0.0722f;
    Z = var_R * 0.0193f + var_G * 0.1192f + var_B * 0.9505f;

    /* CIELAB */
    var_X = X / ref_X;
    var_Y = Y / ref_Y;
    var_Z = Z / ref_Z;

    if ( var_X > 0.008856f ) {
        var_X = (float)pow(var_X, 1.0f / 3.0f);
    }
    else {
        var_X = (7.787f * var_X) + (16 / 116);
    }

    if ( var_Y > 0.008856f ) {
        var_Y = (float)pow(var_Y, 1.0f / 3.0f);
    }
    else {
        var_Y = (7.787f * var_Y) + (16.0f / 116.0f);
    }

    if ( var_Z > 0.008856f ) {
        var_Z = (float)pow(var_Z, 1.0f / 3.0f);
    }
    else {
        var_Z = (7.787f * var_Z) + (16.0f / 116.0f);
    }

    result[0] = ((116 * var_Y) - 16);
    result[1] = (500 * (var_X - var_Y));
    result[2] = (200 * (var_Y - var_Z));
    for (i = 0; i < 3; i++) {
        if (result[i] < 0) result[i] += 255;
    }

    *Cl = (unsigned char)result[0];
    *Ca = (unsigned char)result[1];
    *Cb = (unsigned char)result[2];
}

int rgb_to_cielab(unsigned char img[],
                  int width, int height, int bitsperpixel,
                  unsigned char img_cielab[])
{
    int i, bytes_per_pixel = bitsperpixel/8;

    if (bytes_per_pixel < 3) return 1;

    for (i = 0; i < width*height; i++) {
        rgb_to_cielab_pixel(img[i*bytes_per_pixel + 2],
                            img[i*bytes_per_pixel + 1],
                            img[i*bytes_per_pixel],
                            &img_cielab[i*bytes_per_pixel],
                            &img_cielab[i*bytes_per_pixel + 1],
                            &img_cielab[i*bytes_per_pixel + 2]);
    }

    return 0;
}

/* reduce RGB to 8 colours (simple colour segmentation) */
void rgb_colour_reduce(unsigned char img[],
                       int width, int height)
{
    int x, y, z, i, index;
    unsigned int average[3],colour[2*2*2][3],hits[2*2*2];
    int pixels = width*height;

    average[0] = img[0];
    average[1] = img[1];
    average[2] = img[2];
    for (i = 3; i < pixels*3; i+=3) {
        average[0] += img[i];
        average[1] += img[i+1];
        average[2] += img[i+2];
    }
    average[0] /= pixels;
    average[1] /= pixels;
    average[2] /= pixels;

    /* clear colours */
    for (i = 0; i < 2*2*2; i++) {
        hits[i] = 0;
        colour[i][0] = 0;
        colour[i][1] = 0;
        colour[i][2] = 0;
    }

    /* update the colour buckets */
    for (i = 0; i < pixels*3; i+=3) {
        x = y = z = 0;
        if (img[i] > average[0]) x = 1;
        if (img[i+1] > average[1]) y = 1;
        if (img[i+2] > average[2]) z = 1;
        index = (x*4) + (y*2) + z;
        colour[index][0] += img[i];
        colour[index][1] += img[i+1];
        colour[index][2] += img[i+2];
        hits[index]++;
    }

    /* calculate average colours */
    for (i = 0; i < 2*2*2; i++) {
        if (hits[i] > 0) {
            colour[i][0] /= hits[i];
            colour[i][1] /= hits[i];
            colour[i][2] /= hits[i];
        }
    }

    /* update the image */
    for (i = 0; i < pixels*3; i+=3) {
        x = y = z = 0;
        if (img[i] > average[0]) x = 1;
        if (img[i+1] > average[1]) y = 1;
        if (img[i+2] > average[2]) z = 1;
        index = (x*4) + (y*2) + z;
        img[i] = colour[index][0];
        img[i+1] = colour[index][1];
        img[i+2] = colour[index][2];
    }
}
