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

  char * translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
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

static char * hibc_secondary_data_flag(char result[], int start_index, int end_index)
{
  int i, date_offset;
  char * date_value = NULL;
  char * translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  translated_str[0] = 0;

  if (result[start_index+1] == '$') {
    /* $$  */
    if (result[start_index+2] == '+') {
      if ((result[start_index+3] >= '2') &&
          (result[start_index+3] <= '7')) {
        /* $$+2..$$+7 */
        date_offset = 4;

        switch(result[start_index+2]) {
        case '2': {
          date_value = data_id_convert_date("MMDDYY", &result[start_index+3]);
          date_offset = 4+6;
          break;
        }
        case '3': {
          date_value = data_id_convert_date("YYMMDD", &result[start_index+3]);
          date_offset = 4+6;
          break;
        }
        case '4': {
          date_value = data_id_convert_date("YYMMDDHH", &result[start_index+3]);
          date_offset = 4+8;
          break;
        }
        case '5': {
          date_value = data_id_convert_date("YYJJJ", &result[start_index+3]);
          date_offset = 4+5;
          break;
        }
        case '6': {
          date_value = data_id_convert_date("YYJJJHH", &result[start_index+3]);
          date_offset = 4+7;
          break;
        }
        case '7': {
          break;
        }
        }

        if (date_value != NULL) {
          decode_strcat(translated_str, "EXPIRY: ");
          decode_strcat(translated_str, date_value);
          free(date_value);
          decode_strcat_char(translated_str, '\n');
        }

        decode_strcat(translated_str, "SERIAL: ");
        for (i = start_index+date_offset; i < end_index; i++) {
          decode_strcat_char(translated_str, result[i]);
        }
        decode_strcat_char(translated_str, '\n');
      }
      else {
        /* $$+ */
        date_value = data_id_convert_date("MMYY", &result[start_index+3]);
        date_offset = 3+4;
        if (date_value != NULL) {
          decode_strcat(translated_str, "EXPIRY: ");
          decode_strcat(translated_str, date_value);
          free(date_value);
          decode_strcat_char(translated_str, '\n');
        }

        decode_strcat(translated_str, "SERIAL: ");
        for (i = start_index+date_offset; i < end_index; i++) {
          decode_strcat_char(translated_str, result[i]);
        }
        decode_strcat_char(translated_str, '\n');
      }
    }
    if ((result[start_index+2] >= '2') &&
        (result[start_index+2] <= '7')) {
      /* $$2..$$7 */
      date_offset = 3;

      switch(result[start_index+2]) {
      case '2': {
        date_value = data_id_convert_date("MMDDYY", &result[start_index+3]);
        date_offset = 3+6;
        break;
      }
      case '3': {
        date_value = data_id_convert_date("YYMMDD", &result[start_index+3]);
        date_offset = 3+6;
        break;
      }
      case '4': {
        date_value = data_id_convert_date("YYMMDDHH", &result[start_index+3]);
        date_offset = 3+8;
        break;
      }
      case '5': {
        date_value = data_id_convert_date("YYJJJ", &result[start_index+3]);
        date_offset = 3+5;
        break;
      }
      case '6': {
        date_value = data_id_convert_date("YYJJJHH", &result[start_index+3]);
        date_offset = 3+7;
        break;
      }
      case '7': {
        break;
      }
      }

      if (date_value != NULL) {
        decode_strcat(translated_str, "EXPIRY: ");
        decode_strcat(translated_str, date_value);
        free(date_value);
        decode_strcat_char(translated_str, '\n');
      }

      decode_strcat(translated_str, "LOT NUMBER: ");
      for (i = start_index+date_offset; i < end_index; i++) {
        decode_strcat_char(translated_str, result[i]);
      }
      decode_strcat_char(translated_str, '\n');
    }
    else {
      /* $$ */
      date_value = data_id_convert_date("MMYY", &result[start_index+3]);
      date_offset = 2+4;

      if (date_value != NULL) {
        decode_strcat(translated_str, "EXPIRY: ");
        decode_strcat(translated_str, date_value);
        free(date_value);
        decode_strcat_char(translated_str, '\n');
      }

      decode_strcat(translated_str, "LOT NUMBER: ");
      for (i = start_index+date_offset; i < end_index; i++) {
        decode_strcat_char(translated_str, result[i]);
      }
      decode_strcat_char(translated_str, '\n');
    }
  }
  else if (result[start_index+1] == '+') {
    /* $+ */
    date_offset = 2;
    decode_strcat(translated_str, "SERIAL: ");
    for (i = start_index+date_offset; i < end_index; i++) {
      decode_strcat_char(translated_str, result[i]);
    }
    decode_strcat_char(translated_str, '\n');
  }
  else {
    /* $ */
    decode_strcat(translated_str, "LOT NUMBER: ");
    for (i = start_index+1; i < end_index; i++) {
      decode_strcat_char(translated_str, result[i]);
    }
    decode_strcat_char(translated_str, '\n');
  }

  if (strlen(translated_str) > 0) return translated_str;
  free(translated_str);
  return NULL;
}

static char * hibc_secondary_data(char result[], int start_index, int end_index)
{
  int i;

  if (end_index - start_index < 4) return NULL;

  if (result[start_index] == '$') {
    return hibc_secondary_data_flag(result, start_index, end_index);
  }
  else {
    char * translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
    translated_str[0] = 0;

    char * data_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
    data_str[0] = 0;
    for (i = start_index; i < end_index; i++) {
      decode_strcat_char(data_str, result[i]);
    }

    char * id = (char*)safemalloc(5*sizeof(char));
    char * id_human_readable = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
    char * id_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));

    if (get_data_identifier(data_str, id, id_human_readable, id_value) == 1) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, id_human_readable);
      decode_strcat(translated_str, ": ");
      char * date_value = data_id_convert_date(id_human_readable, id_value);
      if (date_value != NULL) {
        decode_strcat(translated_str, date_value);
        free(date_value);
      }
      else {
        decode_strcat(translated_str, id_value);
      }
      decode_strcat_char(translated_str, '\n');
      free(data_str);
      free(id);
      free(id_human_readable);
      free(id_value);
      return translated_str;
    }

    free(data_str);
    free(id);
    free(id_human_readable);
    free(id_value);
    free(translated_str);
  }

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
