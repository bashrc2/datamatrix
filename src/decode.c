/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix decoding functions
 *  Copyright (c) 2025, Bob Mottram
 *  bob@libreserver.org
 *
 *  This is partly based on, or inspired by, an ECC kernel module
 *  by Fabrice Bellard <fabrice.bellard@netgem.com>
 *  "ECC code for correcting errors detected by DiskOnChip 2000 and
 *  Millennium ECC hardware" written in 2000 under GPLv2 license
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

/* encoding types */
#define ASCII   0
#define C40     1
#define X12     2
#define TEXT    3
#define EDIFACT 4
#define BYTE256 5

static void locate_erasures(int damage_pattern_x,
                            int damage_pattern_y,
                            int * codeword_pattern[],
                            unsigned char damage_pattern[],
                            int erasures[],
                            int * erasures_length)
{
  *erasures_length = 0;

  if (damage_pattern == NULL) return;
  for (int y = 0; y < damage_pattern_y; y++) {
    for (int x = 0; x < damage_pattern_x; x++) {
      if (damage_pattern[y*damage_pattern_x + x]) {
        erasures[*erasures_length] = codeword_pattern[x][y];
        *erasures_length = (*erasures_length) + 1;
      }
    }
  }
}

static unsigned char ecc200_unrandomize_255_state(unsigned char value, int idx)
{
  int pseudoRandom = ((149 * (idx + 1)) % 255) + 1;
  int tmp = value - pseudoRandom;
  return (unsigned char)((tmp >= 0) ? tmp : tmp + 256);
}

static void ecc200_decode_next_byte_256(unsigned char * is_structured_append,
                                        unsigned char is_gs1_encodation,
                                        unsigned char data[],
                                        int datalength,
                                        int * position,
                                        int * state,
                                        int * shift,
                                        char result[],
                                        char gs1_result[],
                                        char iso15434_result[],
                                        unsigned char * is_iso1543,
                                        char format_code[],
                                        int * iso15434_data_start,
                                        char iso15434_uii[])
{
  int i, d2, pos_from_start = *position;
  unsigned char d1 = ecc200_unrandomize_255_state(data[*position], pos_from_start++);
  int length = 0;
  char decoded_char;

  *position = (*position) + 1;

  if (d1 == 0) {
    length = datalength - (*position);
  }
  else if (d1 < 250) {
    length = d1;
  }
  else {
    d2 = ecc200_unrandomize_255_state(data[(*position) + 1], pos_from_start++);
    length = (d1 - 249) * 250 + d2;
    *position = (*position) + 1;
  }

  for (i = 0; i < length; i++) {
    if (*position >= datalength) {
      printf("An ECC200 decoding error occurred in ecc200_decode_next_byte_256\n");
    }
    else {
      decoded_char = (char)(ecc200_unrandomize_255_state(data[*position], pos_from_start++));
      decode_strcat_char(result, decoded_char);
      if (is_gs1_encodation == 1) {
        decode_strcat_char(gs1_result, decoded_char);
      }
      *position = (*position) + 1;
    }
  }
  *state = ASCII;
}

/**
 * \brief returns the percentage of unused error correction
 * \return unused error correction as a percentage
 */
static unsigned char get_unused_error_correction(int no_of_codewords,
                                                 int error_correction_codewords,
                                                 int * no_of_errors,
                                                 int * no_of_erasures)
{
  float unused_error_correction;

  const int modules_per_codeword = 8;
  float e2t = ((*no_of_erasures) + (2 * (*no_of_errors))) / modules_per_codeword;
  float Ecap = no_of_codewords - error_correction_codewords;
  if (e2t > Ecap) {
    unused_error_correction = 0;
  }
  else {
    unused_error_correction = 1.0f - (e2t / Ecap);
  }
  return (unsigned char)(unused_error_correction * 100);
}

/**
 * \brief ASCII encoding
 */
static void ecc200_decode_next_ascii(unsigned char * is_structured_append,
                                     unsigned char * is_gs1_encodation,
                                     int * application_identifier,
                                     unsigned char * application_identifier_length,
                                     int * application_data_start,
                                     int * application_data_end,
                                     unsigned char * application_data_variable,
                                     unsigned char data[],
                                     int datalength,
                                     int * position,
                                     int * state,
                                     int * shift,
                                     char result[],
                                     char gs1_result[],
                                     char gs1_url[],
                                     unsigned char debug,
                                     char iso15434_result[],
                                     unsigned char * is_iso1543,
                                     char format_code[],
                                     int * iso15434_data_start,
                                     char iso15434_uii[])
{
  int no, first_digit, last_digit;

  unsigned char current_byte = data[*position];
  if (debug == 1) {
    if (*position == 0) {
      printf("Initial byte: %d  ", (int)current_byte);
      if (current_byte == 232) {
        printf("GS1 ");
      }
    }
  }
  *position = (*position) + 1;
  if ((current_byte >= 1) && (current_byte <= 128)) {
    /* ASCII 0-127 */
    if (*shift == 0) {
      decode_strcat_char(result, (char)(current_byte - 1));
    }
    else {
      /* ASCII 127-255 */
      decode_strcat_char(result, (char)(current_byte - 1 + 127));
      *shift = 0;
    }
  }
  else if ((current_byte >= 130) && (current_byte <= 229)) {
    /* numbers 0-99 */
    no = current_byte - 130;
    first_digit = no / 10;
    last_digit = no % 10;
    decode_strcat_char(result, (char)(first_digit + '0'));
    decode_strcat_char(result, (char)(last_digit + '0'));
  }
  else if (current_byte == 230) {
    *state = C40;
  }
  else if (current_byte == 231) {
    *state = BYTE256;
    *shift = 0;
  }
  else if (current_byte == 238) {
    *state = X12;
    *shift = 0;
  }
  else if (current_byte == 239) {
    *state = TEXT;
    *shift = 0;
  }
  else if (current_byte == 240) {
    *state = EDIFACT;
    *shift = 0;
  }
  else if (current_byte == 232) {
    /* FNC1 */
    if (*position > 1) {
      decode_strcat_char(result, (char)(29));
    }
    else {
      /* GS1 encodation */
      *is_gs1_encodation = 1;
    }
  }
  else if (current_byte == 233) {
    *is_structured_append = 1;
  }
  else if (current_byte == 234) {
    /* reader programming */
  }
  else if (current_byte == 235) {
    *shift = 1;
  }
  else if (current_byte == 236) {
    /* 05 Macro */
  }
  else if (current_byte == 237) {
    /* 06 Macro */
  }
  else if ((current_byte == 129) ||
           (current_byte == 253)) {
    /* PAD */
    *position = datalength;
  }

  if (debug == 1) {
    printf("%d ", (int)current_byte);
  }

  /* GS1 semantics processing */
  if (*is_gs1_encodation == 1) {
    gs1_semantics(result, gs1_result, gs1_url, debug,
                  application_identifier,
                  application_identifier_length,
                  application_data_start,
                  application_data_end,
                  application_data_variable);
  }

  iso15434_semantics(result,
                     iso15434_result, debug,
                     is_iso1543,
                     format_code,
                     iso15434_data_start,
                     iso15434_uii);
}

/**
 * \brief C40 encoding (IEC 16022)
 */
