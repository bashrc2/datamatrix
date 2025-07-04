/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix grid dimensions
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

int valid_squares[] = {
  10,  12,  14,  16,  18,  20,  22,  24,  26,  32, 36,  40,  44,  48,
  52,  64,  72,  80,  88,  96, 104, 120, 132, 144
};

int valid_rectangles[] = {
  8, 18,
  8, 32,
  12, 26,
  12, 36,
  16, 36,
  16, 48
};

/**
 * \brief returns an array of valid square dimensions as defined by IEC16022
 * \return array of square dimensions
 */
int * get_valid_squares()
{
  return valid_squares;
}

/**
 * \brief returns an array of valid rectangular dimensions as defined by IEC16022
 * \return array of rectangular dimensions
 */
int * get_valid_rectangles()
{
  return valid_rectangles;
}
