/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  GS1 semantics functions
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
 * \brief translates the abbreviated data qualifier to something human readable
 * \param result string decoded so far
 * \param start_index starting position in the result string
 * \param end_index ending position in the result string
 * \param iso15434_uii returned unique item identifier
 * \param format_code format code
 * \param debug set to 1 for debugging
 * \return translated string
 */
char * iso15434_translate_data_qualifier(char result[],
                                         int start_index,
                                         int end_index,
                                         char iso15434_uii[],
                                         char format_code[],
                                         unsigned char debug)
{
  int i, start_pos=0;
  char * translated_str = NULL;
  unsigned char found = 0;

  if (end_index - start_index < 4) return NULL;

  if ((strcmp(format_code, "12") == 0) ||
      (strcmp(format_code, "DD") == 0)) {
    if (debug == 1) {
      printf("Format code %s\n", format_code);
    }
    /* MFR */
    if ((result[start_index] == 'M') &&
        (result[start_index+1] == 'F') &&
        (result[start_index+2] == 'R')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "MANUFACTURER/CAGE: ");
      found = 1;
    }

    /* SPL */
    if ((result[start_index] == 'S') &&
        (result[start_index+1] == 'P') &&
        (result[start_index+2] == 'L')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "SUPPLIER/CAGE: ");
      found = 1;
    }

    /* SER / SEQ */
    if ((result[start_index] == 'S') &&
        (result[start_index+1] == 'E') &&
        ((result[start_index+2] == 'R') || (result[start_index+2] == 'Q'))) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "SERIAL: ");
      found = 1;
    }

    /* CAG */
    if ((result[start_index] == 'C') &&
        (result[start_index+1] == 'A') &&
        (result[start_index+2] == 'G')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "CAGE: ");
      found = 1;
    }

    /* PNO */
    if ((result[start_index] == 'P') &&
        (result[start_index+1] == 'N') &&
        (result[start_index+2] == 'O')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "PART NUMBER: ");
      found = 1;
    }

    /* DUN */
    if ((result[start_index] == 'D') &&
        (result[start_index+1] == 'U') &&
        (result[start_index+2] == 'N')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "DISTRIBUTION UNIT NUMBER: ");
      found = 1;
    }

    /* UID / UIC */
    if ((result[start_index] == 'U') &&
        (result[start_index+1] == 'I') &&
        ((result[start_index+2] == 'D') || (result[start_index+2] == 'C'))) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "UNIQUE ID: ");
      found = 1;
    }

    /* USN */
    if ((result[start_index] == 'U') &&
        (result[start_index+1] == 'S') &&
        (result[start_index+2] == 'N')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "UNIQUE SERIAL: ");
      found = 1;
    }

    /* UST */
    if ((result[start_index] == 'U') &&
        (result[start_index+1] == 'S') &&
        (result[start_index+2] == 'T')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "UNITS: ");
      found = 1;
    }

    /* FID */
    if ((result[start_index] == 'F') &&
        (result[start_index+1] == 'I') &&
        (result[start_index+2] == 'D')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "FACILITY: ");
      found = 1;
    }

    /* EOID */
    if ((result[start_index] == 'E') &&
        (result[start_index+1] == 'O') &&
        (result[start_index+2] == 'I') &&
        (result[start_index+3] == 'D')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "OPERATOR: ");
      found = 1;
    }

    /* MID */
    if ((result[start_index] == 'M') &&
        (result[start_index+1] == 'I') &&
        (result[start_index+2] == 'D')) {
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;
      decode_strcat(translated_str, "MACHINE: ");
      found = 1;
    }
  }

  if (found == 1) {
    /* position to begin reading at */
    if (result[start_index+4] == ' ') {
      start_pos = start_index+5;
    }
    else if (result[start_index+3] == ' ') {
      start_pos = start_index+4;
    }
    else {
      start_pos = start_index+3;
    }

    for (i = start_pos; i < end_index; i++) {
      decode_strcat_char(translated_str, result[i]);

      decode_strcat_char(iso15434_uii, result[i]);
    }
    return translated_str;
  }
  else {
    /* format code 06  */
    if (strcmp(format_code, "06") == 0) {
      if (debug == 1) {
        printf("Format code 06\n");
      }
      char * data_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      data_str[0] = 0;
      for (i = start_pos; i < end_index; i++) {
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
        decode_strcat(iso15434_uii, id_value);
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
    }
    else if (strcmp(format_code, "05") == 0) {
      if (debug == 1) {
        printf("Format code 05\n");
      }
      translated_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      translated_str[0] = 0;

      unsigned char application_identifier_length = 4;
      for (unsigned char app_id_len = 4; app_id_len >= 2; app_id_len--) {
        application_identifier_length = app_id_len;
        if (end_index - start_pos > application_identifier_length) {
          /* get the application identifier */
          char app_id_str[5];
          for (i = start_pos; i < start_pos + application_identifier_length; i++) {
            app_id_str[i] = result[i];
          }
          app_id_str[application_identifier_length] = 0;
          if (debug == 1) {
            printf("Application Identifier %s\n", &app_id_str[0]);
          }
          int application_identifier = -1;
          int application_data_start = start_pos;
          int application_data_end = start_pos + application_identifier_length;
          gs1_semantics(&app_id_str[0], translated_str,
                        NULL, debug,
                        &application_identifier,
                        &application_identifier_length,
                        &application_data_start,
                        &application_data_end);
          if (debug == 1) {
            printf("result: %d %s\n", application_identifier, result);
          }
          if (application_identifier != -1) {
            application_data_end =
              application_data_start + strlen(result) - application_identifier_length;
            gs1_semantics(result, translated_str,
                          NULL, debug,
                          &application_identifier,
                          &application_identifier_length,
                          &application_data_start,
                          &application_data_end);
            if (debug == 1) {
              printf("translated_str 1 %d: %s\n", i, translated_str);
            }
            if (strlen(translated_str) > 0) {
              /* remove trailing newline */
              translated_str[strlen(translated_str)-1] = 0;
              /* update the UII */
              for (i = start_pos + application_identifier_length; i < end_index; i++) {
                decode_strcat_char(iso15434_uii, result[i]);
              }
              break;
            }
          }
        }
      }
      if (strlen(translated_str) == 0) {
        application_identifier_length = 4;
        for (i = start_pos; i < end_index; i++) {
          decode_strcat_char(translated_str, result[i]);
          /* format 05, miss the first 4 characters */
          if (i >= start_pos + application_identifier_length) {
            decode_strcat_char(iso15434_uii, result[i]);
          }
        }
      }
    }
  }

  return translated_str;
}