static void ecc200_decode_next_c40(unsigned char * is_structured_append,
                                   unsigned char * is_gs1_encodation,
                                   int * application_identifier,
                                   unsigned char * application_identifier_length,
                                   int * application_data_start,
                                   int * application_data_end,
                                   unsigned char * application_data_variable,
                                   unsigned char data[],
                                   int datalength,
                                   int * position,
                                   int * state,
                                   int * shift,
                                   char result[],
                                   char gs1_result[],
                                   char gs1_url[],
                                   unsigned char debug,
                                   char iso15434_result[],
                                   unsigned char * is_iso1543,
                                   char format_code[],
                                   int * iso15434_data_start,
                                   char iso15434_uii[])
{
  int i, a, b, c, packed;

  if ((*position) + 1 == datalength) {
    /* ASCII used for a single character */
    ecc200_decode_next_ascii(is_structured_append,
                             is_gs1_encodation,
                             application_identifier,
                             application_identifier_length,
                             application_data_start,
                             application_data_end,
                             application_data_variable,
                             data,
                             datalength,
                             position,
                             state,
                             shift,
                             result, gs1_result, gs1_url, debug,
                             iso15434_result,
                             is_iso1543,
                             format_code,
                             iso15434_data_start,
                             iso15434_uii);
    return;
  }

  a = data[*position];
  b = data[(*position) + 1];
  *position = (*position) + 2;

  /* pack two data values into three bytes: 1600 * C1 + 40 * c2 + C3 + 1 */
  packed = a * 256 + b;
  int * c40Values = (int*)safemalloc(3*sizeof(int));
  c40Values[0] = ((packed - 1) / 1600);
  c40Values[1] = ((packed - 1) / 40) % 40;
  c40Values[2] = (packed - 1) % 40;

  /* extract the three characters */
  for (i = 0; i < 3; i++) {
    if (debug == 1) {
      printf("%d ", c40Values[i]);
    }
    if (*shift == 0) {
      if (c40Values[i] <= 2) {
        *shift = c40Values[i] + 1;
      }
      else if (c40Values[i] == 3) {
        /* space */
        decode_strcat_char(result, ' ');
      }
      else if (c40Values[i] <= 13) {
        /* 0-9 */
        decode_strcat_char(result, (char)(c40Values[i] - 13 + '9'));
      }
      else if (c40Values[i] <= 39) {
        if (*state == C40) {
          /* A-Z */
          decode_strcat_char(result, (char)(c40Values[i] - 39 + 'Z'));
        }
        else if (*state == TEXT) {
          /* a-z */
          decode_strcat_char(result, (char)(c40Values[i] - 39 + 'z'));
        }
      }
    }
    else if (*shift == 1) {
      /* Shift 1 set ASCII 0 - 31 */
      decode_strcat_char(result, (char)(c40Values[i]));
      *shift = 0;
    }
    else if (*shift == 2) {
      /* Shift 2 set */
      if (c40Values[i] <= 14) {
        /* ASCII 33 - 47 */
        decode_strcat_char(result, (char)(c40Values[i] + 33));
      }
      else if (c40Values[i] <= 21) {
        /* ASCII 58 - 64 */
        decode_strcat_char(result, (char)(c40Values[i] + 43));
      }
      else if (c40Values[i] <= 26) {
        /* ASCII 91 - 95 */
        decode_strcat_char(result, (char)(c40Values[i] + 69));
      }
      *shift = 0;
    }
    else if (*shift == 3) {
      /* Shift 3 set */
      if (*state == C40) {
        decode_strcat_char(result, (char)(c40Values[i] + 96));
      }
      else if (*state == TEXT) {
        if (c40Values[i] == 0) {
          decode_strcat_char(result, (char)(c40Values[i] + 96));
        }
        else if (c40Values[i] <= 26) {
          /* A-Z */
          decode_strcat_char(result, (char)(c40Values[i] - 26 + 'Z'));
        }
        else {
          /* { | } ~ DEL */
          decode_strcat_char(result, (char)(c40Values[i] - 31 + 127));
        }
      }
      *shift = 0;
    }
  }
  free(c40Values);

  /* Unlatch if codeword 254 follows 2 codewords in C40/Text encodation */
  if (*position < datalength) {
    c = data[*position];
    if (c == 254) {
      *state = ASCII;
      *position = (*position) + 1;
      *shift = 0;
    }
  }
  else {
    *state = ASCII;
  }
}

/**
 * \brief edifact encodation
 */
static void ecc200_decode_next_edifact(unsigned char * is_structured_append,
                                       unsigned char * is_gs1_encodation,
                                       int * application_identifier,
                                       unsigned char * application_identifier_length,
                                       int * application_data_start,
                                       int * application_data_end,
                                       unsigned char * application_data_variable,
                                       unsigned char data[],
                                       int datalength,
                                       int * position,
                                       int * state,
                                       int * shift,
                                       char result[],
                                       char gs1_result[],
                                       char gs1_url[],
                                       unsigned char debug,
                                       char iso15434_result[],
                                       unsigned char * is_iso1543,
                                       char format_code[],
                                       int * iso15434_data_start,
                                       char iso15434_uii[])
{
  int i;
  char * unpacked = (char*)safemalloc(4*sizeof(char));

  while (*position < datalength) {
    if (*position + 2 >= datalength)
      printf("An ECC200 decoding error occured in ecc200_decode_next_edifact\n");
    unpacked[0] =
      (char)((data[*position] & 0xfc) >> 2);
    unpacked[1] =
      (char)((data[*position] & 0x03) << 4 | (data[(*position) + 1] & 0xf0) >> 4);
    unpacked[2] =
      (char)((data[(*position) + 1] & 0x0f) << 2 | (data[(*position) + 2] & 0xc0) >> 6);
    unpacked[3] = (char)(data[*position + 2] & 0x3f);
    *position = (*position) + 3;

    for (i = 0; i < 4; i++) {
      if (debug == 1) printf("%d ", unpacked[i]);

      /* Test for unlatch condition */
      if (unpacked[i] == 0x1f) {
        free(unpacked);
        return;
      }
      else {
        decode_strcat_char(result, (char)(unpacked[i] ^ (((unpacked[i] & 0x20) ^ 0x20) << 1)));
      }
    }

    /* Unlatch is implied if fewer than 3 codewords remain */
    if (datalength - (*position) < 3) {
      for (i = 0; i < datalength; i++)
        ecc200_decode_next_ascii(is_structured_append,
                                 is_gs1_encodation,
                                 application_identifier,
                                 application_identifier_length,
                                 application_data_start,
                                 application_data_end,
                                 application_data_variable,
                                 data,
                                 datalength,
                                 position,
                                 state,
                                 shift,
                                 result, gs1_result, gs1_url, debug,
                                 iso15434_result,
                                 is_iso1543,
                                 format_code,
                                 iso15434_data_start,
                                 iso15434_uii);

      free(unpacked);
      return;
    }
  }
  free(unpacked);
}

/**
 * \brief x12 encodation
 */
static void ecc200_decode_next_x12(unsigned char * is_structured_append,
                                   unsigned char * is_gs1_encodation,
                                   int * application_identifier,
                                   unsigned char * application_identifier_length,
                                   int * application_data_start,
                                   int * application_data_end,
                                   unsigned char * application_data_variable,
                                   unsigned char data[],
                                   int datalength,
                                   int * position,
                                   int * state,
                                   int * shift,
                                   char result[],
                                   char gs1_result[],
                                   char gs1_url[],
                                   unsigned char debug,
                                   char iso15434_result[],
                                   unsigned char * is_iso1543,
                                   char format_code[],
                                   int * iso15434_data_start,
                                   char iso15434_uii[])
{
  int i, a, b, packed;

  if ((*position) + 1 == datalength) {
    /* ASCII used for a single character */
    ecc200_decode_next_ascii(is_structured_append,
                             is_gs1_encodation,
                             application_identifier,
                             application_identifier_length,
                             application_data_start,
                             application_data_end,
                             application_data_variable,
                             data,
                             datalength,
                             position,
                             state,
                             shift,
                             result, gs1_result, gs1_url, debug,
                             iso15434_result,
                             is_iso1543,
                             format_code,
                             iso15434_data_start,
                             iso15434_uii);
    return;
  }

  a = data[*position];
  b = data[(*position) + 1];
  *position = (*position) + 2;

  /* 3 bytes encoded by 1600 * C1 + 40 * c2 + C3 + 1 */
  packed = a * 256 + b;
  int * x12Values = (int*)safemalloc(3*sizeof(int));
  x12Values[0] = ((packed - 1) / 1600);
  x12Values[1] = ((packed - 1) / 40) % 40;
  x12Values[2] = (packed - 1) % 40;

  /* extract the three characters */
  for (i = 0; i < 3; i++) {
    if (debug == 1) printf("%d ", x12Values[i]);
    if (x12Values[i] == 0) {
      /* <CR> */
      decode_strcat_char(result, (char)13);
    }
    else if (x12Values[i] == 1) {
      /* segment separator * */
      decode_strcat_char(result, (char)42);
    }
    else if (x12Values[i] == 2) {
      /* segment separator > */
      decode_strcat_char(result, (char)62);
    }
    else if (x12Values[i] == 3) {
      /* Space */
      decode_strcat_char(result, ' ');
    }
    else if (x12Values[i] <= 13) {
      /* 0-9 */
      decode_strcat_char(result, (char)(x12Values[i] - 13 + '9'));
    }
    else if (x12Values[i] <= 39) {
      /* A-Z */
      decode_strcat_char(result, (char)(x12Values[i] - 39 + 'Z'));
    }
  }
  free(x12Values);

  /* Unlatch if codeword 254 follows 2 codewords in C40/Text encodation */
  if (*position < datalength) {
    int c = data[*position];
    if (c == 254) {
      /* switch back to ASCII */
      *state = ASCII;
      *position = (*position) + 1;
      *shift = 0;
    }
  }
  else {
    *state = ASCII;
  }
}

