/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Shrink or grow a perimeter
 *  Copyright (c) 2025-2026, Bob Mottram
 *  bob@libreserver.org
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
 * \brief shrinks the perimeter after dilation
 * \param erosion_itterations number of erosion itterations
 * \param dilate_itterations number of dilation itterations
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 */
void shrinkwrap_shape_perimeter(const int erosion_itterations,
                                const int dilate_itterations,
                                float * perimeter_x0, float * perimeter_y0,
                                float * perimeter_x1, float * perimeter_y1,
                                float * perimeter_x2, float * perimeter_y2,
                                float * perimeter_x3, float * perimeter_y3)
{
    int dilation = (dilate_itterations - erosion_itterations)/2;
    float cx, cy, dx, dy, dist, fraction;
    if (dilation <= 0) return;

    cx = (*perimeter_x0 + *perimeter_x1 + *perimeter_x2 + *perimeter_x3)/4;
    cy = (*perimeter_y0 + *perimeter_y1 + *perimeter_y2 + *perimeter_y3)/4;

    dx = *perimeter_x0 - cx;
    dy = *perimeter_y0 - cy;
    dist = HYPOT(dx, dy);
    fraction = (dist - (float)dilation) / dist;
    *perimeter_x0 = cx + (dx * fraction);
    *perimeter_y0 = cy + (dy * fraction);

    dx = *perimeter_x1 - cx;
    dy = *perimeter_y1 - cy;
    dist = HYPOT(dx, dy);
    fraction = (dist - (float)dilation) / dist;
    *perimeter_x1 = cx + (dx * fraction);
    *perimeter_y1 = cy + (dy * fraction);

    dx = *perimeter_x2 - cx;
    dy = *perimeter_y2 - cy;
    dist = HYPOT(dx, dy);
    fraction = (dist - (float)dilation) / dist;
    *perimeter_x2 = cx + (dx * fraction);
    *perimeter_y2 = cy + (dy * fraction);

    dx = *perimeter_x3 - cx;
    dy = *perimeter_y3 - cy;
    dist = HYPOT(dx, dy);
    fraction = (dist - (float)dilation) / dist;
    *perimeter_x3 = cx + (dx * fraction);
    *perimeter_y3 = cy + (dy * fraction);
}

/**
 * \brief does at least one edge exist along a given line?
 * \param mono_edges_img mono image containing edges
 * \param width width of the image
 * \param height height of the image
 * \param tx start x coordinate of the line to be checked for edges
 * \param ty start y coordinate of the line to be checked for edges
 * \param bx end x coordinate of the line to be checked for edges
 * \param by end y coordinate of the line to be checked for edges
 * \return true is an edge exists on the line, false otherwise
 */
static bool line_has_edges(const unsigned char mono_edges_img[],
                           const int width, const int height,
                           const int tx, const int ty,
                           const int bx, const int by)
{
    int dx = bx - tx;
    int dy = by - ty;
    int x, y, n;

    if ((dx*dx > dy*dy) && (dx != 0)) {
        /* horizontal line */
        x = tx;
        while ((x != bx) && (x >= 0) && (x < width)) {
            y = ty + ((x - tx)*dy/dx);
            n = (y*width) + x;
            if (mono_edges_img[n] != 0) return true;
            if (dx > 0) x++;
            else x--;
        }
    }
    else if (dy != 0) {
        /* vertical line */
        y = ty;
        while ((y != by) && (y >= 0) && (y < height)) {
            x = tx + ((y - ty)*dx/dy);
            n = (y*width) + x;
            if (mono_edges_img[n] != 0) return true;
            if (dy > 0) y++;
            else y--;
        }
    }
    return false;
}

/**
 * \brief search for edges along a line in a particular direction
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param image_data original colour image data
 * \param image_bytesperpixel Number of bytes per pixel in the colour image
 * \param debug set to true if in debug mode
 * \param start_x start x coordinate for the direction to be searched
 * \param start_y start y coordinate for the direction to be searched
 * \param end_x end x coordinate for the direction to be searched
 * \param end_y end y coordinate for the direction to be searched
 * \param line_dx x component of the line vector to be searched for edges
 * \param line_dy y component of the line vector to be searched for edges
 * \param positive presence or absence of edges
 * \param pos_x returned x coordinate for the position where edges exist
 * \param pos_y returned y coordinate for the position where edges exist
 * \return 1 if the line has edges, 0 otherwise
 */
