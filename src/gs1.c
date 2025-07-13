/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  GS1 semantics functions
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

/**
 * \brief state machine for handling GS1 semantics
 * Also see https://github.com/gs1/gs1-syntax-dictionary/blob/main/gs1-syntax-dictionary.txt
 * \param result Plaintext decode string
 * \param gs1_result human readable GS1 formatted decode string
 * \param gs1_url url prefix to use with GS1 digital link
 * \param debug set to 1 if in debug mode
 * \param application_identifier Current GS1 application identifier
 * \param application_identifier_length length of the application identifier string in bytes
 * \param application_data_start position in the result decode string where application data starts
 * \param application_data_end position in the result decode string where application data ends
 * \param application_data_variable if data is variable length how many characters are variable
 */
void gs1_semantics(char result[],
                   char gs1_result[],
                   char gs1_url[],
                   unsigned char debug,
                   int * application_identifier,
                   unsigned char * application_identifier_length,
                   int * application_data_start,
                   int * application_data_end,
                   unsigned char * application_data_variable)
{
  char * app_id_str, * data_str, * date_str;
  char app_id_str2[10];
  unsigned char is_digital_link = 0;

  int data_length = (*application_data_end) - (*application_data_start);

  int curr_pos = (int)strlen(result);
  if (curr_pos != (*application_data_end)) {
    if (curr_pos <= 1) return;

    /* if the minimum data length has not yet arrived */
    if (*application_data_variable > 0) {
      if (curr_pos < (*application_data_end) - (*application_data_variable)) {
        return;
      }
    }

    /* look for FNC1 on variable data lengths */
    if (!((*application_data_variable > 0) &&
          (result[curr_pos-1] == (char)29))) {
      return;
    }
  }

  if (data_length == (*application_identifier_length)) {
    /* read application identifier */
    app_id_str = &result[*application_data_start];
    *application_identifier = atoi(app_id_str);
    *application_data_variable = 0;

    /* see https://www.gs1.org/docs/barcodes/GSCN-25-081-UN-ECE-Recommendation20.pdf
       https://www.tec-it.com/en/support/knowbase/gs1-application-identifier/Default.aspx
     */
    switch(*application_identifier) {
    case 0: {
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 2;
      break;
    }
    case 1: {
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 2;
      break;
    }
    case 2: {
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 2;
      break;
    }
    case 3: {
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 2;
      break;
    }
    case 4: {
      *application_data_end = curr_pos + 16;
      *application_identifier_length = 2;
      break;
    }
    case 10: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      *application_data_variable = 20;
      break;
    }
    case 11: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 12: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 13: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 15: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 16: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 17: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 18: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 19: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 21: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 2;
      *application_data_variable = 20;
      break;
    }
    case 22: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 2;
      *application_data_variable = 20;
      break;
    }
    case 30: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 31: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 32: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 33: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 34: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 35: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 36: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 37: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 2;
      *application_data_variable = 8;
      break;
    }
    case 41: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 90: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 2;
      *application_data_variable = 30;
      break;
    }
    case 91: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 92: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 93: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 94: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 95: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 96: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 97: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 98: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 99: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 235: {
      *application_data_end = curr_pos + 28;
      *application_identifier_length = 3;
      *application_data_variable = 28;
      break;
    }
    case 240: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 241: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 242: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 3;
      *application_data_variable = 6;
      break;
    }
    case 243: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 250: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 251: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 253: {
      *application_data_end = curr_pos + 13 + 17;
      *application_identifier_length = 3;
      *application_data_variable = 17;
      break;
    }
    case 254: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 255: {
      *application_data_end = curr_pos + 13 + 12;
      *application_identifier_length = 3;
      *application_data_variable = 12;
      break;
    }
    case 310: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 311: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 312: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 313: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 314: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 315: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 316: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 320: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 321: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 322: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 323: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 324: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 325: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 326: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 327: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 328: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 329: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 330: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 331: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 332: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 333: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 334: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 335: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 336: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 337: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 340: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 341: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 342: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 343: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 344: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 345: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 346: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 347: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 348: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 349: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 350: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 351: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 352: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 353: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 354: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 355: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 356: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 357: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 360: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 361: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 362: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 363: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 364: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 365: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 366: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 367: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 368: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 369: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 390: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 391: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 392: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 393: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 394: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      break;
    }
    case 395: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 400: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 401: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 402: {
      *application_data_end = curr_pos + 17;
      *application_identifier_length = 3;
      break;
    }
    case 403: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 410: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 411: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 412: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 413: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 414: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 415: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 420: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 421: {
      *application_data_end = curr_pos + 3 + 9;
      *application_identifier_length = 3;
      *application_data_variable = 9;
      break;
    }
    case 422: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 3;
      break;
    }
    case 423: {
      *application_data_end = curr_pos + 3 + 12;
      *application_identifier_length = 3;
      *application_data_variable = 12;
      break;
    }
    case 424: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 3;
      break;
    }
    case 425: {
      *application_data_end = curr_pos + 3 + 12;
      *application_identifier_length = 3;
      *application_data_variable = 12;
      break;
    }
    case 426: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 3;
      break;
    }
    case 703: {
      *application_data_end = curr_pos + 3 + 27;
      *application_identifier_length = 4;
      *application_data_variable = 27;
      break;
    }
    case 710: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 711: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 712: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 713: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 714: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 7001: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 4;
      break;
    }
    case 7002: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 7003: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      break;
    }
    case 7004: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 7005: {
      *application_data_end = curr_pos + 12;
      *application_identifier_length = 4;
      *application_data_variable = 12;
      break;
    }
    case 7006: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 7007: {
      *application_data_end = curr_pos + 6 + 6;
      *application_identifier_length = 4;
      *application_data_variable = 6;
      break;
    }
    case 7008: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 4;
      *application_data_variable = 3;
      break;
    }
    case 7009: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      *application_data_variable = 10;
      break;
    }
    case 7010: {
      *application_data_end = curr_pos + 2;
      *application_identifier_length = 4;
      *application_data_variable = 2;
      break;
    }
    case 7011: {
      *application_data_end = curr_pos + 6 + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 7020: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7021: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7022: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7023: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 7040: {
      *application_data_end = curr_pos + 1 + 3;
      *application_identifier_length = 4;
      break;
    }
    case 7041: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 7240: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7241: {
      *application_data_end = curr_pos + 2;
      *application_identifier_length = 4;
      break;
    }
    case 7242: {
      *application_data_end = curr_pos + 25;
      *application_identifier_length = 4;
      *application_data_variable = 25;
      break;
    }
    case 7250: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 4;
      break;
    }
    case 7251: {
      *application_data_end = curr_pos + 12;
      *application_identifier_length = 4;
      break;
    }
    case 7252: {
      *application_data_end = curr_pos + 1;
      *application_identifier_length = 4;
      break;
    }
    case 7253: {
      *application_data_end = curr_pos + 40;
      *application_identifier_length = 4;
      *application_data_variable = 40;
      break;
    }
    case 7254: {
      *application_data_end = curr_pos + 40;
      *application_identifier_length = 4;
      *application_data_variable = 40;
      break;
    }
    case 7255: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      *application_data_variable = 10;
      break;
    }
    case 7256: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 4;
      *application_data_variable = 90;
      break;
    }
    case 7257: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 7258: {
      *application_data_end = curr_pos + 1 + 1 + 1;
      *application_identifier_length = 4;
      break;
    }
    case 7259: {
      *application_data_end = curr_pos + 40;
      *application_identifier_length = 4;
      *application_data_variable = 40;
      break;
    }
    case 8001: {
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 4;
      break;
    }
    case 8002: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 8003: {
      *application_data_end = curr_pos + 14 + 16;
      *application_identifier_length = 4;
      *application_data_variable = 16;
      break;
    }
    case 8004: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 8005: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 8006: {
      *application_data_end = curr_pos + 14 + 2 + 2;
      *application_identifier_length = 4;
      break;
    }
    case 8007: {
      *application_data_end = curr_pos + 34;
      *application_identifier_length = 4;
      *application_data_variable = 34;
      break;
    }
    case 8008: {
      *application_data_end = curr_pos + 8 + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 8010: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 8011: {
      *application_data_end = curr_pos + 12;
      *application_identifier_length = 4;
      *application_data_variable = 12;
      break;
    }
    case 8012: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 8013: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 8017: {
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 4;
      break;
    }
    case 8018: {
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 4;
      break;
    }
    case 8019: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      *application_data_variable = 10;
      break;
    }
    case 8020: {
      *application_data_end = curr_pos + 25;
      *application_identifier_length = 4;
      *application_data_variable = 25;
      break;
    }
    case 8026: {
      *application_data_end = curr_pos + 14 + 2 + 2;
      *application_identifier_length = 4;
      break;
    }
    case 8030: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 4;
      *application_data_variable = 90;
      break;
    }
    case 8110: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 8111: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      break;
    }
    case 8112: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 8200: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
   }
  }
  else {
    /* read data associated with the application identifier */
    *application_data_end = curr_pos + (*application_identifier_length);
    data_str = &result[*application_data_start];
    date_str = NULL;

    if (strlen(data_str) > 0) {
      /* see https://www.gs1.org/docs/barcodes/GSCN-25-081-UN-ECE-Recommendation20.pdf */

      app_id_str2[0] = 0;
      if ((*application_identifier >= 0) && (*application_identifier < 99999)) {
        if (*application_identifier < 10) {
          sprintf(&app_id_str2[0], "0%d", *application_identifier);
        }
        else {
          sprintf(&app_id_str2[0], "%d", *application_identifier);
        }
      }

      if (gs1_url != NULL) {
        if (strlen(gs1_url) > 0) {
          /* build the GS1 digital link */
          if (strlen(gs1_result) == 0) {
            if (gs1_url[0] != '.') {
              decode_strcat(gs1_result, gs1_url);
            }
          }
          if (gs1_url[0] != '.') {
            decode_strcat_char(gs1_result, '/');
            decode_strcat(gs1_result, &app_id_str2[0]);
          }
          else {
            decode_strcat_char(gs1_result, '(');
            decode_strcat(gs1_result, &app_id_str2[0]);
            decode_strcat_char(gs1_result, ')');
          }
          is_digital_link = 1;
        }
      }

      *application_data_variable = 0;

      switch(*application_identifier) {
      case 0: {
        if (debug == 1) printf("SSCC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SSCC: ");
        }
        break;
      }
      case 1: {
        if (debug == 1) printf("GTIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GTIN: ");
        }
        break;
      }
      case 2: {
        if (debug == 1) printf("CONTENT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CONTENT: ");
        }
        break;
      }
      case 3: {
        if (debug == 1) printf("MTO GTIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MTO GTIN: ");
        }
        break;
      }
      case 4: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 10: {
        if (debug == 1) printf("BATCH/LOT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BATCH/LOT: ");
        }
        break;
      }
      case 11: {
        if (debug == 1) printf("PROD DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROD DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 12: {
        if (debug == 1) printf("DUE DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DUE DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 13: {
        if (debug == 1) printf("PACK DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PACK DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 15: {
        if (debug == 1) printf("BEST BEFORE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BEST BEFORE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 16: {
        if (debug == 1) printf("SELL BY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SELL BY: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 17: {
        if (debug == 1) printf("EXPIRY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "EXPIRY: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 18: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 19: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 21: {
        if (debug == 1) printf("SERIAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SERIAL: ");
        }
        break;
      }
      case 22: {
        if (debug == 1) printf("CPV ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CPV: ");
        }
        break;
      }
      case 30: {
        if (debug == 1) printf("VAR COUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VAR COUNT: ");
        }
        break;
      }
      case 31: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 32: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 33: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 34: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 35: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 36: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 37: {
        if (debug == 1) printf("COUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNT: ");
        }
        break;
      }
      case 41: {
        if (debug == 1) printf("ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ID: ");
        }
        break;
      }
      case 90: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 91: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 92: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 93: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 94: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 95: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 96: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 97: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 98: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 99: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 235: {
        if (debug == 1) printf("TPX ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "TPX: ");
        }
        break;
      }
      case 240: {
        if (debug == 1) printf("ADDITIONAL ID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ADDITIONAL ID: ");
        }
        break;
      }
      case 241: {
        if (debug == 1) printf("CUST PART No ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CUST PART No: ");
        }
        break;
      }
      case 242: {
        if (debug == 1) printf("MTO VARIANT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MTO VARIANT: ");
        }
        break;
      }
      case 243: {
        if (debug == 1) printf("PCN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PCN: ");
        }
        break;
      }
      case 250: {
        if (debug == 1) printf("SECONDARY SERIAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SECONDARY SERIAL: ");
        }
        break;
      }
      case 251: {
        if (debug == 1) printf("REF TO SOURCE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REF TO SOURCE: ");
        }
        break;
      }
      case 253: {
        if (debug == 1) printf("GDTI ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GDTT: ");
        }
        break;
      }
      case 254: {
        if (debug == 1) printf("GLN EXTENSION COMPONENT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GLN EXTENSION COMPONENT: ");
        }
        break;
      }
      case 255: {
        if (debug == 1) printf("GCN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GCN: ");
        }
        break;
      }
      case 310: {
        if (debug == 1) printf("NET WEIGHT (kg) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET WEIGHT (kg): ");
        }
        break;
      }
      case 311: {
        if (debug == 1) printf("LENGTH (m) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (m): ");
        }
        break;
      }
      case 312: {
        if (debug == 1) printf("WIDTH (m) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (m): ");
        }
        break;
      }
      case 313: {
        if (debug == 1) printf("HEIGHT (m) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (m): ");
        }
        break;
      }
      case 314: {
        if (debug == 1) printf("AREA (m2) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (m2): ");
        }
        break;
      }
      case 315: {
        if (debug == 1) printf("NET VOLUME (l) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET VOLUME (l): ");
        }
        break;
      }
      case 316: {
        if (debug == 1) printf("NET VOLUME (m3) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET VOLUME (m3): ");
        }
        break;
      }
      case 320: {
        if (debug == 1) printf("NET WEIGHT (lb) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET WEIGHT (lb): ");
        }
        break;
      }
      case 321: {
        if (debug == 1) printf("LENGTH (in) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (in): ");
        }
        break;
      }
      case 322: {
        if (debug == 1) printf("LENGTH (ft) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (ft): ");
        }
        break;
      }
      case 323: {
        if (debug == 1) printf("LENGTH (yd) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (yd): ");
        }
        break;
      }
      case 324: {
        if (debug == 1) printf("WIDTH (in) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (in): ");
        }
        break;
      }
      case 325: {
        if (debug == 1) printf("WIDTH (ft) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (ft): ");
        }
        break;
      }
      case 326: {
        if (debug == 1) printf("WIDTH (yd) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (yd): ");
        }
        break;
      }
      case 327: {
        if (debug == 1) printf("HEIGHT (in) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (in): ");
        }
        break;
      }
      case 328: {
        if (debug == 1) printf("HEIGHT (ft) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (ft): ");
        }
        break;
      }
      case 329: {
        if (debug == 1) printf("HEIGHT (yd) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (yd): ");
        }
        break;
      }
      case 330: {
        if (debug == 1) printf("GROSS WEIGHT (kg) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GROSS WEIGHT (kg): ");
        }
        break;
      }
      case 331: {
        if (debug == 1) printf("LENGTH (m), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (m), log: ");
        }
        break;
      }
      case 332: {
        if (debug == 1) printf("WIDTH (m), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (m), log: ");
        }
        break;
      }
      case 333: {
        if (debug == 1) printf("HEIGHT (m), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (m), log: ");
        }
        break;
      }
      case 334: {
        if (debug == 1) printf("AREA (m2), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (m2), log: ");
        }
        break;
      }
      case 335: {
        if (debug == 1) printf("VOLUME (l), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (l), log: ");
        }
        break;
      }
      case 336: {
        if (debug == 1) printf("VOLUME (m3), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (m3), log: ");
        }
        break;
      }
      case 337: {
        if (debug == 1) printf("KG PER m2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "KG PER m2: ");
        }
        break;
      }
      case 340: {
        if (debug == 1) printf("GROSS WEIGHT (lb) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GROSS WEIGHT (lb): ");
        }
        break;
      }
      case 341: {
        if (debug == 1) printf("LENGTH (in), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (in), log: ");
        }
        break;
      }
      case 342: {
        if (debug == 1) printf("LENGTH (ft), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (ft), log: ");
        }
        break;
      }
      case 343: {
        if (debug == 1) printf("LENGTH (yd), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LENGTH (yd), log: ");
        }
        break;
      }
      case 344: {
        if (debug == 1) printf("WIDTH (in), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (in), log: ");
        }
        break;
      }
      case 345: {
        if (debug == 1) printf("WIDTH (ft), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (ft), log: ");
        }
        break;
      }
      case 346: {
        if (debug == 1) printf("WIDTH (yd), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "WIDTH (yd), log: ");
        }
        break;
      }
      case 347: {
        if (debug == 1) printf("HEIGHT (in), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (in), log: ");
        }
        break;
      }
      case 348: {
        if (debug == 1) printf("HEIGHT (ft), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (ft), log: ");
        }
        break;
      }
      case 349: {
        if (debug == 1) printf("HEIGHT (yd), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HEIGHT (yd), log: ");
        }
        break;
      }
      case 350: {
        if (debug == 1) printf("AREA (in2) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (in2): ");
        }
        break;
      }
      case 351: {
        if (debug == 1) printf("AREA (ft2) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (ft2): ");
        }
        break;
      }
      case 352: {
        if (debug == 1) printf("AREA (yd2) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (yd2): ");
        }
        break;
      }
      case 353: {
        if (debug == 1) printf("AREA (in2), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (in2), log: ");
        }
        break;
      }
      case 354: {
        if (debug == 1) printf("AREA (ft2), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (ft2), log: ");
        }
        break;
      }
      case 355: {
        if (debug == 1) printf("AREA (yd2), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AREA (yd2), log: ");
        }
        break;
      }
      case 356: {
        if (debug == 1) printf("NET WEIGHT (tr oz) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET WEIGHT (tr oz): ");
        }
        break;
      }
      case 357: {
        if (debug == 1) printf("NET VOLUME (oz) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET VOLUME (oz): ");
        }
        break;
      }
      case 360: {
        if (debug == 1) printf("NET VOLUME (qt US) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET VOLUME (qt US): ");
        }
        break;
      }
      case 361: {
        if (debug == 1) printf("NET VOLUME (gal US) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NET VOLUME (gal US): ");
        }
        break;
      }
      case 362: {
        if (debug == 1) printf("VOLUME (qt US), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (qt US): ");
        }
        break;
      }
      case 363: {
        if (debug == 1) printf("VOLUME (gal US), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (gal US): ");
        }
        break;
      }
      case 364: {
        if (debug == 1) printf("VOLUME (in3) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (in3): ");
        }
        break;
      }
      case 365: {
        if (debug == 1) printf("VOLUME (ft3) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (ft3): ");
        }
        break;
      }
      case 366: {
        if (debug == 1) printf("VOLUME (yd3) ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (yd3): ");
        }
        break;
      }
      case 367: {
        if (debug == 1) printf("VOLUME (in3), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (in3), log: ");
        }
        break;
      }
      case 368: {
        if (debug == 1) printf("VOLUME (ft3), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (ft3), log: ");
        }
        break;
      }
      case 369: {
        if (debug == 1) printf("VOLUME (yd3), log ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VOLUME (yd3), log: ");
        }
        break;
      }
      case 390: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
        }
        break;
      }
      case 391: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
        }
        break;
      }
      case 392: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
        }
        break;
      }
      case 393: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
        }
        break;
      }
      case 394: {
        if (debug == 1) printf("PRCNT OFF ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRCNT OFF: ");
        }
        break;
      }
      case 395: {
        if (debug == 1) printf("PRICE/UoM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE/UoM: ");
        }
        break;
      }
      case 400: {
        if (debug == 1) printf("ORDER NUMBER ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ORDER NUMBER: ");
        }
        break;
      }
      case 401: {
        if (debug == 1) printf("GINC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GINC: ");
        }
        break;
      }
      case 402: {
        if (debug == 1) printf("GSIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GSIN: ");
        }
        break;
      }
      case 403: {
        if (debug == 1) printf("ROUTE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ROUTE: ");
        }
        break;
      }
      case 410: {
        if (debug == 1) printf("SHIP TO LOC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO LOC: ");
        }
        break;
      }
      case 411: {
        if (debug == 1) printf("BILL TO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BILL TO: ");
        }
        break;
      }
      case 412: {
        if (debug == 1) printf("PURCHASE FROM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PURCHASE FROM: ");
        }
        break;
      }
      case 413: {
        if (debug == 1) printf("SHIP FOR LOC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP FOR LOC: ");
        }
        break;
      }
      case 414: {
        if (debug == 1) printf("LOCN NO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LOCN NO: ");
        }
        break;
      }
      case 415: {
        if (debug == 1) printf("PAY TO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PAY TO: ");
        }
        break;
      }
      case 420: {
        if (debug == 1) printf("SHIP TO POST ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO POST: ");
        }
        break;
      }
      case 421: {
        if (debug == 1) printf("SHIP TO POST ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO POST: ");
        }
        break;
      }
      case 422: {
        if (debug == 1) printf("ORIGIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ORIGIN: ");
        }
        break;
      }
      case 423: {
        if (debug == 1) printf("COUNTRY – INITIAL PROCESS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – INITIAL PROCESS: ");
        }
        break;
      }
      case 424: {
        if (debug == 1) printf("COUNTRY – PROCESS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – PROCESS: ");
        }
        break;
      }
      case 425: {
        if (debug == 1) printf("COUNTRY – DISASSEMBLY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – DISASSEMBLY: ");
        }
        break;
      }
      case 426: {
        if (debug == 1) printf("COUNTRY – FULL PROCESS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – FULL PROCESS: ");
        }
        break;
      }
      case 703: {
        if (debug == 1) printf("PROCESSOR # s4 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROCESSOR # s4: ");
        }
        break;
      }
      case 710: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 711: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 712: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 713: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 714: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 7001: {
        if (debug == 1) printf("NSN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NSN: ");
        }
        break;
      }
      case 7002: {
        if (debug == 1) printf("MEAT CUT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MEAT CUT: ");
        }
        break;
      }
      case 7003: {
        if (debug == 1) printf("EXPIRY TIME YYMMDDHHMM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "EXPIRY TIME YYMMDDHHMM: ");
        }
        break;
      }
      case 7004: {
        if (debug == 1) printf("ACTIVE POTENCY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ACTIVE POTENCY: ");
        }
        break;
      }
      case 7005: {
        if (debug == 1) printf("CATCH AREA ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CATCH AREA: ");
        }
        break;
      }
      case 7006: {
        if (debug == 1) printf("FIRST FREEZE DATE YYMMDD ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FIRST FREEZE DATE YYMMDD: ");
        }
        break;
      }
      case 7007: {
        if (debug == 1) printf("HARVEST DATE YYMMDD ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HARVEST DATE YYMMDD: ");
        }
        break;
      }
      case 7008: {
        if (debug == 1) printf("AQUATIC SPECIES ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AQUATIC SPECIES: ");
        }
        break;
      }
      case 7009: {
        if (debug == 1) printf("FISHING GEAR TYPE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FISHING GEAR TYPE: ");
        }
        break;
      }
      case 7010: {
        if (debug == 1) printf("PROD METHOD ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROD METHOD: ");
        }
        break;
      }
      case 7011: {
        if (debug == 1) printf("TEST BY DATE YYMMDD ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "TEST BY DATE YYMMDD: ");
        }
        break;
      }
      case 7020: {
        if (debug == 1) printf("REFURB LOT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REFURB LOT: ");
        }
        break;
      }
      case 7021: {
        if (debug == 1) printf("FUNC STAT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FUNC STAT: ");
        }
        break;
      }
      case 7022: {
        if (debug == 1) printf("REV STAT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REV STAT: ");
        }
        break;
      }
      case 7023: {
        if (debug == 1) printf("GIAI – ASSEMBLY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GIAI – ASSEMBLY: ");
        }
        break;
      }
      case 7040: {
        if (debug == 1) printf("UIC+EXT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "UIC+EXT: ");
        }
        break;
      }
      case 7041: {
        if (debug == 1) printf("UFRGT UNIT TYPE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "UFRGT UNIT TYPE: ");
        }
        break;
      }
      case 7240: {
        if (debug == 1) printf("PROTOCOL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROTOCOL: ");
        }
        break;
      }
      case 7241: {
        if (debug == 1) printf("AIDC MEDIA TYPE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AIDC MEDIA TYPE: ");
        }
        break;
      }
      case 7242: {
        if (debug == 1) printf("VCN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VCN: ");
        }
        break;
      }
      case 7250: {
        if (debug == 1) printf("DOB YYYYMMDD ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DOB YYYYMMDD: ");
        }
        break;
      }
      case 7251: {
        if (debug == 1) printf("DOB TIME YYYYMMDDHHMM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DOB TIME YYYYMMDDHHMM: ");
        }
        break;
      }
      case 7252: {
        if (debug == 1) printf("BIO SEX ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BIO SEX: ");
        }
        break;
      }
      case 7253: {
        if (debug == 1) printf("FAMILY NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FAMILY NAME: ");
        }
        break;
      }
      case 7254: {
        if (debug == 1) printf("GIVEN NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GIVEN NAME: ");
        }
        break;
      }
      case 7255: {
        if (debug == 1) printf("SUFFIX ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SUFFIX: ");
        }
        break;
      }
      case 7256: {
        if (debug == 1) printf("FULL NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FULL NAME: ");
        }
        break;
      }
      case 7257: {
        if (debug == 1) printf("PERSON ADDR ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PERSON ADDR: ");
        }
        break;
      }
      case 7258: {
        if (debug == 1) printf("BIRTH SEQUENCE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BIRTH SEQUENCE: ");
        }
        break;
      }
      case 7259: {
        if (debug == 1) printf("BABY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BABY: ");
        }
        break;
      }
      case 8001: {
        if (debug == 1) printf("DIMENSIONS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DIMENSIONS: ");
        }
        break;
      }
      case 8002: {
        if (debug == 1) printf("CMT NO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CMT NO: ");
        }
        break;
      }
      case 8003: {
        if (debug == 1) printf("GRAI ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GRAI: ");
        }
        break;
      }
      case 8004: {
        if (debug == 1) printf("GIAI ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GRAI: ");
        }
        break;
      }
      case 8005: {
        if (debug == 1) printf("PRICE PER UNIT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE PER UNIT: ");
        }
        break;
      }
      case 8006: {
        if (debug == 1) printf("ITIP ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ITIP: ");
        }
        break;
      }
      case 8007: {
        if (debug == 1) printf("IBAN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "IBAN: ");
        }
        break;
      }
      case 8008: {
        if (debug == 1) printf("PROD TIME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROD TIME: ");
        }
        break;
      }
      case 8010: {
        if (debug == 1) printf("CPID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CPID: ");
        }
        break;
      }
      case 8011: {
        if (debug == 1) printf("CPID SERIAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CPID SERIAL: ");
        }
        break;
      }
      case 8012: {
        if (debug == 1) printf("VERSION ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VERSION: ");
        }
        break;
      }
      case 8013: {
        if (debug == 1) printf("GMN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GMN: ");
        }
        break;
      }
      case 8017: {
        if (debug == 1) printf("GSRN - PROVIDER ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GSRN - PROVIDER: ");
        }
        break;
      }
      case 8018: {
        if (debug == 1) printf("GSRN - RECIPIENT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GSRN - RECIPIENT: ");
        }
        break;
      }
      case 8019: {
        if (debug == 1) printf("SRIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SRIN: ");
        }
        break;
      }
      case 8020: {
        if (debug == 1) printf("REF NO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REF NO: ");
        }
        break;
      }
      case 8026: {
        if (debug == 1) printf("ITIP CONTENT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ITIP CONTENT: ");
        }
        break;
      }
      case 8030: {
        if (debug == 1) printf("DIGSIG ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DIGSIG: ");
        }
        break;
      }
      case 8110: {
        if (debug == 1) printf("COUPON ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUPON: ");
        }
        break;
      }
      case 8111: {
        if (debug == 1) printf("POINTS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "POINTS: ");
        }
        break;
      }
      case 8112: {
        if (debug == 1) printf("COUPON ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUPON: ");
        }
        break;
      }
      case 8200: {
        if (debug == 1) printf("PRODUCT URL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRODUCT URL: ");
        }
        break;
      }
      }

      unsigned char build_digital_link = 0;
      if (gs1_url != NULL) {
        if (strlen(gs1_url) > 0) {
          build_digital_link = 1;
        }
      }
      if (build_digital_link == 1) {
        /* build the GS1 digital link */
        if (gs1_url[0] != '.') {
          decode_strcat_char(gs1_result, '/');
        }
        decode_strcat(gs1_result, data_str);
      }
      else {
        /* human readable */
        if (date_str != NULL) {
          decode_strcat(gs1_result, date_str);
          free(date_str);
        }
        else {
          decode_strcat(gs1_result, data_str);
        }
        decode_strcat_char(gs1_result, '\n');
      }
      if (debug == 1) {
        printf("| (%d)%s | ", *application_identifier, &result[*application_data_start]);
      }
    }
    *application_identifier = 0;
  }
  *application_data_start = curr_pos;
}