static void ecc200_decode(unsigned char data1[],
                          int data_length,
                          char result[],
                          char gs1_result[],
                          char iso15434_result[],
                          char iso15434_uii[],
                          char gs1_url[],
                          unsigned char debug)
{
  /* initial state is ASCII, which may change later */
  int state = ASCII, prev_state = -1;
  int datalength = data_length;
  unsigned char * data = data1;
  int position = 0;
  int shift = 0;
  unsigned char is_structured_append = 0;
  unsigned char is_gs1_encodation = 0;
  int application_identifier = 0;
  unsigned char application_identifier_length = 2;
  int application_data_start = 0;
  int application_data_end = 2;
  unsigned char application_data_variable = 0;
  unsigned char is_iso1543 = 0;
  char format_code[MAX_DECODE_LENGTH];
  int iso15434_data_start = -1;

  format_code[0] = 0;
  iso15434_uii[0] = 0;

  if (debug == 1) {
    printf("\nECC200 bytes: ");
  }
  while (position < datalength) {
    switch (state) {
    case ASCII:
      if ((debug == 1) && (prev_state != state)) printf("ASC ");
      ecc200_decode_next_ascii(&is_structured_append,
                               &is_gs1_encodation,
                               &application_identifier,
                               &application_identifier_length,
                               &application_data_start,
                               &application_data_end,
                               &application_data_variable,
                               data,
                               datalength,
                               &position,
                               &state,
                               &shift,
                               result, gs1_result, gs1_url, debug,
                               iso15434_result,
                               &is_iso1543,
                               &format_code[0],
                               &iso15434_data_start,
                               iso15434_uii);
      break;
    case C40:
      if ((debug == 1) && (prev_state != state)) printf("C40 ");
      ecc200_decode_next_c40(&is_structured_append,
                             &is_gs1_encodation,
                             &application_identifier,
                             &application_identifier_length,
                             &application_data_start,
                             &application_data_end,
                             &application_data_variable,
                             data,
                             datalength,
                             &position,
                             &state,
                             &shift,
                             result, gs1_result, gs1_url, debug,
                             iso15434_result,
                             &is_iso1543,
                             &format_code[0],
                             &iso15434_data_start,
                             iso15434_uii);
      break;
    case TEXT:
      if ((debug == 1) && (prev_state != state)) printf("TXT ");
      ecc200_decode_next_c40(&is_structured_append,
                             &is_gs1_encodation,
                             &application_identifier,
                             &application_identifier_length,
                             &application_data_start,
                             &application_data_end,
                             &application_data_variable,
                             data,
                             datalength,
                             &position,
                             &state,
                             &shift,
                             result, gs1_result, gs1_url, debug,
                             iso15434_result,
                             &is_iso1543,
                             &format_code[0],
                             &iso15434_data_start,
                             iso15434_uii);
      break;
    case BYTE256:
      if ((debug == 1) && (prev_state != state)) printf("BYT ");
      ecc200_decode_next_byte_256(&is_structured_append,
                                  is_gs1_encodation,
                                  data,
                                  datalength,
                                  &position,
                                  &state,
                                  &shift,
                                  result, gs1_result,
                                  iso15434_result,
                                  &is_iso1543,
                                  &format_code[0],
                                  &iso15434_data_start,
                                  iso15434_uii);
      break;
    case EDIFACT:
      if ((debug == 1) && (prev_state != state)) printf("EDI ");
      ecc200_decode_next_edifact(&is_structured_append,
                                 &is_gs1_encodation,
                                 &application_identifier,
                                 &application_identifier_length,
                                 &application_data_start,
                                 &application_data_end,
                                 &application_data_variable,
                                 data,
                                 datalength,
                                 &position,
                                 &state,
                                 &shift,
                                 result, gs1_result, gs1_url, debug,
                                 iso15434_result,
                                 &is_iso1543,
                                 &format_code[0],
                                 &iso15434_data_start,
                                 iso15434_uii);
      break;
    case X12:
      if ((debug == 1) && (prev_state != state)) printf("X12 ");
      ecc200_decode_next_x12(&is_structured_append,
                             &is_gs1_encodation,
                             &application_identifier,
                             &application_identifier_length,
                             &application_data_start,
                             &application_data_end,
                             &application_data_variable,
                             data,
                             datalength,
                             &position,
                             &state,
                             &shift,
                             result, gs1_result, gs1_url, debug,
                             iso15434_result,
                             &is_iso1543,
                             &format_code[0],
                             &iso15434_data_start,
                             iso15434_uii);
      break;
    default: {
      result[0] = 0;
      position = datalength;
    }
    }
    prev_state = state;
  }
  if (debug == 1) {
    printf("\n\n");
  }
}

static int reed_solomon_modnn(int x, int m_NN, int m_MM)
{
  while (x >= m_NN) {
    x -= m_NN;
    x = (x >> m_MM) + (x & m_NN);
  }
  return(x);
}

static void reed_solomon_gen_ltab(unsigned char tal[],
                                  unsigned char m_taltab[],
                                  unsigned char m_tal1tab[])
{
  int i, j, k;

  for (i = 0; i < 256; i++) {
    /* For each value of input */
    m_taltab[i] = 0;
    for (j = 0; j < 8; j++) {
      /* for each column of matrix */
      for (k = 0; k < 8; k++) {
        /* for each row of matrix */
        if ((i & (1 << k)) > 0) {
          m_taltab[i] =
            (unsigned char)((m_taltab[i] ^ tal[7 - k]) & (1 << j));
        }
      }
    }
    m_tal1tab[m_taltab[i]] = (unsigned char)i;
  }
}

static void reed_solomon_generate_gf(int m_MM,
                                     int m_NN,
                                     int m_A0,
                                     int m_alpha_to[],
                                     int m_index_of[],
                                     int m_Pp[])
{
  int i, mask;

  mask = 1;
  m_alpha_to[m_MM] = 0;
  for (i = 0; i < m_MM; i++) {
    m_alpha_to[i] = mask;
    m_index_of[m_alpha_to[i]] = i;

    /* If Pp[i] == 1 then, term @^i occurs in poly-repr of @^m_MM */
    if (m_Pp[i] != 0) {
      /* Bit-wise EXOR operation */
      m_alpha_to[m_MM] = m_alpha_to[m_MM] ^ mask;
    }
    /* single left-shift */
    mask <<= 1;
  }
  m_index_of[m_alpha_to[m_MM]] = m_MM;

  /* Have obtained poly-repr of @^m_MM. Poly-repr of @^(i+1)  given by
     poly-repr of @^i shifted left one-bit and accounting for any @^m_MM
     term that may occur when poly-repr of @^i is shifted. */

  mask >>= 1;
  for (i = m_MM + 1; i < m_NN; i++) {
    if (m_alpha_to[i - 1] >= mask) {
      m_alpha_to[i] = m_alpha_to[m_MM] ^ ((m_alpha_to[i - 1] ^ mask) << 1);
    }
    else {
      m_alpha_to[i] = m_alpha_to[i - 1] << 1;
    }
    m_index_of[m_alpha_to[i]] = i;
  }
  m_index_of[0] = m_A0;
  m_alpha_to[m_NN] = 0;
}

static void reed_solomon_gen_poly(int m_NN,
                                  int m_MM,
                                  int m_KK,
                                  int m_B0,
                                  int m_PRIM,
                                  int m_Gg[],
                                  int m_index_of[],
                                  int m_alpha_to[])
{
  int i, j, x, x2;

  m_Gg[0] = 1;
  for (i = 0; i < m_NN - m_KK; i++) {
    m_Gg[i + 1] = 1;

    /* Below multiply (m_Gg[0]+m_Gg[1]*x + ... +m_Gg[i]x^i) by
       (@**(B0+i)*PRIM + x) */

    for (j = i; j > 0; j--) {
      if ((int)m_Gg[j] != 0) {
        x = (m_index_of[m_Gg[j]]) + (m_B0 + i) * m_PRIM;
        while (x >= m_NN) {
          x -= m_NN;
          x = (x >> m_MM) + (x & m_NN);
        }

        m_Gg[j] = m_Gg[j - 1] ^ m_alpha_to[x];
      }
      else {
        m_Gg[j] = m_Gg[j - 1];
      }
    }

    /* m_Gg[0] can never be zero */
    x2 = m_index_of[m_Gg[0]] + (m_B0 + i) * m_PRIM;
    while (x2 >= m_NN) {
      x2 -= m_NN;
      x2 = (x2 >> m_MM) + (x2 & m_NN);
    }
    m_Gg[0] = m_alpha_to[x2];
  }

  /* convert m_Gg[] to index form for quicker encoding */
  for (i = 0; i <= m_NN - m_KK; i++) {
    m_Gg[i] = m_index_of[m_Gg[i]];
  }
}