static unsigned char search_line_points(const unsigned char mono_img[],
                                        const int width, const int height,
                                        unsigned char image_data[],
                                        const int image_bytesperpixel,
                                        const bool debug,
                                        const int start_x, const int start_y,
                                        const int end_x, const int end_y,
                                        const int line_dx, const int line_dy,
                                        const unsigned char positive,
                                        int * pos_x, int * pos_y)
{
    int x, y, n, retval;
    int search_dx = end_x - start_x;
    int search_dy = end_y - start_y;
    int d, dist = (int)sqrt(SQUARE_MAG(search_dx, search_dy));
    int line_tx, line_ty, line_bx, line_by;

    *pos_x = -1;
    *pos_y = -1;

    for (d = 0; d < dist; d++) {
        x = start_x + (d*search_dx/dist);
        y = start_y + (d*search_dy/dist);
        if (debug) {
            n = (y*width + x)*image_bytesperpixel;
            image_data[n] = 0;
            image_data[n+1] = 255;
            image_data[n+2] = 0;
        }
        line_tx = x - line_dx;
        line_ty = y - line_dy;
        line_bx = x + line_dx;
        line_by = y + line_dy;
        retval = line_has_edges(mono_img, width, height,
                                line_tx, line_ty, line_bx, line_by);
        if (((positive != 0) && (retval)) ||
                ((positive == 0) && (!retval))) {
            *pos_x = x;
            *pos_y = y;
            if (debug) {
                n = (y*width + x)*image_bytesperpixel;
                image_data[n] = 255;
                image_data[n+1] = 0;
                image_data[n+2] = 0;
            }
            return 1;
        }
    }
    return 0;
}

/**
 * \brief try to expand each side outwards until no edges are encountered
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param max_extension_percent
 * \param debug set to true if in debug mode
 * \param image_data original colour image data
 * \param image_bitsperpixel Number of bits per pixel in the colour image
 * \return true if expanded, false otherwise
 */
bool expand_perimeter_sides(const unsigned char mono_img[],
                            const int width, const int height,
                            float * perimeter_x0, float * perimeter_y0,
                            float * perimeter_x1, float * perimeter_y1,
                            float * perimeter_x2, float * perimeter_y2,
                            float * perimeter_x3, float * perimeter_y3,
                            const int max_extension_percent,
                            const bool debug,
                            unsigned char image_data[],
                            const int image_bitsperpixel)
{
    int side, pos_x, pos_y;
    float tx, ty, bx, by, dx, dy, cx=0, cy=0, mid_point_x, mid_point_y;
    float half_dx, half_dy;
    float direction_x, direction_y, offset_x, offset_y;
    float direction_extended_x, direction_extended_y;
    float expanded_tx=0, expanded_ty=0, expanded_bx=0, expanded_by=0;
    bool expanded=false, grown=false;
    int image_bytesperpixel = image_bitsperpixel/8;

    /* get the centre point of the perimeter */
    get_centroid(*perimeter_x0, *perimeter_y0,
                 *perimeter_x1, *perimeter_y1,
                 *perimeter_x2, *perimeter_y2,
                 *perimeter_x3, *perimeter_y3,
                 &cx, &cy);

    for (side = 0; side < 4; side++) {
        expanded = false;
        expanded_tx = -1;

        /* get the coords for a side */
        switch(side) {
        case 0: {
            tx = *perimeter_x0;
            ty = *perimeter_y0;
            bx = *perimeter_x1;
            by = *perimeter_y1;
            break;
        }
        case 1: {
            tx = *perimeter_x1;
            ty = *perimeter_y1;
            bx = *perimeter_x2;
            by = *perimeter_y2;
            break;
        }
        case 2: {
            tx = *perimeter_x2;
            ty = *perimeter_y2;
            bx = *perimeter_x3;
            by = *perimeter_y3;
            break;
        }
        case 3: {
            tx = *perimeter_x3;
            ty = *perimeter_y3;
            bx = *perimeter_x0;
            by = *perimeter_y0;
            break;
        }
        }

        /* side vector */
        dx = bx - tx;
        dy = by - ty;
        half_dx = dx/2;
        half_dy = dy/2;
        /* mid point of the side */
        mid_point_x = tx + half_dx;
        mid_point_y = ty + half_dy;
        /* outward direction vector */
        direction_x = mid_point_x - cx;
        direction_y = mid_point_y - cy;
        /* extended outwards further */
        direction_extended_x = cx +
                               (direction_x * (float)max_extension_percent / 100.0f);
        direction_extended_y = cy +
                               (direction_y * (float)max_extension_percent / 100.0f);

        pos_x = -1;
        pos_y = -1;
        search_line_points(mono_img, width, height,
                           image_data, image_bytesperpixel, debug,
                           (int)mid_point_x, (int)mid_point_y,
                           (int)direction_extended_x, (int)direction_extended_y,
                           (int)half_dx, (int)half_dy, 0, &pos_x, &pos_y);
        if (pos_x != -1) {
            offset_x = (float)pos_x - mid_point_x;
            offset_y = (float)pos_y - mid_point_y;
            expanded_tx = tx + offset_x;
            expanded_ty = ty + offset_y;
            expanded_bx = bx + offset_x;
            expanded_by = by + offset_y;
            expanded = true;
        }

        if (!expanded) continue;
        if (expanded_tx == -1) continue;

        switch(side) {
        case 0: {
            *perimeter_x0 = expanded_tx;
            *perimeter_y0 = expanded_ty;
            *perimeter_x1 = expanded_bx;
            *perimeter_y1 = expanded_by;
            break;
        }
        case 1: {
            *perimeter_x1 = expanded_tx;
            *perimeter_y1 = expanded_ty;
            *perimeter_x2 = expanded_bx;
            *perimeter_y2 = expanded_by;
            break;
        }
        case 2: {
            *perimeter_x2 = expanded_tx;
            *perimeter_y2 = expanded_ty;
            *perimeter_x3 = expanded_bx;
            *perimeter_y3 = expanded_by;
            break;
        }
        case 3: {
            *perimeter_x3 = expanded_tx;
            *perimeter_y3 = expanded_ty;
            *perimeter_x0 = expanded_bx;
            *perimeter_y0 = expanded_by;
            break;
        }
        }
        grown = true;
    }

    return grown;
}

