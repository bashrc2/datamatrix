/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Visionutils - example computer vision functions
 *  Copyright (c) 2011-2015, 2025, Bob Mottram
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
 * \brief does the line intersect with the given line?
 * \param x0 first line top x
 * \param y0 first line top y
 * \param x1 first line bottom x
 * \param y1 first line bottom y
 * \param x2 second line top x
 * \param y2 second line top y
 * \param x3 second line bottom x
 * \param y3 second line bottom y
 * \param xi intersection x coordinate
 * \param yi intersection y coordinate
 * \return 0 if the lines intersect, -2 if parallel
 */
int intersection(float x0, float y0,
                 float x1, float y1,
                 float x2, float y2,
                 float x3, float y3,
                 float * xi, float * yi)
{
  /* constants of linear equations */
  float a1, b1, c1, a2, b2, c2;
  /* the inverse of the determinant of the coefficient */
  float det_inv;
  /* the gradients of each line */
  float m1, m2, dm;
  /* is the intersection along the lines given, or outside them */
  int inside_line = -1;
  float tx, ty, bx, by;

  /* compute gradients */
  if ((x1 - x0) != 0) {
    m1 = (y1 - y0) / (x1 - x0);
  }
  else {
    m1 = (float)1e+10;
  }

  if ((x3 - x2) != 0) {
    m2 = (y3 - y2) / (x3 - x2);
  }
  else {
    m2 = (float)1e+10;
  }

  dm = ABS(m1 - m2);
  if (dm > 0.000001f) {
    /* compute constants */
    a1 = m1;
    a2 = m2;

    b1 = -1;
    b2 = -1;

    c1 = y0 - m1 * x0;
    c2 = y2 - m2 * x2;

    /* compute the inverse of the determinate */
    det_inv = 1 / (a1 * b2 - a2 * b1);

    /* use Kramers rule to compute xi and yi */
    *xi = (b1 * c2 - b2 * c1) * det_inv;
    *yi = (a2 * c1 - a1 * c2) * det_inv;

    /* is the intersection inside the line or outside it? */
    if (x0 < x1) { tx = x0; bx = x1; } else { tx = x1; bx = x0; }
    if (y0 < y1) { ty = y0; by = y1; } else { ty = y1; by = y0; }
    if ((*xi >= tx) && (*xi <= bx) && (*yi >= ty) && (*yi <= by)) {
      if (x2 < x3) { tx = x2; bx = x3; } else { tx = x3; bx = x2; }
      if (y2 < y3) { ty = y2; by = y3; } else { ty = y3; by = y2; }
      if ((*xi >= tx) && (*xi <= bx) && (*yi >= ty) && (*yi <= by)) {
        inside_line = 0;
      }
    }
  }
  else {
    /* parallel lines */
    *xi = PARALLEL_LINES;
  }

  return inside_line;
}

/**
 * \brief returns the perpendicular distance of a point from a line
 * \param x0 line first point x coordinate
 * \param y0 line first point y coordinate
 * \param x1 line second point x coordinate
 * \param y1 line second point y coordinate
 * \param point_x point x coordinate
 * \param point_y point y coordinate
 * \param line_intersection_x returned perpendicular x intersection point on the line
 * \param line_intersection_y returned perpendicular y intersection point on the line
 * \return distance of the point from the line
 */
float point_dist_from_line(float x0, float y0,
                           float x1, float y1,
                           float point_x, float point_y,
                           float * line_intersection_x,
                           float * line_intersection_y)
{
  float dx = x1 - x0;
  float dy = y1 - y0;
  float line_length = (float)sqrt((dx * dx) + (dy * dy));
  float cross = dx*(point_y-y0) - dy*(point_x-x0);
  if (line_length >= 0.001)
    return cross / line_length;
  return UNKNOWN_DISTANCE;
}

/**
 * \brief fits a line to set of edges using RANSAC
 * \param edges array of edges
 * \param no_of_edges number of edges in the array
 * \param linefit temporary array used for line fir refinement
 * \param max_deviation maximum deviation of edge positions from the ideal best fit line in pixels
 * \param no_of_samples number of possible lines to consider
 * \param no_of_edge_samples number of edges to sample for each candidate line
 * \param x0 returned start x coordinate of the best fit line
 * \param y0 returned start y coordinate of the best fit line
 * \param x1 returned end x coordinate of the best fit line
 * \param y1 returned end y coordinate of the best fit line
 * \return number of edges on best fit line
 */