static unsigned char reed_solomon_decode_init(int symbolBits,
                                              int numberParity,
                                              int B0,
                                              int Prim,
                                              int * m_MM,
                                              int * m_NN,
                                              int * m_KK,
                                              int * m_B0,
                                              int * m_PRIM,
                                              int * m_A0,
                                              unsigned char * m_CCSDS,
                                              int m_Pp[],
                                              int m_alpha_to[],
                                              int m_index_of[],
                                              int m_Gg[],
                                              int * m_Ldec,
                                              unsigned char tal[],
                                              unsigned char m_taltab[],
                                              unsigned char m_tal1tab[])
{
  *m_MM = symbolBits;
  *m_NN = (1 << (*m_MM)) - 1;
  *m_KK = (*m_NN) - numberParity;	/* Max. Number of data symbols per block */
  *m_B0 = B0;
  *m_PRIM = Prim;

  if (((*m_MM) == 8) &&
      ((*m_KK) == 223) &&
      ((*m_B0) == 112) &&
      ((*m_PRIM) == 11)) {
    *m_CCSDS = 1;
  }
  else {
    *m_CCSDS = 0;
  }
  memset(m_Pp, 0, ((*m_MM) + 1) * sizeof(int));

  m_Pp[0] = 1;
  m_Pp[*m_MM] = 1;
  if (*m_CCSDS == 1) {
    /* CCSDS field generator polynomial: 1+x+x^2+x^7+x^8 */
    m_Pp[1] = 1; m_Pp[2] = 1; m_Pp[7] = 1;
  }
  else {
    /* Primitive polynomials */
    switch (*m_MM) {
    case 2: {
      /* { 1, 1, 1 }; */
      m_Pp[1] = 1;
      break;
    }
    case 3: {
      /* { 1, 1, 0, 1 };  1 + x + x^3 */
      m_Pp[1] = 1;
      break;
    }
    case 4: {
      /* { 1, 1, 0, 0, 1 }; 1 + x + x^4 */
      m_Pp[1] = 1;
      break;
    }
    case 5: {
      /* { 1, 0, 1, 0, 0, 1 }; 1 + x^2 + x^5 */
      m_Pp[2] = 1;
      break;
    }
    case 6: {
      /* { 1, 1, 0, 0, 0, 0, 1 }; 1 + x + x^6 */
      m_Pp[1] = 1;
      break;
    }
    case 7: {
      /* { 1, 0, 0, 1, 0, 0, 0, 1 }; 1 + x^3 + x^7 */
      m_Pp[3] = 1;
      break;
    }
    case 8: {
      /* { 1, 0, 1, 1, 1, 0, 0, 0, 1 }; 1+x^2+x^3+x^4+x^8
         for (i=2; i<=4; i++) m_Pp[i] = 1;
         { 1, 0, 1, 1, 0, 1, 0, 0, 1 }; 1+x^2+x^3+x^5+x^8 */
      m_Pp[2] = 1; m_Pp[3] = 1; m_Pp[5] = 1;
      break;
    }
    case 9: {
      /* { 1, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; 1+x^4+x^9 */
      m_Pp[4] = 1;
      break;
    }
    case 10: {
      /* { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 }; 1+x^3+x^10 */
      m_Pp[3] = 1;
      break;
    }
    case 11: {
      /* { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 }; 1+x^2+x^11 */
      m_Pp[2] = 1;
      break;
    }
    case 12: {
      /* { 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1 }; 1+x+x^4+x^6+x^12 */
      m_Pp[1] = 1; m_Pp[4] = 1; m_Pp[6] = 1;
      break;
    }
    case 13: {
      /* { 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 }; 1+x+x^3+x^4+x^13 */
      m_Pp[1] = 1; m_Pp[3] = 1; m_Pp[4] = 1;
      break;
    }
    case 14: {
      /* { 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 }; 1+x+x^6+x^10+x^14 */
      m_Pp[1] = 1; m_Pp[6] = 1; m_Pp[10] = 1;
      break;
    }
    case 15: {
      /* { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }; 1+x+x^15 */
      m_Pp[1] = 1;
      break;
    }
    case 16: {
      /* { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 }; 1+x+x^3+x^12+x^16 */
      m_Pp[1] = 1; m_Pp[3] = 1; m_Pp[12] = 1;
      break;
    }
    default: {
      return 0;	/* "Either CCSDS must be defined, or m_MM must be set in range 2-16" */
    }
    }
  }
  /* index->polynomial form conversion table */
  memset(m_alpha_to, 0, ((*m_NN) + 1) * sizeof(int));

  /* Polynomial->index form conversion table */
  memset(m_index_of, 0, ((*m_NN) + 1) * sizeof(int));

  /* No legal value  index form represents zero, so
     we need a special value for this purpose */
  *m_A0 = (*m_NN);

  /* Generator polynomial g(x)  index form */
  memset(m_Gg, 0, ((*m_NN) - (*m_KK) + 1) * sizeof(int));

  reed_solomon_generate_gf(*m_MM, *m_NN, *m_A0, m_alpha_to, m_index_of, m_Pp);

  reed_solomon_gen_poly(*m_NN, *m_MM, *m_KK, *m_B0, *m_PRIM, m_Gg, m_index_of, m_alpha_to);

  if (*m_CCSDS == 1) {
    /* zero initialise */
    memset(m_tal1tab, 0, ((*m_NN) + 1) * sizeof(unsigned char));
    memset(m_taltab, 0, ((*m_NN) + 1) * sizeof(unsigned char));

    reed_solomon_gen_ltab(tal, m_taltab, m_tal1tab);
  }
  if (*m_PRIM != 1) {
    for ((*m_Ldec) = 1; ((*m_Ldec) % (*m_PRIM)) != 0; (*m_Ldec) = (*m_Ldec) + (*m_NN)) ;
    *m_Ldec /= (*m_PRIM);
  }
  else {
    *m_Ldec = 1;
  }
  return 1;
}

static int reed_solomon_decode(int symbolBits,
                               int numberParity,
                               int user_data[],
                               int user_data_length,
                               int paritydata[],
                               int paritydata_length,
                               int eras_pos[],
                               int eras_pos_length,
                               struct grid_2d * grid)
{
  int x, lambda_length, min_value, ci;
  int m_MM = 0;
  int m_NN = 0;
  int m_KK = 0;
  int m_B0 = 0;
  int m_PRIM = 0;
  int m_A0 = 0;
  unsigned char m_CCSDS = 0;
  int m_Ldec = 0;

  int * m_Pp = grid->m_Pp;
  /* index->polynomial form conversion table */
  int * m_alpha_to = grid->m_alpha_to;
  /* Polynomial->index form conversion table */
  int * m_index_of = grid->m_index_of;
  /* Generator polynomial g(x) in index form */
  int * m_Gg = grid->m_Gg;
  unsigned char * m_taltab = grid->m_taltab;
  unsigned char * m_tal1tab = grid->m_tal1tab;

  unsigned char tal[] = {
    0x8d, 0xef, 0xec, 0x86, 0xfa, 0x99, 0xaf, 0x7b
  };

  reed_solomon_decode_init(symbolBits, numberParity, 1, 1,
                           &m_MM, &m_NN, &m_KK,
                           &m_B0, &m_PRIM, &m_A0,
                           &m_CCSDS, m_Pp, m_alpha_to,
                           m_index_of, m_Gg, &m_Ldec,
                           tal, m_taltab, m_tal1tab);

  int deg_lambda, el, deg_omega, posn, tmp2, xi;
  int i, j, r, k;
  int u, q, tmp, num1, num2, den, discr_r;

  int* data = grid->data;
  memset(data, 0, m_NN * sizeof(int));

  int* lambda = grid->lambda;
  memset(lambda, 0, (m_NN - m_KK + 1) * sizeof(int));