/**
 * \brief state machine for handling ISO 15434 encoding
 * \param result Plaintext decode string
 * \param iso15434_result decoded string
 * \param debug set to 1 to enable debugging
 * \param is_iso1543 set to 1 if iso1543 decoding is active
 * \param format_code returned iso1543 format code
 * \param iso15434_data_start position of the start of data within result string
 * \param iso15434_uii returned unique item identifier
 */
void iso15434_semantics(char result[],
                        char iso15434_result[],
                        unsigned char debug,
                        unsigned char * is_iso1543,
                        char format_code[],
                        int * iso15434_data_start,
                        char iso15434_uii[])
{
  int prev_char_value, char_value, i, j;
  int str_len = strlen(result);
  char * translated_str;
  const int CHAR_GS = 29;
  const int CHAR_RS = 30;
  const int CHAR_EOT = 4;

  if (str_len < 3) {
    *is_iso1543 = 0;
    *iso15434_data_start = -1;
    format_code[0] = 0;
    iso15434_result[0] = 0;
    return;
  }

  /* EOT */
  if ((str_len > 1) && (*is_iso1543 == 1)) {
    if ((int)(result[str_len-1]) == CHAR_EOT) {
      if (debug == 1) {
        printf("EOT\n");
      }
      return;
    }
  }

  /* look for the beginning */
  if (str_len == 3) {
    if (strcmp(result, "[)>") == 0) {
      *is_iso1543 = 1;
      if (debug == 1) {
        printf("Beginning of ISO 1543\n");
      }
    }
    return;
  }
  if (*is_iso1543 == 0) return;

  char_value = (int)(result[str_len-1]);

  if (str_len == 4) {
    /* RS */
    if (char_value != CHAR_RS) {
      *is_iso1543 = 0;
      *iso15434_data_start = -1;
      format_code[0] = 0;
      iso15434_result[0] = 0;
    }
    else {
      *is_iso1543 = 1;
      *iso15434_data_start = str_len;
      if (debug == 1) {
        printf("ISO 15434\n");
      }
      if (strstr(iso15434_result, "STANDARD: ") == NULL) {
        decode_strcat(iso15434_result, "STANDARD: ISO15434\n");
      }
    }
    return;
  }

  if ((char_value == CHAR_GS) ||
      (char_value == CHAR_RS)) {
    /* GS or RS */
    for (i = *iso15434_data_start; i < strlen(result); i++) {
      prev_char_value = (int)(result[i]);
      if ((prev_char_value == CHAR_GS) ||
          (prev_char_value == CHAR_RS)) {
        if (strlen(format_code) == 0) {
          for (j = *iso15434_data_start; j < i; j++) {
            decode_strcat_char(format_code, result[j]);
          }
          if ((strcmp(format_code, "12") != 0) &&
              (strcmp(format_code, "DD") != 0) &&
              (strcmp(format_code, "06") != 0) &&
              (strcmp(format_code, "05") != 0)) {
            /* not in a permitted swim lane */
            *is_iso1543 = 0;
            *iso15434_data_start = -1;
            format_code[0] = 0;
            iso15434_result[0] = 0;
            return;
          }

          decode_strcat(iso15434_result, "FORMAT: ");
          decode_strcat(iso15434_result, format_code);
        }
        else {
          translated_str =
            iso15434_translate_data_qualifier(result,
                                              *iso15434_data_start, i,
                                              iso15434_uii,
                                              format_code,
                                              debug);
          if (translated_str != NULL) {
            decode_strcat(iso15434_result, translated_str);
            free(translated_str);
          }
          else {
            for (j = *iso15434_data_start; j < i; j++) {
              decode_strcat_char(iso15434_result, result[j]);
            }
          }
        }
        decode_strcat_char(iso15434_result, '\n');
        *iso15434_data_start = i+1;
      }
    }
    *iso15434_data_start = str_len;
  }
}
