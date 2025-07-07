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
 */
void gs1_semantics(char result[],
                   char gs1_result[],
                   char gs1_url[],
                   unsigned char debug,
                   int * application_identifier,
                   unsigned char * application_identifier_length,
                   int * application_data_start,
                   int * application_data_end)
{
  char * app_id_str, * data_str, * date_str;
  char app_id_str2[10];
  unsigned char is_digital_link = 0;

  int curr_pos = (int)strlen(result);
  if (curr_pos != (*application_data_end)) {
    return;
  }

  if ((*application_data_end) - (*application_data_start) ==
      (*application_identifier_length)) {
    /* read application identifier */
    app_id_str = &result[*application_data_start];
    *application_identifier = atoi(app_id_str);

    /* see https://www.gs1.org/docs/barcodes/GSCN-25-081-UN-ECE-Recommendation20.pdf */
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
      break;
    }
    case 22: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 2;
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
      break;
    }
    case 41: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 235: {
      *application_data_end = curr_pos + 28;
      *application_identifier_length = 3;
      break;
    }
    case 240: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 241: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 242: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 3;
      break;
    }
    case 243: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      break;
    }
    case 250: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 251: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 253: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 254: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      break;
    }
    case 255: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
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
      break;
    }
    case 391: {
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 4;
      break;
    }
    case 392: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      break;
    }
    case 393: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
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
      }

      if (strlen(gs1_url) > 0) {
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