int ransac_fit(int edges[], int no_of_edges,
               int linefit[], int max_edges,
               float max_deviation,
               int no_of_samples,
               int no_of_edge_samples,
               float *x0, float *y0,
               float *x1, float *y1)
{
	int i, max_hits = 0;
  float gradient;
  float min_deviation = 999999;
  int best_index0=0;
  int best_index1=1;
  int hits, hits0, hits1;
  float av_x, av_y;
  int idx, idx0, idx1, xx0, yy0, xx1, yy1, dx, dy;
  float deviation, deviation_sum, predicted_edge_x, predicted_edge_y;
  int sample, index, index0, index1, edge_sample;
  unsigned char horizontal;
  int edge_x, edge_y, linefit_no_of_edges=0;

  if ((no_of_samples == 0) || (no_of_edge_samples == 0)) return max_hits;

	if (no_of_edges < 10) return max_hits;

  if (no_of_edge_samples > no_of_edges) no_of_edge_samples = no_of_edges;

  /* make the random selection deterministic */
  srand(36789);

  for (sample = no_of_samples-1; sample >= 0; sample--) {
    /* select a baseline at random */
    index0 = rand() % no_of_edges;
    index1 = rand() % no_of_edges;
    if (index0 == index1) continue;

    hits = 0;
    deviation_sum = 0;
    idx0 = index0 * 2;
    idx1 = index1 * 2;
    xx0 = edges[idx0];
    yy0 = edges[idx0 + 1];
    xx1 = edges[idx1];
    yy1 = edges[idx1 + 1];
    dx = xx1 - xx0;
    dy = yy1 - yy0;

    /* horizontal orientation? */
    horizontal = 1;
    if (ABS(dy) > ABS(dx)) {
      horizontal = 0;
      gradient = dx / (float)dy;
    }
    else {
      gradient = dy / (float)dx;
    }

    for (edge_sample = no_of_edge_samples-1; edge_sample >= 0; edge_sample--) {
      /* select an edge at random */
      index = rand() % no_of_edges;
      edge_x = edges[index * 2];
      edge_y = edges[(index * 2) + 1];

      /* calculate the deviation from expected */
      deviation = 0;
      if (horizontal == 1) {
        predicted_edge_y = yy0 + ((edge_x - xx0) * gradient);
        deviation = ABS(predicted_edge_y - edge_y);
      }
      else {
        predicted_edge_x = xx0 + ((edge_y - yy0) * gradient);
        deviation = ABS(predicted_edge_x - edge_x);
      }

      if (deviation < max_deviation) {
        hits++;
        deviation_sum += deviation;
      }
    }

    if (hits > 0) {
      /* line with the maximum number of edges within the
         maximum deviation range */
      if (hits > max_hits) {
        best_index0 = index0;
        best_index1 = index1;
        max_hits = hits;
        min_deviation = deviation_sum;
      }
      else {
        /* select the minimum deviation */
        if ((hits == max_hits) &&
            (deviation_sum < min_deviation)) {
          best_index0 = index0;
          best_index1 = index1;
          min_deviation = deviation_sum;
        }
      }
    }
  }

  /* improve fit accuracy */
  if (max_hits > 0) {
    linefit_no_of_edges = 0;
    hits0 = 0;
    hits1 = 0;
    av_x = 0;
    av_y = 0;
    *x0 = 0;
    *y0 = 0;
    *x1 = 0;
    *y1 = 0;
    idx0 = best_index0 * 2;
    idx1 = best_index1 * 2;
    xx0 = edges[idx0];
    yy0 = edges[idx0 + 1];
    xx1 = edges[idx1];
    yy1 = edges[idx1 + 1];
    dx = xx1 - xx0;
    dy = yy1 - yy0;

    /* horizontal orientation? */
    horizontal = 1;
    if (ABS(dy) > ABS(dx)) {
      horizontal = 0;
      gradient = dx / (float)dy;
    }
    else {
      gradient = 0;
      if (dx != 0) gradient = dy / (float)dx;
    }

    /* for each edge */
    for (edge_sample = no_of_edges-1; edge_sample >= 0; edge_sample--) {
      edge_x = edges[edge_sample * 2];
      edge_y = edges[(edge_sample * 2) + 1];

      /* calculate the deviation from expected */
      deviation = 0;
      if (horizontal == 1) {
        predicted_edge_y = yy0 + ((edge_x - xx0) * gradient);
        deviation = ABS(predicted_edge_y - edge_y);
      }
      else {
        predicted_edge_x = xx0 + ((edge_y - yy0) * gradient);
        deviation = ABS(predicted_edge_x - edge_x);
      }

      /* store edge index within tolerance */
      if ((deviation < max_deviation) &&
          (linefit_no_of_edges < max_edges)) {
        linefit[linefit_no_of_edges] = edge_sample * 2;
        linefit_no_of_edges++;
        /* average edge position */
        av_x += edge_x;
        av_y += edge_y;
      }
    }

    if (linefit_no_of_edges > 0) {
      /* average edge position */
      av_x /= linefit_no_of_edges;
      av_y /= linefit_no_of_edges;

      for (i = 0; i < linefit_no_of_edges; i++) {
        idx = linefit[i];
        if (horizontal == 1) {
          if (edges[idx] < av_x) {
            *x0 += edges[idx];
            *y0 += edges[idx + 1];
            hits0++;
          }
          else {
            *x1 += edges[idx];
            *y1 += edges[idx + 1];
            hits1++;
          }
          continue;
        }

        if (edges[idx+1] < av_y) {
          *x0 += edges[idx];
          *y0 += edges[idx + 1];
          hits0++;
        }
        else {
          *x1 += edges[idx];
          *y1 += edges[idx + 1];
          hits1++;
        }
      }

      if (hits0 > 0) {
        *x0 /= hits0;
        *y0 /= hits0;
      }
      if (hits1 > 0) {
        *x1 /= hits1;
        *y1 /= hits1;
      }
    }
  }
  else *x0 = NO_LINE_FIT;

	return(linefit_no_of_edges);
}

