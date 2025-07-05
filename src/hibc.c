/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  HIBC semantics
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

static char * hibc_primary_data(char result[], int end_index)
{
  int i;

  if (end_index < 8) return NULL;

  char * translated_str = (char*)malloc(MAX_DECODE_LENGTH*sizeof(char));
  assert(translated_str);
  translated_str[0] = 0;

  /* first 4 characters */
  decode_strcat(translated_str, "LABELER ID: ");
  for (i = 1; i <= 4; i++) {
    decode_strcat_char(translated_str, result[i]);
  }
  decode_strcat_char(translated_str, '\n');

  decode_strcat(translated_str, "PRODUCT ID: ");
  for (i = 5; i < end_index-1; i++) {
    decode_strcat_char(translated_str, result[i]);
  }
  decode_strcat_char(translated_str, '\n');

  decode_strcat(translated_str, "UNIT OF MEASURE: ");
  decode_strcat_char(translated_str, result[end_index-1]);
  decode_strcat_char(translated_str, '\n');

  return translated_str;

}

static char * hibc_secondary_data(char result[], int start_index, int end_index)
{
  return NULL;
}

/**
 * \brief state machine for handling HIBC encoding
 * \param result Plaintext decode string
 * \param hibc_result decoded string
 * \param debug set to 1 to enable debugging
 */
void hibc_semantics(char result[],
                    char hibc_result[],
                    unsigned char debug)
{
  int i, index = 0;
  int str_len = strlen(result);
  unsigned char is_hibc = 0;
  int hibc_data_start = 0;
  char * translated_str = NULL;

  if (str_len == 0) return;

  /* HIBC Supplier Labeling flag */
  if ((result[0] == '+') && (is_hibc == 0)) {
    is_hibc = 1;
    hibc_result[0] = 0;
    hibc_data_start = 1;
    if (debug == 1) {
      printf("HIBC %s\n", result);
    }
  }

  if (is_hibc == 0) return;

  for (i = hibc_data_start; i < str_len; i++) {
    if (result[i] == '/') {
      if (index == 0) {
        translated_str = hibc_primary_data(result, i);
      }
      else {
        translated_str = hibc_secondary_data(result, hibc_data_start, i);
      }
      if (translated_str != NULL) {
        decode_strcat(hibc_result, translated_str);
        free(translated_str);
        index++;
      }
      hibc_data_start = i+1;
    }
  }

  if (index == 0) {
    translated_str = hibc_primary_data(result, str_len);
  }
  else {
    translated_str = hibc_secondary_data(result, hibc_data_start, str_len);
  }
  if (translated_str != NULL) {
    decode_strcat(hibc_result, translated_str);
    free(translated_str);
  }
}