  int* s = grid->s;
  /* Err+Eras Locator poly and syndrome poly */
  memset(s, 0, (m_NN - m_KK + 1) * sizeof(int));

  int* b = grid->b;
  memset(b, 0, (m_NN - m_KK + 1) * sizeof(int));

  int* t = grid->t;
  memset(t, 0, (m_NN - m_KK + 1) * sizeof(int));

  int* omega = grid->omega;
  memset(omega, 0, (m_NN - m_KK + 1) * sizeof(int));

  int* root = grid->root;
  memset(root, 0, (m_NN - m_KK) * sizeof(int));

  int* reg = grid->reg;
  memset(reg, 0, (m_NN - m_KK + 1) * sizeof(int));

  int* loc = grid->loc;
  memset(loc, 0, (m_NN - m_KK) * sizeof(int));

  int syn_error, count;
  if ((user_data_length > m_KK) || (paritydata_length != m_NN - m_KK)) {
    return -1;
  }
  memcpy(data, user_data, user_data_length * sizeof(int));
  for (i = m_KK, j = 0; i < m_NN; i++, j++) {
    data[i] = (int)paritydata[j];
  }
  int no_eras = 0;
  if (eras_pos != NULL) {
    no_eras = eras_pos_length;
    if (no_eras >= m_NN - m_KK) {
      /* printf("Too many erasures for error correction to work\n"); */
      return -1;
    }
  }
  if (m_CCSDS == 1) {
    /* Convert to conventional basis */
    for (i = 0; i < m_NN; i++) {
      data[i] = (int)(m_tal1tab[data[i]]);
    }
  }

  /* Check for illegal input values */
  for (i = 0; i < m_NN; i++) {
    if (data[i] > m_NN) {
      return -1;
    }
  }

  /* ****  "form the syndromes; i.e., evaluate data(x) at roots of g(x) namely @**(B0+i)*PRIM, i = 0, ... ,(m_NN-m_KK-1)" */

  for (i = 1; i <= m_NN - m_KK; i++) {
    s[i] = data[0];
  }

  for (j = 1; j < m_NN; j++) {
    if ((int)data[j] == 0) continue;
    tmp = (int)m_index_of[(int)data[j]];

    for (i = 1; i <= m_NN - m_KK; i++) {
      x = tmp + (m_B0 + i - 1) * m_PRIM * j;
      while (x >= m_NN) {
        x -= m_NN;
        x = (x >> m_MM) + (x & m_NN);
      }

      s[i] = s[i] ^ (int)m_alpha_to[x];
    }
  }

  /* Convert syndromes to index form, checking for nonzero condition */
  syn_error = 0;
  for (i = 1; i <= m_NN - m_KK; i++) {
    syn_error |= s[i];
    s[i] = m_index_of[s[i]];
  }
  if (syn_error == 0) {
    /* if syndrome is zero, data[] is a codeword and there are no
       errors to correct. So return data[] unmodified */
    return 0;
  }

  lambda_length = m_NN - m_KK + 1;
  memset(lambda, 0, lambda_length * sizeof(int));

  lambda[0] = 1;
  if (no_eras > 0) {
    /* Init lambda to be the erasure locator polynomial */
    x = m_PRIM * eras_pos[0];
    while (x >= m_NN) {
      x -= m_NN;
      x = (x >> m_MM) + (x & m_NN);
    }

    lambda[1] = m_alpha_to[x];
    for (i = 1; i < no_eras; i++) {
      posn = eras_pos[i];

      u = m_PRIM * posn;
      while (u >= m_NN) {
        u -= m_NN;
        u = (u >> m_MM) + (u & m_NN);
      }

      for (j = i + 1; j > 0; j--) {
        tmp = m_index_of[lambda[j - 1]];
        if (tmp != m_A0) {
					tmp2 = u + tmp;
					while (tmp2 >= m_NN) {
						tmp2 -= m_NN;
						tmp2 = (tmp2 >> m_MM) + (tmp2 & m_NN);
					}

					lambda[j] = lambda[j] ^ m_alpha_to[tmp2];
        }
      }
    }

  }
  for (i = 0; i < m_NN - m_KK + 1; i++) {
    b[i] = m_index_of[lambda[i]];
  }

  /* Begin Berlekamp-Massey algorithm to determine error+erasure locator polynomial */

  r = no_eras;
  el = no_eras;
  while (++r <= m_NN - m_KK) {
    /* r  the step number
       Compute discrepancy at the r-th step  poly-form */
    discr_r = 0;
    for (i = 0; i < r; i++) {
      if ((lambda[i] != 0) && (s[r - i] != m_A0)) {
        x = m_index_of[lambda[i]] + s[r - i];
        while (x >= m_NN) {
          x -= m_NN;
          x = (x >> m_MM) + (x & m_NN);
        }

        discr_r ^= m_alpha_to[x];
      }
    }
    discr_r = m_index_of[discr_r];	/* Index form */
    if (discr_r == m_A0) {
      /* 2 lines below: B(x) <-- x*B(x) */
      for (ci = m_NN - m_KK - 1; ci >= 0; ci--) b[ci + 1] = b[ci]; /* COPY(&b[1],b,m_NN-m_KK); */
      b[0] = m_A0;
    }
    else {
      /* 7 lines below: T(x) <-- lambda(x) - discr_r*x*b(x) */
      t[0] = lambda[0];
      for (i = 0; i < m_NN - m_KK; i++) {
        if (b[i] != m_A0) {
          x = discr_r + b[i];
          while (x >= m_NN) {
            x -= m_NN;
            x = (x >> m_MM) + (x & m_NN);
          }

          t[i + 1] = lambda[i + 1] ^ m_alpha_to[x];
        }
        else
          t[i + 1] = lambda[i + 1];
      }
      if (2 * el <= r + no_eras - 1) {
        el = r + no_eras - el;

        /* 2 lines below: B(x) <-- inv(discr_r) * lambda(x) */

        for (i = 0; i <= m_NN - m_KK; i++) {
          x = m_index_of[lambda[i]] - discr_r + m_NN;
          while (x >= m_NN) {
            x -= m_NN;
            x = (x >> m_MM) + (x & m_NN);
          }

          b[i] = (lambda[i] == 0) ? m_A0 : x;
        }
      }
      else {
        /* 2 lines below: B(x) <-- x*B(x) */
        for (ci = m_NN - m_KK - 1; ci >= 0; ci--) b[ci + 1] = b[ci]; /* COPYDOWN(&b[1],b,m_NN-m_KK); */
        b[0] = m_A0;
      }
      /* lambda = t; COPY(lambda,t,m_NN-m_KK+1); */
      for (xi = 0; xi < lambda_length; xi++) {
        lambda[xi] = t[xi];
      }
    }
  }

  /* Convert lambda to index form and compute deg(lambda(x)) */
  deg_lambda = 0;
  for (i = 0; i < m_NN - m_KK + 1; i++) {
    lambda[i] = m_index_of[lambda[i]];
    if (lambda[i] != m_A0) {
      deg_lambda = i;
    }
  }

  /* Find roots of the error+erasure locator polynomial by Chien Search */
  for (i = 1; i <= m_NN - m_KK; i++) {
    reg[i] = lambda[i];
  }

  /* Number of roots of lambda(x) */
  count = 0;
  for (i = 1, k = m_NN - m_Ldec; i <= m_NN; i++, k = reed_solomon_modnn(m_NN + k - m_Ldec, m_NN, m_MM)) {
    q = 1;
    for (j = deg_lambda; j > 0; j--) {
      if (reg[j] != m_A0) {
        x = reg[j] + j;
        while (x >= m_NN) {
          x -= m_NN;
          x = (x >> m_MM) + (x & m_NN);
        }

        reg[j] = x;
        q ^= m_alpha_to[reg[j]];
      }
    }
    if (q != 0)
      continue;

    /* store root (index-form) and error location number */
    root[count] = i;
    loc[count] = k;

    /* If we've already found max possible roots,
       abort the search to save time */
    if (++count == deg_lambda)
      break;
  }
  if (deg_lambda != count) {
    /* deg(lambda) unequal to number of roots => uncorrectable
       error detected */
    return -1;
  }

  /* Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo
     x**(m_NN-m_KK)).  index form. Also find deg(omega). */