/**
 * \brief try to fit each side to edges
 * \param mono_img mono image array
 * \param width width of the image
 * \param height height of the image
 * \param perimeter_x0 first perimeter x coord
 * \param perimeter_y0 first perimeter y coord
 * \param perimeter_x1 second perimeter x coord
 * \param perimeter_y1 second perimeter y coord
 * \param perimeter_x2 third perimeter x coord
 * \param perimeter_y2 third perimeter y coord
 * \param perimeter_x3 fourth perimeter x coord
 * \param perimeter_y3 fourth perimeter y coord
 * \param min_extension_percent
 * \param image_data original colour image data
 * \param image_bitsperpixel Number of bits per pixel in the colour image
 * \return true if contracted, false otherwise
 */
bool contract_perimeter_sides(const unsigned char mono_img[],
                              const int width, const int height,
                              float * perimeter_x0,
                              float * perimeter_y0,
                              float * perimeter_x1,
                              float * perimeter_y1,
                              float * perimeter_x2,
                              float * perimeter_y2,
                              float * perimeter_x3,
                              float * perimeter_y3,
                              const int min_extension_percent,
                              const bool debug,
                              unsigned char image_data[],
                              const int image_bitsperpixel)
{
    int side, left_x, left_y, right_x, right_y;
    float tx, ty, bx, by, xi, yi;
    float prev_tx, prev_ty, prev_bx, prev_by;
    float next_tx, next_ty, next_bx, next_by;
    float dx, dy, cx=0, cy=0, mid_point_x, mid_point_y;
    int dx_quarter, dy_quarter;
    float left_mid_point_x, left_mid_point_y;
    float right_mid_point_x, right_mid_point_y;
    float direction_x, direction_y;
    float direction_contracted_right_x, direction_contracted_right_y;
    float direction_contracted_left_x, direction_contracted_left_y;
    bool contracted=false;
    int image_bytesperpixel = image_bitsperpixel/8;

    /* get the centre point of the perimeter */
    get_centroid(*perimeter_x0, *perimeter_y0,
                 *perimeter_x1, *perimeter_y1,
                 *perimeter_x2, *perimeter_y2,
                 *perimeter_x3, *perimeter_y3,
                 &cx, &cy);

    for (side = 0; side < 4; side++) {
        /* get the coords for a side */
        switch(side) {
        case 0: {
            prev_tx = *perimeter_x3;
            prev_ty = *perimeter_y3;
            prev_bx = *perimeter_x0;
            prev_by = *perimeter_y0;

            tx = *perimeter_x0;
            ty = *perimeter_y0;
            bx = *perimeter_x1;
            by = *perimeter_y1;

            next_tx = *perimeter_x1;
            next_ty = *perimeter_y1;
            next_bx = *perimeter_x2;
            next_by = *perimeter_y2;
            break;
        }
        case 1: {
            prev_tx = *perimeter_x0;
            prev_ty = *perimeter_y0;
            prev_bx = *perimeter_x1;
            prev_by = *perimeter_y1;

            tx = *perimeter_x1;
            ty = *perimeter_y1;
            bx = *perimeter_x2;
            by = *perimeter_y2;

            next_tx = *perimeter_x2;
            next_ty = *perimeter_y2;
            next_bx = *perimeter_x3;
            next_by = *perimeter_y3;
            break;
        }
        case 2: {
            prev_tx = *perimeter_x1;
            prev_ty = *perimeter_y1;
            prev_bx = *perimeter_x2;
            prev_by = *perimeter_y2;

            tx = *perimeter_x2;
            ty = *perimeter_y2;
            bx = *perimeter_x3;
            by = *perimeter_y3;

            next_tx = *perimeter_x3;
            next_ty = *perimeter_y3;
            next_bx = *perimeter_x0;
            next_by = *perimeter_y0;
            break;
        }
        case 3: {
            prev_tx = *perimeter_x2;
            prev_ty = *perimeter_y2;
            prev_bx = *perimeter_x3;
            prev_by = *perimeter_y3;

            tx = *perimeter_x3;
            ty = *perimeter_y3;
            bx = *perimeter_x0;
            by = *perimeter_y0;

            next_tx = *perimeter_x0;
            next_ty = *perimeter_y0;
            next_bx = *perimeter_x1;
            next_by = *perimeter_y1;
            break;
        }
        }

        /* side vector */
        dx = bx - tx;
        dy = by - ty;
        /* mid point of the side */
        mid_point_x = tx + (dx/2);
        mid_point_y = ty + (dy/2);
        /* left mid point of the side */
        left_mid_point_x = tx + (dx/4);
        left_mid_point_y = ty + (dy/4);
        /* right mid point of the side */
        right_mid_point_x = tx + (dx*3/4);
        right_mid_point_y = ty + (dy*3/4);
        /* inward direction vector */
        direction_x = mid_point_x - cx;
        direction_y = mid_point_y - cy;
        /* left contracted inwards further */
        direction_contracted_left_x =
            left_mid_point_x -
            (direction_x * (float)min_extension_percent / 100.0f);
        direction_contracted_left_y =
            left_mid_point_y -
            (direction_y * (float)min_extension_percent / 100.0f);
        /* right contracted inwards further */
        direction_contracted_right_x =
            right_mid_point_x -
            (direction_x * (float)min_extension_percent / 100.0f);
        direction_contracted_right_y =
            right_mid_point_y -
            (direction_y * (float)min_extension_percent / 100.0f);
        /* left travel to be searched */
        dx_quarter = (int)(dx / 4);
        dy_quarter = (int)(dy / 4);
        left_x = -1;
        search_line_points(mono_img, width, height,
                           image_data, image_bytesperpixel, debug,
                           (int)left_mid_point_x, (int)left_mid_point_y,
                           (int)direction_contracted_left_x,
                           (int)direction_contracted_left_y,
                           dx_quarter, dy_quarter, 1, &left_x, &left_y);

        /* right travel to be searched */
        right_x = -1;
        search_line_points(mono_img, width, height,
                           image_data, image_bytesperpixel, debug,
                           (int)right_mid_point_x, (int)right_mid_point_y,
                           (int)direction_contracted_right_x,
                           (int)direction_contracted_right_y,
                           dx_quarter, dy_quarter, 1, &right_x, &right_y);
        if ((left_x == -1) && (right_x == -1)) continue;
        contracted = true;
        if (left_x == -1) {
            left_x = (int)left_mid_point_x;
            left_y = (int)left_mid_point_y;
        }
        if (right_x == -1) {
            right_x = (int)right_mid_point_x;
            right_y = (int)right_mid_point_y;
        }
        /* find the intercept between this line and the previous one */
        xi = -1;
        yi = -1;
        intersection((float)left_x, (float)left_y, (float)right_x, (float)right_y,
                     prev_tx, prev_ty, prev_bx, prev_by,
                     &xi, &yi);
        if (xi != -1) {
            switch(side) {
            case 0: {
                *perimeter_x0 = xi;
                *perimeter_y0 = yi;
                break;
            }
            case 1: {
                *perimeter_x1 = xi;
                *perimeter_y1 = yi;
                break;
            }
            case 2: {
                *perimeter_x2 = xi;
                *perimeter_y2 = yi;
                break;
            }
            case 3: {
                *perimeter_x3 = xi;
                *perimeter_y3 = yi;
                break;
            }
            }
        }
        /* find the intercept between this line and the next one */
        xi = -1;
        yi = -1;
        intersection((float)left_x, (float)left_y, (float)right_x, (float)right_y,
                     next_tx, next_ty, next_bx, next_by,
                     &xi, &yi);
        if (xi != -1) {
            switch(side) {
            case 0: {
                *perimeter_x1 = xi;
                *perimeter_y1 = yi;
                break;
            }
            case 1: {
                *perimeter_x2 = xi;
                *perimeter_y2 = yi;
                break;
            }
            case 2: {
                *perimeter_x3 = xi;
                *perimeter_y3 = yi;
                break;
            }
            case 3: {
                *perimeter_x0 = xi;
                *perimeter_y0 = yi;
                break;
            }
            }
        }
    }

    return contracted;
}
