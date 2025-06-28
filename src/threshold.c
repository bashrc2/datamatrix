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

/* \brief is this a mostly dark image?
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

int meanlight_threshold(unsigned char * img, int width, int height,
                        int bitsperpixel, int threshold,
                        unsigned char * thresholded)
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

int proximal_threshold(unsigned char * img, int width, int height,
                       int r, int g, int b, int radius,
                       unsigned char * result)
{
  int i,j,dr,dg,db,diff,max_diff;

  max_diff = radius*radius*3;

  memset(result, 255, width*height*sizeof(unsigned char));

  j = width*height - 1;
  for (i = (width*height*3) - 1; i >= 2; i-=3, j--) {
    dr = r - (int)img[i];
    dg = g - (int)img[i - 1];
    db = b - (int)img[i - 2];
    diff = dr*dr + dg*dg + db*db;
    if (diff < max_diff) {
      result[j] = 0;
    }
  }
  return 0;
}


int proximal_erase(unsigned char * img, int width, int height,
                   int radius, int min_coverage_percent)
{
  int x, y, n,xx,yy,n2,hits,max_hits;

  max_hits = (radius*2)*(radius*2);

  for (y = 0; y < height; y++) {
    n = y*width;
    for (x = 0; x < width; x++,n++) {
      if (img[n] != 0) {
        continue;
      }
      hits = 0;
      for (yy = y - radius; yy < y + radius; yy++) {
        if ((yy < 0) || (yy >= height)) {
          continue;
        }
        n2 = yy*width;
        for (xx = x - radius; xx < x + radius; xx++) {
          if ((xx < 0) || (xx >= width)) {
            continue;
          }
          if (img[n2+xx] == 0) {
            hits++;
          }
        }
      }
      if (hits * 100 / max_hits < min_coverage_percent) {
        img[n] = 255;
      }
    }
  }
  return 0;
}

static int proximal_fill_point(unsigned char * img, int width, int height,
                               int x, int y, int r, int g, int b,
                               int background, unsigned char * result,
                               int * bounding_box)
{
  int n, xx, yy;

  if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
    return 0;
  }
  n = y*width + x;
  if (img[n] != 0) {
    return 0;
  }
  n *= 3;
  if (result[n] != background) {
    return 0;
  }
  result[n] = b;
  result[n+1] = g;
  result[n+2] = r;

  if (x < bounding_box[0]) bounding_box[0] = x;
  if (y < bounding_box[1]) bounding_box[1] = y;
  if (x > bounding_box[2]) bounding_box[2] = x;
  if (y > bounding_box[3]) bounding_box[3] = y;

  for (yy = y - 1; yy <= y + 1; yy++) {
    for (xx = x - 1; xx <= x + 1; xx++) {
      if ((xx == x) && (yy == y)) {
        continue;
      }
      proximal_fill_point(img, width, height, xx, yy, r, g, b,
                          background, result,
                          bounding_box);
    }
  }
  return 1;
}

static void proximal_clear_fill(unsigned char * img,
                                unsigned char * result, int width, int height,
                                int r, int g, int b, int background)
{
  int n, i=0;

  for (n = 0; n < width*height*3; n += 3, i++) {
    if ((result[n] == b) && (result[n+1] == g) && (result[n+2] == r)) {
      result[n] = background;
      result[n+1] = background;
      result[n+2] = background;
      img[i] = background;
    }
  }
}

/* reduce, reuse, recycle */
static int polygon_reduce(int * polygon_points, int no_of_points, int max_variance)
{
  int p, p2, start_x, start_y, mid_x, mid_y, end_x, end_y;
  int dx, dy, predicted_x, predicted_y, ddx, ddy;
  int new_no_of_points;
  int points_removed = 1;

  while (points_removed != 0) {
    new_no_of_points = no_of_points;
    points_removed = 0;
    for (p = no_of_points-1; p >= 2; p-=2) {
      start_x = polygon_points[p*2];
      start_y = polygon_points[p*2+1];
      mid_x = polygon_points[(p-1)*2];
      mid_y = polygon_points[(p-1)*2+1];
      end_x = polygon_points[(p-2)*2];
      end_y = polygon_points[(p-2)*2+1];
      dx = end_x - start_x;
      dy = end_y - start_y;
      predicted_x = start_x + (dx/2);
      predicted_y = start_y + (dy/2);
      ddx = mid_x - predicted_x;
      ddy = mid_y - predicted_y;
      if (ddx*ddx + ddy*ddy > max_variance) {
        continue;
      }
      for (p2 = p-1; p2 < new_no_of_points-1; p2++) {
        polygon_points[p2*2] = polygon_points[(p2+1)*2];
        polygon_points[p2*2+1] = polygon_points[(p2+1)*2+1];
      }
      new_no_of_points--;
      points_removed = 1;
    }
    no_of_points = new_no_of_points;
  }
  return no_of_points;
}

