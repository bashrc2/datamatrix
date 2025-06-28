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

void gs1_semantics(char result[],
                   char gs1_result[],
                   unsigned char debug,
                   int * application_identifier,
                   unsigned char * application_identifier_length,
                   int * application_data_start,
                   int * application_data_end)
{
  int curr_pos = (int)strlen(result);
  if (curr_pos != (*application_data_end)) {
    return;
  }

  if ((*application_data_end) - (*application_data_start) ==
      (*application_identifier_length)) {
    /* read application identifier */
    *application_identifier = atoi(&result[*application_data_start]);
    /* see https://www.gs1.org/docs/barcodes/GSCN-25-081-UN-ECE-Recommendation20.pdf */
    switch(*application_identifier) {
    case 0: {
      if (debug == 1) printf("SSCC ");
      decode_strcat(gs1_result, "SSCC: ");
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 2;
      break;
    }
    case 1: {
      if (debug == 1) printf("GTIN ");
      decode_strcat(gs1_result, "GTIN: ");
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 2;
      break;
    }
    case 2: {
      if (debug == 1) printf("CONTENT ");
      decode_strcat(gs1_result, "CONTENT: ");
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 2;
      break;
    }
    case 3: {
      if (debug == 1) printf("MTO GTIN ");
      decode_strcat(gs1_result, "MTO GTIN: ");
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 2;
      break;
    }
    case 4: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 16;
      *application_identifier_length = 2;
      break;
    }
    case 10: {
      if (debug == 1) printf("BATCH/LOT ");
      decode_strcat(gs1_result, "BATCH/LOT: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 11: {
      if (debug == 1) printf("PROD DATE ");
      decode_strcat(gs1_result, "PROD DATE: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 12: {
      if (debug == 1) printf("DUE DATE ");
      decode_strcat(gs1_result, "DUE DATE: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 13: {
      if (debug == 1) printf("PACK DATE ");
      decode_strcat(gs1_result, "PACK DATE: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 15: {
      if (debug == 1) printf("BEST BEFORE ");
      decode_strcat(gs1_result, "BEST BEFORE: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 16: {
      if (debug == 1) printf("SELL BY ");
      decode_strcat(gs1_result, "SELL BY: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 17: {
      if (debug == 1) printf("EXPIRY ");
      decode_strcat(gs1_result, "EXPIRY: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 18: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 19: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 2;
      break;
    }
    case 21: {
      if (debug == 1) printf("SERIAL ");
      decode_strcat(gs1_result, "SERIAL: ");
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 2;
      break;
    }
    case 22: {
      if (debug == 1) printf("CPV ");
      decode_strcat(gs1_result, "CPV: ");
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 2;
      break;
    }
    case 30: {
      if (debug == 1) printf("VAR COUNT ");
      decode_strcat(gs1_result, "VAR COUNT: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 31: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 32: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 33: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 34: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 35: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 36: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 37: {
      if (debug == 1) printf("COUNT ");
      decode_strcat(gs1_result, "COUNT: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 2;
      break;
    }
    case 41: {
      if (debug == 1) printf("ID ");
      decode_strcat(gs1_result, "ID: ");
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 235: {
      if (debug == 1) printf("TPX ");
      decode_strcat(gs1_result, "TPX: ");
      *application_data_end = curr_pos + 28;
      *application_identifier_length = 3;
      break;
    }
    case 240: {
      if (debug == 1) printf("ADDITIONAL ID ");
      decode_strcat(gs1_result, "ADDITIONAL ID: ");
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 241: {
      if (debug == 1) printf("CUST PART No ");
      decode_strcat(gs1_result, "CUST PART No: ");
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 242: {
      if (debug == 1) printf("MTO VARIANT ");
      decode_strcat(gs1_result, "MTO VARIANT: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 3;
      break;
    }
    case 243: {
      if (debug == 1) printf("PCN ");
      decode_strcat(gs1_result, "PCN: ");
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      break;
    }
    case 250: {
      if (debug == 1) printf("SECONDARY SERIAL ");
      decode_strcat(gs1_result, "SECONDARY SERIAL: ");
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    case 251: {
      if (debug == 1) printf("REF TO SOURCE ");
      decode_strcat(gs1_result, "REF TO SOURCE: ");
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_identifier_length = 3;
      break;
    }
    case 253: {
      if (debug == 1) printf("GDTI ");
      decode_strcat(gs1_result, "GDTT: ");
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 254: {
      if (debug == 1) printf("GLN EXTENSION COMPONENT ");
      decode_strcat(gs1_result, "GLN EXTENSION COMPONENT: ");
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      break;
    }
    case 255: {
      if (debug == 1) printf("GCN ");
      decode_strcat(gs1_result, "GCN: ");
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 310: {
      if (debug == 1) printf("NET WEIGHT (kg) ");
      decode_strcat(gs1_result, "NET WEIGHT (kg): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 311: {
      if (debug == 1) printf("LENGTH (m) ");
      decode_strcat(gs1_result, "LENGTH (m): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 312: {
      if (debug == 1) printf("WIDTH (m) ");
      decode_strcat(gs1_result, "WIDTH (m): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 313: {
      if (debug == 1) printf("HEIGHT (m) ");
      decode_strcat(gs1_result, "HEIGHT (m): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 314: {
      if (debug == 1) printf("AREA (m2) ");
      decode_strcat(gs1_result, "AREA (m2): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 315: {
      if (debug == 1) printf("NET VOLUME (l) ");
      decode_strcat(gs1_result, "NET VOLUME (l): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 316: {
      if (debug == 1) printf("NET VOLUME (m3) ");
      decode_strcat(gs1_result, "NET VOLUME (m3): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 320: {
      if (debug == 1) printf("NET WEIGHT (lb) ");
      decode_strcat(gs1_result, "NET WEIGHT (lb): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 321: {
      if (debug == 1) printf("LENGTH (in) ");
      decode_strcat(gs1_result, "LENGTH (in): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 322: {
      if (debug == 1) printf("LENGTH (ft) ");
      decode_strcat(gs1_result, "LENGTH (ft): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 323: {
      if (debug == 1) printf("LENGTH (yd) ");
      decode_strcat(gs1_result, "LENGTH (yd): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 324: {
      if (debug == 1) printf("WIDTH (in) ");
      decode_strcat(gs1_result, "WIDTH (in): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 325: {
      if (debug == 1) printf("WIDTH (ft) ");
      decode_strcat(gs1_result, "WIDTH (ft): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 326: {
      if (debug == 1) printf("WIDTH (yd) ");
      decode_strcat(gs1_result, "WIDTH (yd): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 327: {
      if (debug == 1) printf("HEIGHT (in) ");
      decode_strcat(gs1_result, "HEIGHT (in): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 328: {
      if (debug == 1) printf("HEIGHT (ft) ");
      decode_strcat(gs1_result, "HEIGHT (ft): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 329: {
      if (debug == 1) printf("HEIGHT (yd) ");
      decode_strcat(gs1_result, "HEIGHT (yd): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 330: {
      if (debug == 1) printf("GROSS WEIGHT (kg) ");
      decode_strcat(gs1_result, "GROSS WEIGHT (kg): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 331: {
      if (debug == 1) printf("LENGTH (m), log ");
      decode_strcat(gs1_result, "LENGTH (m), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 332: {
      if (debug == 1) printf("WIDTH (m), log ");
      decode_strcat(gs1_result, "WIDTH (m), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 333: {
      if (debug == 1) printf("HEIGHT (m), log ");
      decode_strcat(gs1_result, "HEIGHT (m), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 334: {
      if (debug == 1) printf("AREA (m2), log ");
      decode_strcat(gs1_result, "AREA (m2), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 335: {
      if (debug == 1) printf("VOLUME (l), log ");
      decode_strcat(gs1_result, "VOLUME (l), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 336: {
      if (debug == 1) printf("VOLUME (m3), log ");
      decode_strcat(gs1_result, "VOLUME (m3), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 337: {
      if (debug == 1) printf("KG PER m2 ");
      decode_strcat(gs1_result, "KG PER m2: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 340: {
      if (debug == 1) printf("GROSS WEIGHT (lb) ");
      decode_strcat(gs1_result, "GROSS WEIGHT (lb): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 341: {
      if (debug == 1) printf("LENGTH (in), log ");
      decode_strcat(gs1_result, "LENGTH (in), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 342: {
      if (debug == 1) printf("LENGTH (ft), log ");
      decode_strcat(gs1_result, "LENGTH (ft), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 343: {
      if (debug == 1) printf("LENGTH (yd), log ");
      decode_strcat(gs1_result, "LENGTH (yd), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 344: {
      if (debug == 1) printf("WIDTH (in), log ");
      decode_strcat(gs1_result, "WIDTH (in), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 345: {
      if (debug == 1) printf("WIDTH (ft), log ");
      decode_strcat(gs1_result, "WIDTH (ft), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 346: {
      if (debug == 1) printf("WIDTH (yd), log ");
      decode_strcat(gs1_result, "WIDTH (yd), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 347: {
      if (debug == 1) printf("HEIGHT (in), log ");
      decode_strcat(gs1_result, "HEIGHT (in), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 348: {
      if (debug == 1) printf("HEIGHT (ft), log ");
      decode_strcat(gs1_result, "HEIGHT (ft), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 349: {
      if (debug == 1) printf("HEIGHT (yd), log ");
      decode_strcat(gs1_result, "HEIGHT (yd), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 350: {
      if (debug == 1) printf("AREA (in2) ");
      decode_strcat(gs1_result, "AREA (in2): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 351: {
      if (debug == 1) printf("AREA (ft2) ");
      decode_strcat(gs1_result, "AREA (ft2): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 352: {
      if (debug == 1) printf("AREA (yd2) ");
      decode_strcat(gs1_result, "AREA (yd2): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 353: {
      if (debug == 1) printf("AREA (in2), log ");
      decode_strcat(gs1_result, "AREA (in2), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 354: {
      if (debug == 1) printf("AREA (ft2), log ");
      decode_strcat(gs1_result, "AREA (ft2), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 355: {
      if (debug == 1) printf("AREA (yd2), log ");
      decode_strcat(gs1_result, "AREA (yd2), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 356: {
      if (debug == 1) printf("NET WEIGHT (tr oz) ");
      decode_strcat(gs1_result, "NET WEIGHT (tr oz): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 357: {
      if (debug == 1) printf("NET VOLUME (oz) ");
      decode_strcat(gs1_result, "NET VOLUME (oz): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 360: {
      if (debug == 1) printf("NET VOLUME (qt US) ");
      decode_strcat(gs1_result, "NET VOLUME (qt US): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 361: {
      if (debug == 1) printf("NET VOLUME (gal US) ");
      decode_strcat(gs1_result, "NET VOLUME (gal US): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 362: {
      if (debug == 1) printf("VOLUME (qt US), log ");
      decode_strcat(gs1_result, "VOLUME (qt US): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 363: {
      if (debug == 1) printf("VOLUME (gal US), log ");
      decode_strcat(gs1_result, "VOLUME (gal US): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 364: {
      if (debug == 1) printf("VOLUME (in3) ");
      decode_strcat(gs1_result, "VOLUME (in3): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 365: {
      if (debug == 1) printf("VOLUME (ft3) ");
      decode_strcat(gs1_result, "VOLUME (ft3): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 366: {
      if (debug == 1) printf("VOLUME (yd3) ");
      decode_strcat(gs1_result, "VOLUME (yd3): ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 367: {
      if (debug == 1) printf("VOLUME (in3), log ");
      decode_strcat(gs1_result, "VOLUME (in3), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 368: {
      if (debug == 1) printf("VOLUME (ft3), log ");
      decode_strcat(gs1_result, "VOLUME (ft3), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 369: {
      if (debug == 1) printf("VOLUME (yd3), log ");
      decode_strcat(gs1_result, "VOLUME (yd3), log: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 390: {
      if (debug == 1) printf("AMOUNT ");
      decode_strcat(gs1_result, "AMOUNT: ");
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      break;
    }
    case 391: {
      if (debug == 1) printf("AMOUNT ");
      decode_strcat(gs1_result, "AMOUNT: ");
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 4;
      break;
    }
    case 392: {
      if (debug == 1) printf("PRICE ");
      decode_strcat(gs1_result, "PRICE: ");
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      break;
    }
    case 393: {
      if (debug == 1) printf("PRICE ");
      decode_strcat(gs1_result, "PRICE: ");
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      break;
    }
    case 394: {
      if (debug == 1) printf("PRCNT OFF ");
      decode_strcat(gs1_result, "PRCNT OFF: ");
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      break;
    }
    case 395: {
      if (debug == 1) printf("PRICE/UoM ");
      decode_strcat(gs1_result, "PRICE/UoM: ");
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 400: {
      if (debug == 1) printf("ORDER NUMBER ");
      decode_strcat(gs1_result, "ORDER NUMBER: ");
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      break;
    }
    }
  }
  else {
    /* read data associated with the application identifier */
    *application_data_end = curr_pos + (*application_identifier_length);
    decode_strcat(gs1_result, &result[*application_data_start]);
    decode_strcat_char(gs1_result, '\n');
    if (debug == 1) {
      printf("| (%d)%s | ", *application_identifier, &result[*application_data_start]);
    }
    *application_identifier = 0;
  }
  *application_data_start = curr_pos;
}
