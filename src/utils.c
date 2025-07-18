/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Utility functions
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

void * safemalloc(int n)
{
  void * ptr = malloc(n);
  if (!ptr) {
    fprintf(stderr, "Malloc(%d) failed\n", n);
    exit(1);
  }
  return ptr;
}

/**
 * \brief a safe version of strcat which avoids buffer overrun
 * \param result the string to be added to
 * \param text the string to be added */
int decode_strcat(char * result, char * text)
{
  if (strlen(result) + strlen(text) + sizeof(char) >= MAX_DECODE_LENGTH)
    return -1;
  strcat(result, text);
  return 0;
}

/**
 * \brief add a single character to a string
 * \param result the string to be added to
 * \param chr the character to be added */
int decode_strcat_char(char * result, char chr)
{
  int length = strlen(result);
  if (length + (2*sizeof(char)) >= MAX_DECODE_LENGTH)
    return -1;
  result[length++] = chr;
  result[length] = 0;
  return 0;
}

/**
 * \brief simplified implementation of gnu99's getline
 * \param line to be returned
 * \param fp file pointer
 * \return 0 if a line has been read or -1 if at the end of file
 */
int getline2(char line[], FILE * fp)
{
  int i = 0;
  char c = '.';

  line[0] = 0;

  if (fp == NULL){
    return -1;
  }

  while (c != '\n') {
    if (i >= MAX_DECODE_LENGTH - 1) {
      line[i++] = 0;
      break;
    }
    c = getc(fp);
    if (c == EOF) {
      line[i] = 0;
      break;
    }
    line[i] = c;
    i++;
  }

  if (i > 0) {
    line[i] = 0;
  }
  if (c == EOF) return -1;
  return 0;
}