/* Get points around the perimeter of a bounding box.
   This is a very simplistic method, but may work well enough.
*/
static int polygon_perimeter_points(unsigned char * img,
                                    int tx, int ty, int width,
                                    int bb_width, int bb_height,
                                    int * polygon_points,
                                    int max_polygon_points,
                                    int r, int g, int b,
                                    int max_variance)
{
  int x,y,no_of_points=0,n;
  int x_step = 1 + (bb_width / 32);

  if (x_step < 2) x_step = 2;

  /* top down left to right */
  for (x = tx; x < tx + bb_width; x += x_step) {
    for (y = ty; y < ty + bb_height; y++) {
      n = (y*width + x)*3;
      if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
        if (no_of_points < max_polygon_points) {
          polygon_points[no_of_points*2] = x;
          polygon_points[no_of_points*2+1] = y;
          no_of_points++;
        }
        break;
      }
    }
  }

  /* bottom up right to left */
  for (x = tx + bb_width - 1; x >= tx; x -= x_step) {
    for (y = ty + bb_height - 1; y > ty; y--) {
      n = (y*width + x)*3;
      if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
        if (no_of_points < max_polygon_points) {
          polygon_points[no_of_points*2] = x;
          polygon_points[no_of_points*2+1] = y;
          no_of_points++;
        }
        break;
      }
    }
  }

  /* reduce the number of polygon points */
  no_of_points = polygon_reduce(polygon_points, no_of_points, max_variance);

  return no_of_points;
}

/*
  Detects an interior polygon inside of a larger polygon
*/
static int polygon_interior_points(unsigned char * img,
                                   int tx, int ty, int width,
                                   int bb_width, int bb_height,
                                   int * polygon_points,
                                   int max_polygon_points,
                                   int r, int g, int b,
                                   int max_variance)
{
  int x,y,no_of_points=0,n, state;
  int x_step = 1 + (bb_width / 32);
  int top_y=0,bottom_y=0,interior_ty=0,interior_by=0;

  if (x_step < 2) x_step = 2;

  /* top down left to right */
  for (x = tx; x < tx + bb_width; x += x_step) {
    state = 0;
    top_y=0;
    interior_ty = 0;
    interior_by = 0;
    for (y = ty; y < ty + bb_height; y++) {
      n = (y*width + x)*3;
      if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
        switch(state) {
        case 0: {
          /* start of top of larger polygon */
          top_y = y;
          state = 1;
          break;
        }
        case 2: {
          /* bottom of interior */
          state = 3;
          interior_by = y;
          break;
        }
        }
      }
      else {
        if (state == 1) {
          /* within interior */
          interior_ty = y;
          state = 2;
        }
      }
    }
    if (state == 3) {
      if ((interior_ty - top_y > 1) &&
          (interior_by - interior_ty > 1)) {
        if (no_of_points < max_polygon_points) {
          polygon_points[no_of_points*2] = x;
          polygon_points[no_of_points*2+1] = interior_ty;
          no_of_points++;
        }
      }
    }
  }

  /* bottom up right to left */
  for (x = tx + bb_width - 1; x >= tx; x -= x_step) {
    state = 0;
    bottom_y=0;
    interior_ty = 0;
    interior_by = 0;
    for (y = ty + bb_height - 1; y > ty; y--) {
      n = (y*width + x)*3;
      if ((img[n] == b) && (img[n+1] == g) && (img[n+2] == r)) {
        switch(state) {
        case 0: {
          /* start of top of larger polygon */
          bottom_y = y;
          state = 1;
          break;
        }
        case 2: {
          /* bottom of interior */
          state = 3;
          interior_by = y;
          break;
        }
        }
      }
      else {
        if (state == 1) {
          /* within interior */
          interior_ty = y;
          state = 2;
        }
      }
    }
    if (state == 3) {
      if ((bottom_y - interior_ty > 1) &&
          (interior_ty - interior_by > 1)) {
        if (no_of_points < max_polygon_points) {
          polygon_points[no_of_points*2] = x;
          polygon_points[no_of_points*2+1] = interior_ty;
          no_of_points++;
        }
      }
    }
  }

  /* reduce the number of interior polygon points */
  no_of_points = polygon_reduce(polygon_points, no_of_points, max_variance);

  return no_of_points;
}