  deg_omega = 0;
  for (i = 0; i < m_NN - m_KK; i++) {
    tmp = 0;
    j = (deg_lambda < i) ? deg_lambda : i;
    for (; j >= 0; j--) {
      if ((s[i + 1 - j] != m_A0) && (lambda[j] != m_A0)) {
        x = s[i + 1 - j] + lambda[j];
        while (x >= m_NN) {
          x -= m_NN;
          x = (x >> m_MM) + (x & m_NN);
        }

        tmp ^= m_alpha_to[x];
      }
    }
    if (tmp != 0) {
      deg_omega = i;
    }
    omega[i] = m_index_of[tmp];
  }
  omega[m_NN - m_KK] = m_A0;


  /* Compute error values  poly-form. num1 = omega(inv(X(l))), num2 =
     inv(X(l))**(B0-1) and den = lambda_pr(inv(X(l))) all  poly-form */

  for (j = count - 1; j >= 0; j--) {
    num1 = 0;
    for (i = deg_omega; i >= 0; i--) {
      if (omega[i] != m_A0) {
        x = omega[i] + i * root[j];
        while (x >= m_NN) {
          x -= m_NN;
          x = (x >> m_MM) + (x & m_NN);
        }

        num1 ^= m_alpha_to[x];
      }
    }

    x = root[j] * (m_B0 - 1) + m_NN;
    while (x >= m_NN) {
      x -= m_NN;
      x = (x >> m_MM) + (x & m_NN);
    }

    num2 = m_alpha_to[x];
    den = 0;

    /* lambda[i+1] for i even using the formal derivative lambda_pr of lambda[i] */

    min_value = m_NN - m_KK - 1;
    if (deg_lambda < min_value) min_value = deg_lambda;

    for (i = min_value & ~1; i >= 0; i -= 2) {
      if ((int)lambda[i + 1] != m_A0) {
        x = lambda[i + 1] + i * root[j];
        while (x >= m_NN) {
          x -= m_NN;
          x = (x >> m_MM) + (x & m_NN);
        }

        den ^= m_alpha_to[x];
      }
    }
    if (den == 0) {
      /* Convert to dual- basis */
      return -1;
    }

    /* Apply error to data */
    if (num1 != 0) {
      x = m_index_of[num1] + m_index_of[num2] + m_NN - m_index_of[den];
      while (x >= m_NN) {
        x -= m_NN;
        x = (x >> m_MM) + (x & m_NN);
      }

      data[loc[j]] = data[loc[j]] ^ m_alpha_to[x];
    }
  }
  if (m_CCSDS == 1) {
    /* Convert to dual- basis */
    for (i = 0; i < m_NN; i++) {
      data[i] = (int)(m_taltab[data[i]]);
    }
  }
  if (count > 0) {
    if (count - no_eras > floor((m_NN - m_KK - no_eras) / 2.0)) {
      count = -1;
    }
    else {
      for (i = user_data_length; i < m_KK; i++) {
        if (data[i] != 0) {
          count = -1;
          break;
        }
      }
      if (count > 0) {
        memcpy(user_data, data, user_data_length * sizeof(int));
        for (i = m_KK, j = 0; i < m_NN; i++, j++) paritydata[j] = data[i];
      }
    }
  }

  return count;
}

/**
 * \brief reed solomon error correction
 */
static int reed_solomon_correct(int n_err_data,
                                unsigned char rs_codewords[],
                                int rs_codewords_length,
                                int * error_codeword_count,
                                int erasure_index[],
                                int erasure_index_length,
                                int * no_of_erasures,
                                struct grid_2d * grid)
{
  unsigned char found;
  int damaged_codeword_index, ndata, n2, idx, i = 0, j;
  int nucorrections;
  int n = 255 - (*error_codeword_count);
  if (n < 0) return -1;
  int data_length = n;
  int * data = (int*)safemalloc(n*sizeof(int));
  memset(data, 0, n * sizeof(int));

  int parity_length = (*error_codeword_count);
  int * parity = (int*)safemalloc((*error_codeword_count)*sizeof(int));
  memset(parity, 0, (*error_codeword_count) * sizeof(int));

  /* create erasures array */
  int * erasures = NULL;
  if (erasure_index != NULL) {
    erasures = (int*)safemalloc(erasure_index_length*sizeof(int));
    memset(erasures, 0, erasure_index_length * sizeof(int));
  }

  /* NOTE: symbol order is reversed */
  ndata = rs_codewords_length - (*error_codeword_count);
  for (i = ndata - 1; i >= 0; i--) {
    idx = ndata - i - 1;
    if (idx < data_length) {
      data[idx] = (int)(rs_codewords[i]);
    }
  }

  n2 = 0;
  for (i = rs_codewords_length - 1; i >= ndata; i--) {
    if (i > -1) {
      parity[n2++] = (int)(rs_codewords[i]);
    }
  }

  if (erasure_index != NULL) {
    n2 = 0;
    for (i = 0; i < erasure_index_length; i++) {
      damaged_codeword_index = erasure_index[i];
      damaged_codeword_index = rs_codewords_length - 1 - damaged_codeword_index;

      found = 0;
      j = 0;
      while ((j < n2) && (found == 0)) {
        if (erasures[j] == damaged_codeword_index) found = 1;
        j++;
      }

      if (found == 0) {
        erasures[n2++] = damaged_codeword_index;
      }
    }

    if (n2 < erasure_index_length) {
      if (n2 > 0) {
        int * new_erasures = (int*)safemalloc(n2*sizeof(int));
        memcpy(new_erasures, erasures, n2 * sizeof(int));
        free(erasures);
        erasures = new_erasures;
        erasure_index_length = n2;
      }
      else {
        if (erasures != NULL) free(erasures);
        erasures = NULL;
      }
    }
  }

  if (parity_length > 0) {
    nucorrections =
      reed_solomon_decode(8, n_err_data, data, data_length,
                          parity, parity_length,
                          erasures, erasure_index_length,
                          grid);
  }
  else {
    nucorrections = -1;
  }

  if (erasures != NULL) {
    *no_of_erasures = erasure_index_length;
  }
  else {
    *no_of_erasures = 0;
  }

  if (nucorrections < 0) {
    if (erasures != NULL) free(erasures);
    free(data);
    free(parity);
    return -1;
  }
  for (i = ndata; i < n; i++) {
    if (data[i] != 0) {
      if (erasures != NULL) free(erasures);
      free(data);
      free(parity);
      return -1;
    }
  }
  for (i = ndata - 1; i >= 0; i--) {
    idx = ndata - 1 - i;
    if (idx < data_length) rs_codewords[i] = (unsigned char)data[idx];
  }

  if (erasures != NULL) free(erasures);
  free(data);
  free(parity);
  return nucorrections;
}

static unsigned char reed_solomon(unsigned char codewords[],
                                  int codewords_length,
                                  int n_err_data,
                                  int erasures[],
                                  int erasures_length,
                                  int * nooferrors,
                                  int * nooferasures,
                                  unsigned char corrected_codewords[],
                                  int * corrected_codewords_length,
                                  struct grid_2d * grid)
{
  int i, no_of_errors = -1;
  int no_of_erasures = 0;
  int error_codeword_count;
  unsigned char * rs_codewords;

  rs_codewords = (unsigned char*)safemalloc(codewords_length*sizeof(unsigned char));

  if (erasures_length == 0) no_of_erasures = 0;

  error_codeword_count = n_err_data;
  memcpy(rs_codewords, codewords, codewords_length*sizeof(unsigned char));

  no_of_errors =
    reed_solomon_correct(n_err_data,
                         rs_codewords,
                         codewords_length,
                         &error_codeword_count,
                         erasures,
                         erasures_length,
                         &no_of_erasures,
                         grid);

  *nooferrors = no_of_errors;
  *nooferasures = no_of_erasures;

  *corrected_codewords_length = codewords_length - error_codeword_count;
  for (i = 0; i < (*corrected_codewords_length); i++)
    corrected_codewords[i] = rs_codewords[i];

  free(rs_codewords);
  if (no_of_errors != -1) {
    return 0;
  }
  return -1;
}

/**
 * \brief returns the number of error correction words within the given data matrix pattern
 * \param cells_across number of cells across the grid
 * \param cells_down number of cells down the grid
 * \return number of error correction words
 */
