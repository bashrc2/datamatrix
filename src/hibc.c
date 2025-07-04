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

/**
 * \brief
 * \param result string decoded so far
 * \param start_pos starting position in the result string
 * \param end_index ending position in the result string
 * \return translated string
 */
static char * hibc_translate(char result[],
                             int start_pos,
                             int end_index)
{
  int i;
  char * translated_str = NULL;

  /* get the field string */
  char * data_str = (char*)malloc(MAX_DECODE_LENGTH*sizeof(char));
  assert(data_str != NULL);
  data_str[0] = 0;
  for (i = start_pos; i < end_index; i++) {
    decode_strcat_char(data_str, result[i]);
  }

  /* does it have a data identifier? */
  char * id = (char*)malloc(5*sizeof(char));
  char * id_human_readable = (char*)malloc(MAX_DECODE_LENGTH*sizeof(char));
  char * id_value = (char*)malloc(MAX_DECODE_LENGTH*sizeof(char));
  assert(id != NULL);
  assert(id_human_readable != NULL);
  assert(id_value != NULL);

  if (get_data_identifier(data_str, id, id_human_readable, id_value) == 1) {
    translated_str = (char*)malloc(MAX_DECODE_LENGTH*sizeof(char));
    assert(translated_str);
    translated_str[0] = 0;
    decode_strcat(translated_str, id_human_readable);
    decode_strcat(translated_str, ": ");
    decode_strcat(translated_str, id_value);
  }

  free(data_str);
  free(id);
  free(id_human_readable);
  free(id_value);
  return translated_str;
}

/**
 * \brief state machine for handling HIBC encoding
 * \param result Plaintext decode string
 * \param hibc_result decoded string
 * \param debug set to 1 to enable debugging
 * \param is_hibc set to 1 if hibc decoding is active
 * \param hibc_data_start position of the start of data within result string
 */
void hibc_semantics(char result[],
                    char hibc_result[],
                    unsigned char debug,
                    unsigned char * is_hibc,
                    int * hibc_data_start)
{
  int i;
  int str_len = strlen(result);

  if (str_len == 0) return;

  /* HIBC Supplier Labeling flag */
  if ((result[0] == '+') && (*is_hibc == 0)) {
    *is_hibc = 1;
    hibc_result[0] = 0;
    *hibc_data_start = 1;
    if (debug == 1) {
      printf("HIBC\n");
    }
  }

  if (*is_hibc == 0) return;

  for (i = *hibc_data_start; i < str_len; i++) {
    if (result[i] == '/') {
      char * translated_str = hibc_translate(result, *hibc_data_start, i);
      if (translated_str != NULL) {
        decode_strcat(hibc_result, translated_str);
        decode_strcat_char(hibc_result, '\n');
        free(translated_str);
      }
      *hibc_data_start = i+1;
    }
  }
}
