/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Help functions
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

void show_help()
{
  printf("datamatrix command options:\n\n");
  printf("-f --filename [PNG file]      Filename of a PNG file containing the datamatrix image\n");
  printf("--square                      Matrix is expected to be square\n");
  printf("--rectangle                   Matrix is expected to be rectangular\n");
  printf("--quality                     Show quality metrics for verification reports\n");
  printf("-o --output [PNG file]        Save an image showing the detected datamatrix\n");
  printf("-g --grid [PNG file]          Save an image showing the detected grid\n");
  printf("--hist [PNG file]             Save reflectance histogram\n");
  printf("--shape [PNG file]            Save cell shape variance image\n");
  printf("-t --template [TEX file]      Path for verification report template\n");
  printf("-r --report [TEX file]        Filename to save a verification report as");
  printf("--histcentres                 Only sample grid cell centres for reflectance histogram\n");
  printf("--mingrid [8..144]            Minimum matrix dimension\n");
  printf("--maxgrid [8..144]            Maximum matrix dimension\n");
  printf("--debug                       Show debugging information\n");
  printf("--csv                         Show quality metrics in CSV format\n");
  printf("--json                        Show quality metrics in JSON format\n");
  printf("--yaml                        Show quality metrics in yaml format\n");
  printf("--freq [8..144]               Set the matrix dimension\n");
  printf("--erode [0..20]               Set erosion itterations\n");
  printf("--dilate [0..20]              Set dilation itterations\n");
  printf("--meanlight [0..20]           Set mean light threshold\n");
  printf("--resolver [url]              GS1 resolver (eg. https://id.gs1.org)\n");
  printf("--sampleradius [pixels]       Radius for sampling each grid cell\n");
  printf("--minsegmentlength [pixels]   Minimum edge segment length for segment joining\n");
  printf("--resizewidth [pixels]        Width of image after resizing from original\n");
  printf("--resizeheight [pixels]       Height of image after resizing from original\n");
  printf("--binwidth [pixels]           Width of the binary image used for perimeter detection\n");
  printf("--raw                         Show the raw decoded text\n");
  printf("--aperture [ref]              Aperture reference number from ISO 15416\n");
  printf("--light [nm]                  Peak light wavelength used in nanometres\n");
  printf("--angle [90|45]               Angle of illumination in degrees\n");
  printf("--tests                       Run unit tests\n");
}