static int get_no_of_error_correction_words(int cells_across, int cells_down)
{
  if (cells_across == cells_down) {
    /* square codes */
    switch (cells_across) {
    case 10: return 5;
    case 12: return 7;
    case 14: return 10;
    case 16: return 12;
    case 18: return 14;
    case 20: return 18;
    case 22: return 20;
    case 24: return 24;
    case 26: return 28;
    case 32: return 36;
    case 36: return 42;
    case 40: return 48;
    case 44: return 56;
    case 48: return 68;
    case 52: return 84;
    case 64: return 112;
    case 72: return 144;
    case 80: return 192;
    case 88: return 224;
    case 96: return 272;
    case 104: return 336;
    case 120: return 408;
    case 132: return 496;
    case 144: return 620;
    }
    return 0;
  }

  /* rectangular codes */
  switch (cells_down) {
  case 8: {
    if (cells_across == 18) return 7;
    if (cells_across == 32) return 11;
    break;
  }
  case 12: {
    if (cells_across == 26) return 14;
    if (cells_across == 36) return 18;
    break;
  }
  case 16: {
    if (cells_across == 36) return 24;
    if (cells_across == 48) return 28;
    break;
  }
  }
  return 0;
}

/**
 * \brief place key and value within translation_table
 * \param translation_table Translation table array
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \param row current row
 * \param col current column
 * \param chr key to be inserted
 * \param bit value to be inserted
 */
static void translation_table_module(struct key_value_pair_int* translation_table[],
                                     int no_of_rows, int no_of_cols,
                                     int row, int col, int chr, int bit)
{
  if (translation_table == NULL) return;
  if (translation_table[0] == NULL) return;
  if (row < 0) {
    row += no_of_rows;
    col += 4 - ((no_of_rows + 4) % 8);
  }
  if (col < 0) {
    col += no_of_cols;
    row += 4 - ((no_of_cols + 4) % 8);
  }
  if ((row > -1) && (row < no_of_rows) &&
      (col > -1) && (col < no_of_cols)) {
    translation_table[row][col].Key = chr;
    translation_table[row][col].Value = bit;
  }
}

/**
 * \brief "utah" places the 8 bits in utah shape
 * \param translation_table table data
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \param row
 * \param col
 * \param chr
 */
static void utah(struct key_value_pair_int** translation_table,
                 int no_of_rows, int no_of_cols,
                 int row, int col, int chr)
{
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row - 2, col - 2, chr, 1);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row - 2, col - 1, chr, 2);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row - 1, col - 2, chr, 3);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row - 1, col - 1, chr, 4);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row - 1, col, chr, 5);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row, col - 2, chr, 6);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row, col - 1, chr, 7);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           row, col, chr, 8);
}

/**
 * \brief corner1 shape within translation table
 * \param translation_table
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \param chr
 */
static void corner1(struct key_value_pair_int** translation_table,
                    int no_of_rows, int no_of_cols,
                    int chr)
{
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, 0, chr, 1);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, 1, chr, 2);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, 2, chr, 3);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 2, chr, 4);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 1, chr, 5);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           1, no_of_cols - 1, chr, 6);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           2, no_of_cols - 1, chr, 7);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           3, no_of_cols - 1, chr, 8);
}

/**
 * \brief corner2 shape within translation table
 * \param translation_table
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \param chr
 */
static void corner2(struct key_value_pair_int** translation_table,
                    int no_of_rows, int no_of_cols,
                    int chr)
{
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 3, 0, chr, 1);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 2, 0, chr, 2);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, 0, chr, 3);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 4, chr, 4);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 3, chr, 5);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 2, chr, 6);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 1, chr, 7);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           1, no_of_cols - 1, chr, 8);
}

/**
 * \brief corner3 shape within translation table
 * \param translation_table
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \param chr
 */
static void corner3(struct key_value_pair_int** translation_table,
                    int no_of_rows, int no_of_cols,
                    int chr)
{
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 3, 0, chr, 1);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 2, 0, chr, 2);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, 0, chr, 3);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 2, chr, 4);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 1, chr, 5);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           1, no_of_cols - 1, chr, 6);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           2, no_of_cols - 1, chr, 7);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           3, no_of_cols - 1, chr, 8);
}

/**
 * \brief corner4 shape within translation table
 * \param translation_table
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \param chr
 */
static void corner4(struct key_value_pair_int** translation_table,
                    int no_of_rows, int no_of_cols,
                    int chr)
{
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, 0, chr, 1);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           no_of_rows - 1, no_of_cols - 1, chr, 2);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 3, chr, 3);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 2, chr, 4);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           0, no_of_cols - 1, chr, 5);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           1, no_of_cols - 3, chr, 6);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           1, no_of_cols - 2, chr, 7);
  translation_table_module(translation_table, no_of_rows, no_of_cols,
                           1, no_of_cols - 1, chr, 8);
}

static void show_translation_table(struct key_value_pair_int** translation_table,
                                   int dim_x, int dim_y)
{
  int x, y, key, value;

  /* don't show large codes */
  if (dim_x > 22) return;

  printf("Translation table\n\n");
  for (y = 0; y < dim_y; y++) {
    for (x = 0; x < dim_x; x++) {
      key = translation_table[x][y].Key;
      value = translation_table[x][y].Value;
      if ((key>64) || (key<0)) key = 0;
      if ((value>64) || (value<0)) value = 0;
      printf("%02d %02d ", key, value);
    }
    printf("\n");
  }
  printf("\n");
}

/**
 * \brief creates a translation table using the given dimensions
 * \param no_of_rows number of rows in the table
 * \param no_of_cols number of columns in the table
 * \return translation table array
 */
static struct key_value_pair_int** generate_translation_table(int no_of_rows, int no_of_cols)
{
  int i, chr, row, col;
  struct key_value_pair_int** translation_table =
    (struct key_value_pair_int**)safemalloc(no_of_rows*sizeof(struct key_value_pair_int*));
  for (i = 0; i < no_of_rows; i++) {
    translation_table[i] =
      (struct key_value_pair_int*)safemalloc(no_of_cols*sizeof(struct key_value_pair_int));
    /* clear the table to zero */
    memset(translation_table[i], 0, no_of_cols*sizeof(struct key_value_pair_int));
  }

  if (translation_table != NULL) {
    chr = 1;
    row = 4;
    col = 0;

    do {
      /* handle special cases */
      if ((row == no_of_rows) && (col == 0)) {
        corner1(translation_table, no_of_rows, no_of_cols, chr++);
      }
      if ((row == no_of_rows - 2) && (col == 0) && ((no_of_cols % 4) != 0)) {
        corner2(translation_table, no_of_rows, no_of_cols, chr++);
      }
      if ((row == no_of_rows - 2) && (col == 0) && (no_of_cols % 8 == 4)) {
        corner3(translation_table, no_of_rows, no_of_cols, chr++);
      }
      if ((row == no_of_rows + 4) && (col == 2) && ((no_of_cols % 8) == 0)) {
        corner4(translation_table, no_of_rows, no_of_cols, chr++);
      }

      /* populate diagonally up */
      do {
        if ((row < no_of_rows) && (col >= 0) &&
            (translation_table[row][col].Key == 0)) {
          utah(translation_table, no_of_rows, no_of_cols, row, col, chr++);
        }

        row -= 2;
        col += 2;
      } while ((row >= 0) && (col < no_of_cols));

      row += 1;
      col += 3;

      /* populate diagonally down */
      do {
        if ((row >= 0) && (col < no_of_cols) &&
            (translation_table[row][col].Key == 0)) {
          utah(translation_table, no_of_rows, no_of_cols, row, col, chr++);
        }

        row += 2;
        col -= 2;
      }
      while ((row < no_of_rows) && (col >= 0));

      row += 3;
      col += 1;

    } while ((row < no_of_rows) || (col < no_of_cols));
    return translation_table;
  }
  return NULL;
}

/**
 * \brief finds the given codeword+bit within the translation table
 */
static void locate_bit(struct key_value_pair_int* translation_table[],
                       int table_dim_x,
                       int table_dim_y,
                       int codeword,
                       int bit,
                       int * Key,
                       int * Value)
{
  int x, y;

  x = 0;
  while (x < table_dim_x) {
    y = 0;
    while (y < table_dim_y) {
      if ((translation_table[x][y].Key == codeword + 1) &&
          (translation_table[x][y].Value == bit + 1)) {
        *Key = x;
        *Value = y;
        return;
      }
      y++;
    }
    x++;
  }
}

/**
 * \brief displays the codeword pattern
 * \param title title to be shown
 * \param codeword_pattern the coreword array to be shown
 * \param dimension_x x dimension of the grid
 * \param dimension_y y dimension of the grid
 */