/**
 * \brief angle between three points describing a corner
 * \param x0 line first point x coordinate
 * \param y0 line first point y coordinate
 * \param x1 line second point x coordinate
 * \param y1 line second point y coordinate
 * \param x2 line third point x coordinate
 * \param y2 line third point y coordinate
 * \return angle in radians
 */
float corner_angle(float x0, float y0,
                   float x1, float y1,
                   float x2, float y2)
{
  float pt1 = x0 - x1, pt2 = y0 - y1;
  float pt3 = x2 - x1, pt4 = y2 - y1;
  float angle = ((pt1 * pt3) + (pt2 * pt4)) /
    (((float)sqrt((pt1*pt1) + (pt2*pt2))) *
     ((float)sqrt((pt3*pt3) + (pt4*pt4))));

  angle = (float)acos(angle);
  return(angle);
}

/**
 * \brief get the length of the given perimeter side
 * \param side index number of the perimeter side
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \return length of the side
 */
float get_side_length(int side,
                      float perimeter_x0, float perimeter_y0,
                      float perimeter_x1, float perimeter_y1,
                      float perimeter_x2, float perimeter_y2,
                      float perimeter_x3, float perimeter_y3)
{
  float x0=0, y0=0, x1=0, y1=0, dx, dy;

  switch(side) {
  case 0: {
    x0 = (int)perimeter_x0;
    y0 = (int)perimeter_y0;
    x1 = (int)perimeter_x1;
    y1 = (int)perimeter_y1;
    break;
  }
  case 1: {
    x0 = (int)perimeter_x1;
    y0 = (int)perimeter_y1;
    x1 = (int)perimeter_x2;
    y1 = (int)perimeter_y2;
    break;
  }
  case 2: {
    x0 = (int)perimeter_x2;
    y0 = (int)perimeter_y2;
    x1 = (int)perimeter_x3;
    y1 = (int)perimeter_y3;
    break;
  }
  case 3: {
    x0 = (int)perimeter_x3;
    y0 = (int)perimeter_y3;
    x1 = (int)perimeter_x0;
    y1 = (int)perimeter_y0;
    break;
  }
  }
  dx = x1 - x0;
  dy = y1 - y0;
  return (float)sqrt(dx*dx + dy*dy);
}

/**
 * \brief get the longest side of a perimeter
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \return longest side length
 */
float get_longest_side(float perimeter_x0, float perimeter_y0,
                       float perimeter_x1, float perimeter_y1,
                       float perimeter_x2, float perimeter_y2,
                       float perimeter_x3, float perimeter_y3)
{
  int side;
  float length, max_length=0;

  for (side = 0; side < 4; side++) {
    length = get_side_length(side,
                             perimeter_x0, perimeter_y0,
                             perimeter_x1, perimeter_y1,
                             perimeter_x2, perimeter_y2,
                             perimeter_x3, perimeter_y3);
    if (length > max_length) max_length = length;
  }
  return max_length;
}

/**
 * \brief get the shortest side of a perimeter
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \return shortest side length
 */
float get_shortest_side(float perimeter_x0, float perimeter_y0,
                        float perimeter_x1, float perimeter_y1,
                        float perimeter_x2, float perimeter_y2,
                        float perimeter_x3, float perimeter_y3)
{
  int side;
  float length, min_length=-1;

  for (side = 0; side < 4; side++) {
    length = get_side_length(side,
                             perimeter_x0, perimeter_y0,
                             perimeter_x1, perimeter_y1,
                             perimeter_x2, perimeter_y2,
                             perimeter_x3, perimeter_y3);
    if ((side == 0) || (length < min_length)) min_length = length;
  }
  return min_length;
}

/**
 * \brief get the centre of a perimeter
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param centre_x returned x coordinate for the centroid
 * \param centre_y returned y coordinate for the centroid
 */
void get_centroid(float perimeter_x0, float perimeter_y0,
                  float perimeter_x1, float perimeter_y1,
                  float perimeter_x2, float perimeter_y2,
                  float perimeter_x3, float perimeter_y3,
                  float * centre_x, float * centre_y)
{
  *centre_x = (perimeter_x0 + perimeter_x1 + perimeter_x2 + perimeter_x3) / 4;
  *centre_y = (perimeter_y0 + perimeter_y1 + perimeter_y2 + perimeter_y3) / 4;
}