int proximal_fill(unsigned char * img, int width, int height,
                  unsigned char * result,
                  int max_width, int max_height,
                  int max_variance,
                  int * polygon_id,
                  int * polygon_vertices,
                  int * polygons,
                  int max_total_polygon_points)
{
  int x, y, r, g, b, bb_width, bb_height, v,xx,yy,n;
  const int background = 255;
  int bounding_box[4];
  const int max_polygon_points = 1024;
  int polygon_points[max_polygon_points*2], polygon_no_of_points;
  int polygon_count = 0;
  int polygon_coords_total = 0;
  int curr_polygon_id = 0;

  memset(result, background, width*height*3*sizeof(unsigned char));

  r = (unsigned char)(rand()%254);
  g = (unsigned char)(rand()%254);
  b = (unsigned char)(rand()%254);

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      bounding_box[0] = x;
      bounding_box[1] = y;
      bounding_box[2] = x;
      bounding_box[3] = y;
      if (proximal_fill_point(img, width, height,
                              x, y, r, g, b,
                              background, result,
                              bounding_box) != 0) {
        bb_width = bounding_box[2] - bounding_box[0];
        bb_height = bounding_box[3] - bounding_box[1];
        if ((bb_width > max_width) || (bb_height > max_height)) {
          proximal_clear_fill(img, result, width, height, r, g, b, background);
        }
        else {
          polygon_no_of_points = \
            polygon_perimeter_points(result,
                                     bounding_box[0],
                                     bounding_box[1],
                                     width,
                                     bb_width, bb_height,
                                     polygon_points,
                                     max_polygon_points,
                                     r, g, b,
                                     max_variance);
          if ((polygon_no_of_points > 2) &&
              (polygon_coords_total + polygon_no_of_points < max_total_polygon_points)) {
            for (v = 0; v < polygon_no_of_points; v++) {
              /* show vertices */
              xx = polygon_points[v*2];
              yy = polygon_points[v*2+1];
              n = (yy*width + xx)*3;
              result[n] = 0;
              result[n+1] = 0;
              result[n+2] = 0;
              /* store vertices in polygons array */
              polygons[polygon_coords_total*2] = xx;
              polygons[polygon_coords_total*2+1] = yy;
              polygon_coords_total++;
            }
            polygon_id[polygon_count] = curr_polygon_id;
            polygon_vertices[polygon_count++] = polygon_no_of_points;
          }

          /* interior polygon */
          polygon_no_of_points = \
            polygon_interior_points(result,
                                    bounding_box[0],
                                    bounding_box[1],
                                    width,
                                    bb_width, bb_height,
                                    polygon_points,
                                    max_polygon_points,
                                    r, g, b,
                                    max_variance);
          if ((polygon_no_of_points > 2) &&
              (polygon_coords_total + polygon_no_of_points < max_total_polygon_points)) {
            for (v = 0; v < polygon_no_of_points; v++) {
              /* show vertices */
              xx = polygon_points[v*2];
              yy = polygon_points[v*2+1];
              n = (yy*width + xx)*3;
              result[n] = 0;
              result[n+1] = 0;
              result[n+2] = 0;
              /* store vertices in polygons array */
              polygons[polygon_coords_total*2] = xx;
              polygons[polygon_coords_total*2+1] = yy;
              polygon_coords_total++;
            }
            polygon_id[polygon_count] = curr_polygon_id;
            polygon_vertices[polygon_count++] = polygon_no_of_points;
          }
          curr_polygon_id++;
        }
        r = (unsigned char)(rand()%254);
        g = (unsigned char)(rand()%254);
        b = (unsigned char)(rand()%254);
      }
    }
  }
  return polygon_count;
}

