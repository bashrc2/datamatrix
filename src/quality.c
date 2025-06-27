/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix quality metrics
 *  Copyright (c) 2025, Bob Mottram
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

/* AS9132 calculate angle of distortion */
static void quality_metric_angle_of_distortion(struct grid_2d * grid)
{
  float corner_radians =
    corner_angle(grid->perimeter.x0, grid->perimeter.y0,
                 grid->perimeter.x1, grid->perimeter.y1,
                 grid->perimeter.x2, grid->perimeter.y2);
  if (corner_radians < 0) corner_radians = -corner_radians;
  if (corner_radians > PI)
    corner_radians = (2 * (float)PI) - corner_radians;
  float angle_degrees = corner_radians / (float)PI * 180;
  grid->angle_of_distortion = 90 - angle_degrees;
}

void calculate_quality_metrics(struct grid_2d * grid)
{
  quality_metric_angle_of_distortion(grid);
}

void show_quality_metrics(struct grid_2d * grid)
{
  printf("Fixed pattern damage: %d%%\n", (int)grid->fixed_pattern_damage);
  printf("Angle of distortion: %.1f degrees\n", grid->angle_of_distortion);
}