static void show_codeword_pattern(char * title, int * codeword_pattern[],
                                  int dimension_x, int dimension_y)
{
  int x, y, i=0;

  /* don't show large codes */
  if (dimension_x > 18) return;

  printf("%s:\n\n", title);
  for (y = 0; y < dimension_y; y++) {
    if ((y == dimension_y-1) || (y==1)) printf("XX\n");
    for (x = 0; x < dimension_x; x++, i++) {
      if ((x > 0) && (y > 0) &&
          (x < dimension_x-1) && (y < dimension_y-1)) {
        printf("%02x ", codeword_pattern[x][y]);
      }
      else {
        if ((x == 0) || (y == dimension_y-1)) {
          if (x == 0) {
            if (y == dimension_y-1) {
              printf("XXXX");
            }
            else {
              printf("XX  ");
            }
          }
          else {
            printf("XXX");
          }
        }
        else {
          if (x == dimension_x-1) {
            if (y % 2 == 1) {
              printf(" XX ");
            }
            else {
              printf("   ");
            }
          }
          if (y == 0) {
            if (1 - (x % 2) == 1) {
              printf("XX ");
            }
            else {
              printf("   ");
            }
          }
        }
      }
    }
    printf("\n");
  }
}

/**
 * \brief translates the given grid occupancy pattern into a set of codewords
 * \param grid grid object
 * \param debug set to 1 if in debug mode
 * \return number of codewords
 */
static int translate(struct grid_2d * grid, unsigned char debug)
{
  int i, bit, max_codewords, codewords_length = 0;
  int Key, Value, x, y;
  int dim_x = grid->dimension_y - 2;
  int dim_y = grid->dimension_x - 2;

  struct key_value_pair_int ** translation_table = generate_translation_table(dim_x, dim_y);
  if (translation_table != NULL) {
    if (debug == 1) {
      show_translation_table(translation_table, dim_x, dim_y);
    }
    max_codewords = (int)floor(dim_x * dim_y / 8.0);
    codewords_length = max_codewords;

    for (i = 0; i < max_codewords; i++) {
      grid->codeword[i] = 0;

      for (bit = 0; bit < 8; bit++) {
        grid->data_bytes[bit] = 0;
        Key = 0;
        Value = 0;
        locate_bit(translation_table, dim_x, dim_y, i, bit, &Key, &Value);
        x = Value + 1;
        y = Key + 1;
        if ((x < grid->dimension_x) && (y < grid->dimension_y)) {
          if (grid->occupancy[x][y] != 0) grid->data_bytes[bit] = 1;
          if (grid->codeword_pattern != NULL) {
            grid->codeword_pattern[x][y] = i;
          }
          continue;
        }
        codewords_length = 0;
        i = max_codewords;
        bit = 8;
      }

      if (codewords_length > 0) {
        grid->codeword[i] =
          (unsigned char)(grid->data_bytes[0] << 7 |
                          grid->data_bytes[1] << 6 |
                          grid->data_bytes[2] << 5 |
                          grid->data_bytes[3] << 4 |
                          grid->data_bytes[4] << 3 |
                          grid->data_bytes[5] << 2 |
                          grid->data_bytes[6] << 1 |
                          grid->data_bytes[7]);
      }
    }

    if (debug == 1) {
      show_codeword_pattern("Codeword pattern", grid->codeword_pattern,
                            grid->dimension_x, grid->dimension_y);
    }

    /* free translation table */
    for (x = 0; x < dim_x; x++) free(translation_table[x]);
    free(translation_table);
  }
  return codewords_length;
}

/**
 * \brief decode the detected datamatrix within the given grid
 * \param grid grid object
 * \param debug set to 1 to enable debug mode
 * \param gs1_url url prefix for GS1 digital link
 * \param result returned decoded text
 * \param human_readable set to 1 if the decode should be human readable
 */
void datamatrix_decode(struct grid_2d * grid, unsigned char debug,
                       char gs1_url[], char result[],
                       unsigned char human_readable)
{
  int i, codewords_length, error_correcting_words;
  int corrected_codewords_length, erasures_length;
  int grid_no_of_errors=0, grid_no_of_erasures=0;
  int * erasures = NULL;
  char * gs1_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  char * iso15434_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  char * hibc_result = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  char * iso15434_uii = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
  int condensed=0;

  result[0] = 0;
  gs1_result[0] = 0;
  iso15434_result[0] = 0;
  iso15434_uii[0] = 0;
  hibc_result[0] = 0;

  int original_dimension_x = grid->dimension_x;
  int original_dimension_y = grid->dimension_y;
  condensed = condense_data_blocks(grid, debug);

  /* clear the codeword pattern */
  for (i = 0; i < grid->dimension_x; i++)
    memset(grid->codeword_pattern[i], 0, grid->dimension_y * sizeof(int));

  codewords_length = translate(grid, debug);
  if (codewords_length == 0) {
    free(gs1_result);
    free(iso15434_result);
    free(hibc_result);
    free(iso15434_uii);
    return;
  }

  /* how many of the codewords are used for parity error correction */
  error_correcting_words =
    get_no_of_error_correction_words(grid->dimension_x, grid->dimension_y);

  /* use the reed solomon algorithm to decode and perform error correction */
  corrected_codewords_length = 0;
  erasures_length = 0;
  for (i = 0; i < grid->dimension_x*grid->dimension_y; i++)
    if (grid->damage[i] > 0) erasures_length++;
  if (erasures_length > 0) {
    memset(grid->erasures, 0, erasures_length * sizeof(int));
    locate_erasures(grid->dimension_x, grid->dimension_y,
                    grid->codeword_pattern,
                    grid->damage,
                    grid->erasures,
                    &erasures_length);
  }
  grid->no_of_errors = 0;
  grid->no_of_erasures = 0;
  grid->unused_error_correction = 0;
  grid->gs1_datamatrix = 0;
  grid->iso15434_datamatrix = 0;
  grid->hibc_datamatrix = 0;
  grid_no_of_errors = grid->no_of_errors;
  grid_no_of_erasures = grid->no_of_erasures;
  if (reed_solomon(grid->codeword, codewords_length,
                   error_correcting_words,
                   erasures, erasures_length,
                   &grid_no_of_errors, &grid_no_of_erasures,
                   grid->corrected_codewords,
                   &corrected_codewords_length, grid) == 0) {
    grid->no_of_errors = grid_no_of_errors;
    grid->no_of_erasures = grid_no_of_erasures;
    ecc200_decode(grid->corrected_codewords,
                  corrected_codewords_length, result,
                  gs1_result, iso15434_result,
                  iso15434_uii,
                  gs1_url, debug);
    if (human_readable == 1) {
      /* if there is a GS1 formatted decode then return that instead */
      if (strlen(gs1_result) > 0) {
        grid->gs1_datamatrix = 1;
        result[0] = 0;
        if (strlen(gs1_url) == 0) {
          decode_strcat(result, "STANDARD: GS1\n");
        }
        decode_strcat(result, gs1_result);
        if (strlen(gs1_url) == 0) {
          /* remove the final newline */
          result[strlen(result)-1] = 0;
        }
      }
      /* if there is an ISO 15434 decode then return that instead */
      if (strlen(iso15434_result) > 0) {
        grid->iso15434_datamatrix = 1;
        result[0] = 0;
        decode_strcat(result, iso15434_result);
        if (strlen(iso15434_uii) > 0) {
          decode_strcat(result, "UII: ");
          decode_strcat(result, iso15434_uii);
        }
        else {
          /* remove the final newline */
          result[strlen(result)-1] = 0;
        }
      }
      /* if there is a HIBC decode then return that instead */
      if (strlen(result) > 0) {
        if (result[0] == '+') {
          hibc_semantics(result, hibc_result, debug);
          if (strlen(hibc_result) > 0) {
            grid->hibc_datamatrix = 1;
            result[0] = 0;
            decode_strcat(result, hibc_result);
            /* remove the final newline */
            result[strlen(result)-1] = 0;
          }
        }
      }
    }
    grid->unused_error_correction =
      get_unused_error_correction(codewords_length,
                                  error_correcting_words,
                                  &grid_no_of_errors,
                                  &grid_no_of_erasures);

    grid->no_of_errors = grid_no_of_errors;
    grid->no_of_erasures = grid_no_of_erasures;
  }
  free(gs1_result);
  free(iso15434_result);
  free(hibc_result);
  free(iso15434_uii);

  if (condensed == 1) {
    /* restore the original grid dimensions */
    grid->dimension_x = original_dimension_x;
    grid->dimension_y = original_dimension_y;
  }
}