int show_polygons(unsigned char * img, int width, int height,
                  int no_of_polygons,
                  int * polygon_id,
                  int * polygon_vertices,
                  int * polygons)
{
  int p, v, tx, ty, bx=0, by=0, r, g, b, start_x=0, start_y=0, n;
  int vertex_index = 0;
  int line_width = 3;

  memset(img, 255, width*height*3*sizeof(unsigned char));

  r = (unsigned char)(rand()%254);
  g = (unsigned char)(rand()%254);
  b = (unsigned char)(rand()%254);
  for (p = 0; p < no_of_polygons; p++) {
    if (p > 0) {
      if (polygon_id[p] != polygon_id[p-1]) {
        r = (unsigned char)(rand()%254);
        g = (unsigned char)(rand()%254);
        b = (unsigned char)(rand()%254);
      }
    }
    for (v = 0; v < polygon_vertices[p]; v++, vertex_index++) {
      if (v < polygon_vertices[p] - 1) {
        tx = polygons[vertex_index*2];
        ty = polygons[vertex_index*2+1];
        bx = polygons[(vertex_index+1)*2];
        by = polygons[(vertex_index+1)*2+1];
        if (v == 0) {
          /* position of the first vertex */
          start_x = tx;
          start_y = ty;
        }
      }
      else {
        /* last vertex links back to the first */
        tx = polygons[vertex_index*2];
        ty = polygons[vertex_index*2+1];
        bx = start_x;
        by = start_y;
      }

      draw_line(img, (unsigned int)width, (unsigned int)height, 3*8,
                tx, ty, bx, by, line_width,
                r, g, b);

      n = (ty*width + tx)*3;
      img[n] = 0;
      img[n+1] = 0;
      img[n+2] = 0;
    }
  }
  return 0;
}

/* returns non-zero if the given point is in the given polygon */
int point_in_polygon(int x, int y, int points[], int no_of_points)
{
  int i, j, c = 0;

  for(i = 0, j = no_of_points - 1; i < no_of_points; j = i++) {
    if (((points[i*2+1] >= y) != (points[j*2+1] >= y)) &&
        (x <= (points[j*2] - points[i*2]) * (y - points[i*2+1]) /
         (points[j*2+1] - points[i*2+1]) + points[i*2]))
      c = 1 - c;
  }

  return c;
}

int save_polygons_json(int no_of_polygons,
                       int * polygon_id,
                       int * polygon_vertices,
                       int * polygons,
                       char * filename)
{
  int p, v;
  int vertex_index = 0;
  int is_interior = 0;
  int next_interior = 0;
  FILE * fp;

  fp = fopen(filename, "w");
  if (fp == 0) return 1;

  fprintf(fp, "{\n");
  fprintf(fp, "  \"polygons\": {\n\n");

  for (p = 0; p < no_of_polygons; p++) {
    is_interior = 0;
    if (p > 0) {
      if (polygon_id[p - 1] == polygon_id[p]) {
        is_interior = 1;
      }
    }
    if (is_interior == 0) {
      fprintf(fp, "    \"%d\": {\n", polygon_id[p]);
      fprintf(fp, "      \"ext\": [");
    }
    else {
      fprintf(fp, ",\n");
      fprintf(fp, "      \"int\": [");
    }
    for (v = 0; v < polygon_vertices[p]; v++, vertex_index++) {
      if (v > 0) {
        fprintf(fp, ",(%d,%d)",
                polygons[vertex_index*2],
                polygons[vertex_index*2+1]);
      }
      else {
        fprintf(fp, "(%d,%d)",
                polygons[vertex_index*2],
                polygons[vertex_index*2+1]);
      }
    }
    fprintf(fp, "]");
    next_interior = 0;
    if (p < no_of_polygons - 1) {
      if (polygon_id[p + 1] == polygon_id[p]) {
        next_interior = 1;
      }
    }
    if (next_interior == 0) {
      /* has no interior */
      fprintf(fp, "\n");
      if (p < no_of_polygons - 1) {
        fprintf(fp, "    },\n");
      }
      else {
        fprintf(fp, "    }\n");
      }
    }
  }

  /* end of polygons */
  fprintf(fp, "  }\n");
  fprintf(fp, "}\n");

  fclose(fp);

  return 0;
}
