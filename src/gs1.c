/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  GS1 semantics functions
 *  Copyright (c) 2025-2026, Bob Mottram
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

#define APP_ID(end_len, id_len, variable_len) \
    *application_data_end = curr_pos + (end_len); \
    *application_identifier_length = (id_len); \
    *application_data_variable = (variable_len);

#define DECODE(title) \
    if (debug == 1) printf(title " ");  \
    if (is_digital_link == 0) { \
        decode_strcat(gs1_result, title ": "); \
    }

#define DECODE_DATE(title, dateformat) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      date_str = data_id_convert_date(dateformat, data_str); \
    }

#define DECODE_DECIMAL(title) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      decimal_str = get_decimal_value(*application_identifier, data_str); \
    }

#define DECODE_CURRENCY(title) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      curr_str = get_currency_value(*application_identifier, data_str); \
    }

#define DECODE_COUNTRY(title) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      country_str = get_country(data_str); \
    }

#define DECODE_COUNTRY_ALPHA2(title) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      country_str = get_country_alpha2(data_str); \
    }

#define DECODE_TEMPERATURE(title) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      temperature = get_temperature(data_str); \
    }

#define DECODE_PROCESSOR(title) \
    if (debug == 1) printf(title " "); \
    if (is_digital_link == 0) { \
      decode_strcat(gs1_result, title ": "); \
      processor_country_code[0] = data_str[0]; \
      processor_country_code[1] = data_str[1]; \
      processor_country_code[2] = data_str[2]; \
      processor_country_code[3] = 0; \
      processor_country_str = get_country(&processor_country_code[0]); \
    }

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
    char last_str[2];
    char company_prefix_code[4];
    char processor_country_code[4];
    char * app_id_str, * data_str, * date_str, * end_date_str;
    char * curr_str, * decimal_str, * country_str, * coupon_str, * issn_str;
    char * company_prefix_str, * processor_country_str, * package_type_str;
    unsigned char gtin_check_digit_passed = -1;
    unsigned char gsin_check_digit_passed = -1;
    unsigned char gsrn_check_digit_passed = -1;
    unsigned char sscc_check_digit_passed = -1;
    unsigned char grai_check_digit_passed = -1;
    unsigned char gln_check_digit_passed = -1;
    char sscc_package_type = ' ';
    char app_id_str2[10];
    char birth_sequence[2];
    char itip_piece_number_str[3];
    char itip_total_count_str[3];
    unsigned char is_digital_link = 0;
    int roll_width_mm = -1;
    int roll_diameter_mm = -1;
    int roll_length_metres = -1;
    int roll_no_of_splices = -1;
    int roll_winding_direction = -1;
    float temperature = UNKNOWN_VALUE;
    float latitude = UNKNOWN_VALUE, longitude = UNKNOWN_VALUE;

    int data_length = (*application_data_end) - (*application_data_start);

    int curr_pos = (int)strlen(result);
    int authority_to_leave = -1;
    int signature_required = -1;
    int aidc_media_type = -1;
    int bio_sex = -1;
    if (curr_pos != (*application_data_end)) {
        if (curr_pos <= 1) return;

        /* if the minimum data length has not yet arrived */
        if (*application_data_variable > 0) {
            if (curr_pos <
                    (*application_data_end) - (*application_data_variable)) {
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
            APP_ID(18, 2, 0);
            break;
        }
        case 1: {
            APP_ID(14, 2, 0);
            break;
        }
        case 2: {
            APP_ID(14, 2, 0);
            break;
        }
        case 3: {
            APP_ID(14, 2, 0);
            break;
        }
        case 4: {
            APP_ID(16, 2, 0);
            break;
        }
        case 10: {
            APP_ID(6, 2, 20);
            break;
        }
        case 11: {
            APP_ID(6, 2, 0);
            break;
        }
        case 12: {
            APP_ID(6, 2, 0);
            break;
        }
        case 13: {
            APP_ID(6, 2, 0);
            break;
        }
        case 15: {
            APP_ID(6, 2, 0);
            break;
        }
        case 16: {
            APP_ID(6, 2, 0);
            break;
        }
        case 17: {
            APP_ID(6, 2, 0);
            break;
        }
        case 18: {
            APP_ID(6, 2, 0);
            break;
        }
        case 19: {
            APP_ID(6, 2, 0);
            break;
        }
        case 21: {
            APP_ID(20, 2, 20);
            break;
        }
        case 22: {
            APP_ID(20, 2, 20);
            break;
        }
        case 30: {
            APP_ID(8, 3, 0);
            break;
        }
        case 31: {
            APP_ID(8, 3, 0);
            break;
        }
        case 32: {
            APP_ID(8, 3, 0);
            break;
        }
        case 33: {
            APP_ID(8, 3, 0);
            break;
        }
        case 34: {
            APP_ID(8, 3, 0);
            break;
        }
        case 35: {
            APP_ID(8, 3, 0);
            break;
        }
        case 36: {
            APP_ID(8, 3, 0);
            break;
        }
        case 37: {
            APP_ID(8, 2, 8);
            break;
        }
        case 41: {
            APP_ID(8, 3, 0);
            break;
        }
        case 90: {
            APP_ID(30, 2, 30);
            break;
        }
        case 91: {
            APP_ID(90, 2, 90);
            break;
        }
        case 92: {
            APP_ID(90, 2, 90);
            break;
        }
        case 93: {
            APP_ID(90, 2, 90);
            break;
        }
        case 94: {
            APP_ID(90, 2, 90);
            break;
        }
        case 95: {
            APP_ID(90, 2, 90);
            break;
        }
        case 96: {
            APP_ID(90, 2, 90);
            break;
        }
        case 97: {
            APP_ID(90, 2, 90);
            break;
        }
        case 98: {
            APP_ID(90, 2, 90);
            break;
        }
        case 99: {
            APP_ID(90, 2, 90);
            break;
        }
        case 235: {
            APP_ID(28, 3, 28);
            break;
        }
        case 240: {
            APP_ID(30, 3, 30);
            break;
        }
        case 241: {
            APP_ID(30, 3, 30);
            break;
        }
        case 242: {
            APP_ID(6, 3, 6);
            break;
        }
        case 243: {
            APP_ID(20, 3, 20);
            break;
        }
        case 250: {
            APP_ID(30, 3, 30);
            break;
        }
        case 251: {
            APP_ID(30, 3, 30);
            break;
        }
        case 253: {
            APP_ID(13 + 17, 3, 17);
            break;
        }
        case 254: {
            APP_ID(20, 3, 20);
            break;
        }
        case 255: {
            APP_ID(13 + 12, 3, 12);
            break;
        }
        case 310: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3100: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3101: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3102: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3103: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3104: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3105: {
            APP_ID(6, 4, 0);
            break;
        }
        case 311: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3110: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3111: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3112: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3113: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3114: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3115: {
            APP_ID(6, 4, 0);
            break;
        }
        case 312: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3120: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3121: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3122: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3123: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3124: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3125: {
            APP_ID(6, 4, 0);
            break;
        }
        case 313: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3130: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3131: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3132: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3133: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3134: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3135: {
            APP_ID(6, 4, 0);
            break;
        }
        case 314: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3140: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3141: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3142: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3143: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3144: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3145: {
            APP_ID(6, 4, 0);
            break;
        }
        case 315: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3150: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3151: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3152: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3153: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3154: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3155: {
            APP_ID(6, 4, 0);
            break;
        }
        case 316: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3160: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3161: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3162: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3163: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3164: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3165: {
            APP_ID(6, 4, 0);
            break;
        }
        case 320: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3200: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3201: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3202: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3203: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3204: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3205: {
            APP_ID(6, 4, 0);
            break;
        }
        case 321: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3210: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3211: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3212: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3213: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3214: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3215: {
            APP_ID(6, 4, 0);
            break;
        }
        case 322: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3220: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3221: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3222: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3223: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3224: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3225: {
            APP_ID(6, 4, 0);
            break;
        }
        case 323: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3230: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3231: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3232: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3233: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3234: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3235: {
            APP_ID(6, 4, 0);
            break;
        }
        case 324: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3240: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3241: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3242: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3243: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3244: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3245: {
            APP_ID(6, 4, 0);
            break;
        }
        case 325: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3250: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3251: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3252: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3253: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3254: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3255: {
            APP_ID(6, 4, 0);
            break;
        }
        case 326: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3260: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3261: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3262: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3263: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3264: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3265: {
            APP_ID(6, 4, 0);
            break;
        }
        case 327: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3270: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3271: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3272: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3273: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3274: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3275: {
            APP_ID(6, 4, 0);
            break;
        }
        case 328: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3280: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3281: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3282: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3283: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3284: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3285: {
            APP_ID(6, 4, 0);
            break;
        }
        case 329: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3290: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3291: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3292: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3293: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3294: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3295: {
            APP_ID(6, 4, 0);
            break;
        }
        case 330: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3300: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3301: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3302: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3303: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3304: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3305: {
            APP_ID(6, 4, 0);
            break;
        }
        case 331: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3310: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3311: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3312: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3313: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3314: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3315: {
            APP_ID(6, 4, 0);
            break;
        }
        case 332: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3320: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3321: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3322: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3323: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3324: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3325: {
            APP_ID(6, 4, 0);
            break;
        }
        case 333: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3330: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3331: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3332: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3333: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3334: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3335: {
            APP_ID(6, 4, 0);
            break;
        }
        case 334: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3340: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3341: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3342: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3343: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3344: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3345: {
            APP_ID(6, 4, 0);
            break;
        }
        case 335: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3350: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3351: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3352: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3353: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3354: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3355: {
            APP_ID(6, 4, 0);
            break;
        }
        case 336: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3360: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3361: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3362: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3363: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3364: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3365: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3370: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3371: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3372: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3373: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3374: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3375: {
            APP_ID(6, 4, 0);
            break;
        }
        case 340: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3400: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3401: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3402: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3403: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3404: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3405: {
            APP_ID(6, 4, 0);
            break;
        }
        case 341: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3410: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3411: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3412: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3413: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3414: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3415: {
            APP_ID(6, 4, 0);
            break;
        }
        case 342: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3420: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3421: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3422: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3423: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3424: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3425: {
            APP_ID(6, 4, 0);
            break;
        }
        case 343: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3430: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3431: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3432: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3433: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3434: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3435: {
            APP_ID(6, 4, 0);
            break;
        }
        case 344: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3440: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3441: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3442: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3443: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3444: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3445: {
            APP_ID(6, 4, 0);
            break;
        }
        case 345: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3450: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3451: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3452: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3453: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3454: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3455: {
            APP_ID(6, 4, 0);
            break;
        }
        case 346: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3460: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3461: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3462: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3463: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3464: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3465: {
            APP_ID(6, 4, 0);
            break;
        }
        case 347: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3470: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3471: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3472: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3473: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3474: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3475: {
            APP_ID(6, 4, 0);
            break;
        }
        case 348: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3480: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3481: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3482: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3483: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3484: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3485: {
            APP_ID(6, 4, 0);
            break;
        }
        case 349: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3490: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3491: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3492: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3493: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3494: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3495: {
            APP_ID(6, 4, 0);
            break;
        }
        case 350: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3500: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3501: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3502: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3503: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3504: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3505: {
            APP_ID(6, 4, 0);
            break;
        }
        case 351: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3510: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3511: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3512: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3513: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3514: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3515: {
            APP_ID(6, 4, 0);
            break;
        }
        case 352: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3520: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3521: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3522: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3523: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3524: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3525: {
            APP_ID(6, 4, 0);
            break;
        }
        case 353: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3530: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3531: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3532: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3533: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3534: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3535: {
            APP_ID(6, 4, 0);
            break;
        }
        case 354: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3540: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3541: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3542: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3543: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3544: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3545: {
            APP_ID(6, 4, 0);
            break;
        }
        case 355: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3550: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3551: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3552: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3553: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3554: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3555: {
            APP_ID(6, 4, 0);
            break;
        }
        case 356: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3560: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3561: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3562: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3563: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3564: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3565: {
            APP_ID(6, 4, 0);
            break;
        }
        case 357: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3570: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3571: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3572: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3573: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3574: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3575: {
            APP_ID(6, 4, 0);
            break;
        }
        case 360: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3600: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3601: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3602: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3603: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3604: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3605: {
            APP_ID(6, 4, 0);
            break;
        }
        case 361: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3610: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3611: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3612: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3613: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3614: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3615: {
            APP_ID(6, 4, 0);
            break;
        }
        case 362: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3620: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3621: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3622: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3623: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3624: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3625: {
            APP_ID(6, 4, 0);
            break;
        }
        case 363: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3630: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3631: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3632: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3633: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3634: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3635: {
            APP_ID(6, 4, 0);
            break;
        }
        case 364: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3640: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3641: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3642: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3643: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3644: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3645: {
            APP_ID(6, 4, 0);
            break;
        }
        case 365: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3650: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3651: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3652: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3653: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3654: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3655: {
            APP_ID(6, 4, 0);
            break;
        }
        case 366: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3660: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3661: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3662: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3663: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3664: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3665: {
            APP_ID(6, 4, 0);
            break;
        }
        case 367: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3670: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3671: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3672: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3673: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3674: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3675: {
            APP_ID(6, 4, 0);
            break;
        }
        case 368: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3680: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3681: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3682: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3683: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3684: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3685: {
            APP_ID(6, 4, 0);
            break;
        }
        case 369: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3690: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3691: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3692: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3693: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3694: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3695: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3900: {
            APP_ID(15, 4, 15);
            break;
        }
        case 3901: {
            APP_ID(15, 4, 15);
            break;
        }
        case 3902: {
            APP_ID(15, 4, 15);
            break;
        }
        case 3910: {
            APP_ID(3 + 15, 4, 15);
            break;
        }
        case 3911: {
            APP_ID(3 + 15, 4, 15);
            break;
        }
        case 3912: {
            APP_ID(3 + 15, 4, 15);
            break;
        }
        case 3920: {
            APP_ID(15, 4, 15);
            break;
        }
        case 3921: {
            APP_ID(15, 4, 15);
            break;
        }
        case 3922: {
            APP_ID(15, 4, 15);
            break;
        }
        case 3930: {
            APP_ID(3 + 15, 4, 15);
            break;
        }
        case 3931: {
            APP_ID(3 + 15, 4, 15);
            break;
        }
        case 3932: {
            APP_ID(3 + 15, 4, 15);
            break;
        }
        case 394: {
            APP_ID(4, 4, 0);
            break;
        }
        case 3940: {
            APP_ID(4, 4, 0);
            break;
        }
        case 3941: {
            APP_ID(4, 4, 0);
            break;
        }
        case 3942: {
            APP_ID(4, 4, 0);
            break;
        }
        case 3943: {
            APP_ID(4, 4, 0);
            break;
        }
        case 3950: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3951: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3952: {
            APP_ID(6, 4, 0);
            break;
        }
        case 3953: {
            APP_ID(6, 4, 0);
            break;
        }
        case 400: {
            APP_ID(30, 3, 30);
            break;
        }
        case 401: {
            APP_ID(30, 3, 30);
            break;
        }
        case 402: {
            APP_ID(17, 3, 0);
            break;
        }
        case 403: {
            APP_ID(30, 3, 30);
            break;
        }
        case 410: {
            APP_ID(13, 3, 0);
            break;
        }
        case 411: {
            APP_ID(13, 3, 0);
            break;
        }
        case 412: {
            APP_ID(13, 3, 0);
            break;
        }
        case 413: {
            APP_ID(13, 3, 0);
            break;
        }
        case 414: {
            APP_ID(13, 3, 0);
            break;
        }
        case 415: {
            APP_ID(13, 3, 0);
            break;
        }
        case 416: {
            APP_ID(13, 3, 0);
            break;
        }
        case 417: {
            APP_ID(13, 3, 0);
            break;
        }
        case 420: {
            APP_ID(20, 3, 20);
            break;
        }
        case 421: {
            APP_ID(3 + 9, 3, 9);
            break;
        }
        case 422: {
            APP_ID(3, 3, 0);
            break;
        }
        case 423: {
            APP_ID(3 + 12, 3, 12);
            break;
        }
        case 424: {
            APP_ID(3, 3, 0);
            break;
        }
        case 425: {
            APP_ID(3 + 12, 3, 12);
            break;
        }
        case 426: {
            APP_ID(3, 3, 0);
            break;
        }
        case 703: {
            APP_ID(3 + 27, 4, 27);
            break;
        }
        case 710: {
            APP_ID(20, 3, 20);
            break;
        }
        case 711: {
            APP_ID(20, 3, 20);
            break;
        }
        case 712: {
            APP_ID(20, 3, 20);
            break;
        }
        case 713: {
            APP_ID(20, 3, 20);
            break;
        }
        case 714: {
            APP_ID(20, 3, 20);
            break;
        }
        case 715: {
            APP_ID(20, 3, 20);
            break;
        }
        case 716: {
            APP_ID(20, 3, 20);
            break;
        }
        case 4300: {
            APP_ID(35, 4, 35);
            break;
        }
        case 4301: {
            APP_ID(35, 4, 35);
            break;
        }
        case 4302: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4303: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4304: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4305: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4306: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4307: {
            APP_ID(2, 4, 0);
            break;
        }
        case 4308: {
            APP_ID(30, 4, 30);
            break;
        }
        case 4309: {
            APP_ID(20, 4, 20);
            break;
        }
        case 4310: {
            APP_ID(35, 4, 35);
            break;
        }
        case 4311: {
            APP_ID(35, 4, 35);
            break;
        }
        case 4312: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4313: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4314: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4315: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4316: {
            APP_ID(70, 4, 70);
            break;
        }
        case 4317: {
            APP_ID(2, 4, 0);
            break;
        }
        case 4318: {
            APP_ID(20, 4, 20);
            break;
        }
        case 4319: {
            APP_ID(30, 4, 30);
            break;
        }
        case 4320: {
            APP_ID(35, 4, 35);
            break;
        }
        case 4321: {
            APP_ID(1, 4, 0);
            break;
        }
        case 4322: {
            APP_ID(1, 4, 0);
            break;
        }
        case 4323: {
            APP_ID(1, 4, 0);
            break;
        }
        case 4324: {
            APP_ID(10, 4, 0);
            break;
        }
        case 4325: {
            APP_ID(10, 4, 0);
            break;
        }
        case 4326: {
            APP_ID(6, 4, 0);
            break;
        }
        case 4330: {
            APP_ID(6 + 1, 4, 1);
            break;
        }
        case 4331: {
            APP_ID(6 + 1, 4, 1);
            break;
        }
        case 4332: {
            APP_ID(6 + 1, 4, 1);
            break;
        }
        case 4333: {
            APP_ID(6 + 1, 4, 1);
            break;
        }
        case 7001: {
            APP_ID(13, 4, 0);
            break;
        }
        case 7002: {
            APP_ID(30, 4, 30);
            break;
        }
        case 7003: {
            APP_ID(10, 4, 0);
            break;
        }
        case 7004: {
            APP_ID(4, 4, 4);
            break;
        }
        case 7005: {
            APP_ID(12, 4, 12);
            break;
        }
        case 7006: {
            APP_ID(6, 4, 0);
            break;
        }
        case 7007: {
            APP_ID(6 + 6, 4, 6);
            break;
        }
        case 7008: {
            APP_ID(3, 4, 3);
            break;
        }
        case 7009: {
            APP_ID(10, 4, 10);
            break;
        }
        case 7010: {
            APP_ID(2, 4, 2);
            break;
        }
        case 7011: {
            APP_ID(6 + 4, 4, 4);
            break;
        }
        case 7020: {
            APP_ID(20, 4, 20);
            break;
        }
        case 7021: {
            APP_ID(20, 4, 20);
            break;
        }
        case 7022: {
            APP_ID(20, 4, 20);
            break;
        }
        case 7023: {
            APP_ID(30, 4, 30);
            break;
        }
        case 7030: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7031: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7032: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7033: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7034: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7035: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7036: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7037: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7038: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7039: {
            APP_ID(4 + 30, 4, 30);
            break;
        }
        case 7040: {
            APP_ID(1 + 3, 4, 0);
            break;
        }
        case 7041: {
            APP_ID(4, 4, 4);
            break;
        }
        case 7240: {
            APP_ID(20, 4, 20);
            break;
        }
        case 7241: {
            APP_ID(2, 4, 0);
            break;
        }
        case 7242: {
            APP_ID(25, 4, 25);
            break;
        }
        case 7250: {
            APP_ID(8, 4, 0);
            break;
        }
        case 7251: {
            APP_ID(12, 4, 0);
            break;
        }
        case 7252: {
            APP_ID(1, 4, 0);
            break;
        }
        case 7253: {
            APP_ID(40, 4, 40);
            break;
        }
        case 7254: {
            APP_ID(40, 4, 40);
            break;
        }
        case 7255: {
            APP_ID(10, 4, 10);
            break;
        }
        case 7256: {
            APP_ID(90, 4, 90);
            break;
        }
        case 7257: {
            APP_ID(70, 4, 70);
            break;
        }
        case 7258: {
            APP_ID(1 + 1 + 1, 4, 0);
            break;
        }
        case 7259: {
            APP_ID(40, 4, 40);
            break;
        }
        case 8001: {
            APP_ID(14, 4, 0);
            break;
        }
        case 8002: {
            APP_ID(20, 4, 20);
            break;
        }
        case 8003: {
            APP_ID(14 + 16, 4, 16);
            break;
        }
        case 8004: {
            APP_ID(30, 4, 30);
            break;
        }
        case 8005: {
            APP_ID(6, 4, 0);
            break;
        }
        case 8006: {
            APP_ID(14 + 2 + 2, 4, 0);
            break;
        }
        case 8007: {
            APP_ID(34, 4, 34);
            break;
        }
        case 8008: {
            APP_ID(8 + 4, 4, 4);
            break;
        }
        case 8010: {
            APP_ID(30, 4, 30);
            break;
        }
        case 8011: {
            APP_ID(12, 4, 12);
            break;
        }
        case 8012: {
            APP_ID(20, 4, 20);
            break;
        }
        case 8013: {
            APP_ID(30, 4, 30);
            break;
        }
        case 8017: {
            APP_ID(18, 4, 0);
            break;
        }
        case 8018: {
            APP_ID(18, 4, 0);
            break;
        }
        case 8019: {
            APP_ID(10, 4, 10);
            break;
        }
        case 8020: {
            APP_ID(25, 4, 25);
            break;
        }
        case 8026: {
            APP_ID(14 + 2 + 2, 4, 0);
            break;
        }
        case 8030: {
            APP_ID(90, 4, 90);
            break;
        }
        case 8110: {
            APP_ID(70, 4, 70);
            break;
        }
        case 8111: {
            APP_ID(4, 4, 0);
            break;
        }
        case 8112: {
            APP_ID(70, 4, 70);
            break;
        }
        case 8200: {
            APP_ID(70, 4, 70);
            break;
        }
        case 977: {
            APP_ID(10, 3, 0);
            break;
        }
        case 978: {
            APP_ID(10, 3, 0);
            break;
        }
        case 979: {
            APP_ID(10, 3, 0);
            break;
        }
        }
    }
    else {
        /* read data associated with the application identifier */
        *application_data_end = curr_pos + (*application_identifier_length);
        data_str = &result[*application_data_start];
        date_str = NULL;
        end_date_str = NULL;
        curr_str = NULL;
        decimal_str = NULL;
        country_str = NULL;
        processor_country_str = NULL;
        coupon_str = NULL;
        issn_str = NULL;
        company_prefix_str = NULL;
        package_type_str = NULL;
        gtin_check_digit_passed = -1;
        gsin_check_digit_passed = -1;
        gsrn_check_digit_passed = -1;
        sscc_check_digit_passed = -1;
        sscc_package_type = ' ';
        grai_check_digit_passed = -1;
        gln_check_digit_passed = -1;
        roll_width_mm = -1;
        roll_diameter_mm = -1;
        roll_length_metres = -1;
        roll_no_of_splices = -1;
        roll_winding_direction = -1;
        latitude = UNKNOWN_VALUE;
        longitude = UNKNOWN_VALUE;
        authority_to_leave = -1;
        signature_required = -1;
        aidc_media_type = -1;
        temperature = UNKNOWN_VALUE;
        birth_sequence[0] = 0;
        itip_piece_number_str[0] = 0;
        itip_total_count_str[0] = 0;
        bio_sex = -1;

        if (strlen(data_str) > 0) {
            /* see https://www.gs1.org/docs/barcodes/GSCN-25-081-UN-ECE-Recommendation20.pdf */

            app_id_str2[0] = 0;
            if ((*application_identifier >= 0) &&
                    (*application_identifier < 99999)) {
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
                    /* see https://documents.gs1us.org/adobe/assets/deliver/urn:aaid:aem:494e625b-e1d8-4bbd-a1be-5918879cfc3d/An-Introduction-to-the-Serial-Shipping-Container-Code-SSCC.pdf */
                    decode_strcat(gs1_result, "SSCC: ");
                    if ((int)strlen(data_str) > 5) {
                        /* first digit is extension digit */
                        int sscc_start_index = 1;
                        sscc_package_type = data_str[0];
                        if (data_str[1] == '0') {
                            sscc_start_index = 2;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   sscc_start_index, data_str);
                        int check_digit =
                            get_gtin_check_digit(&data_str[sscc_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                sscc_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    sscc_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 1: {
                if (debug == 1) printf("GTIN ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GTIN");
                    if ((int)strlen(data_str) > 4) {
                        int gtin_start_index = 0;
                        if (data_str[0] == '0') {
                            gtin_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gtin_start_index, data_str);
                        /* get the length of the GTIN */
                        int gtin_length = (int)strlen(&data_str[gtin_start_index]);
                        if ((gtin_length == 8) || (gtin_length == 12) ||
                                (gtin_length == 13) || (gtin_length == 14)) {
                            char gtin_type_str[4];
                            sprintf(&gtin_type_str[0], "-%d", gtin_length);
                            decode_strcat(gs1_result, &gtin_type_str[0]);
                        }

                        int check_digit =
                            get_gtin_check_digit(&data_str[gtin_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gtin_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gtin_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                    decode_strcat(gs1_result, ": ");
                }
                break;
            }
            case 2: {
                DECODE("CONTENT");
                break;
            }
            case 3: {
                if (debug == 1) printf("MTO GTIN ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "MTO GTIN");
                    if ((int)strlen(data_str) > 4) {
                        int gtin_start_index = 0;
                        if (data_str[0] == '0') {
                            gtin_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gtin_start_index, data_str);
                        /* get the length of the MTO GTIN */
                        int gtin_length = (int)strlen(&data_str[gtin_start_index]);
                        if ((gtin_length == 8) || (gtin_length == 12) ||
                                (gtin_length == 13) || (gtin_length == 14)) {
                            char gtin_type_str[4];
                            sprintf(&gtin_type_str[0], "-%d", gtin_length);
                            decode_strcat(gs1_result, &gtin_type_str[0]);
                        }

                        int check_digit =
                            get_gtin_check_digit(&data_str[gtin_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gtin_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gtin_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                    decode_strcat(gs1_result, ": ");
                }
                break;
            }
            case 4: {
                DECODE("ID");
                break;
            }
            case 10: {
                DECODE("BATCH/LOT");
                break;
            }
            case 11: {
                DECODE_DATE("PROD DATE", "YYMMDD");
                break;
            }
            case 12: {
                DECODE_DATE("DUE DATE", "YYMMDD");
                break;
            }
            case 13: {
                DECODE_DATE("PACK DATE", "YYMMDD");
                break;
            }
            case 15: {
                DECODE_DATE("BEST BEFORE", "YYMMDD");
                break;
            }
            case 16: {
                DECODE_DATE("SELL BY", "YYMMDD");
                break;
            }
            case 17: {
                DECODE_DATE("EXPIRY", "YYMMDD");
                break;
            }
            case 18: {
                DECODE("ID");
                break;
            }
            case 19: {
                DECODE("ID");
                break;
            }
            case 21: {
                DECODE("SERIAL");
                break;
            }
            case 22: {
                DECODE("CPV");
                break;
            }
            case 30: {
                DECODE("VAR COUNT");
                break;
            }
            case 31: {
                DECODE("ID");
                break;
            }
            case 32: {
                DECODE("ID");
                break;
            }
            case 33: {
                DECODE("ID");
                break;
            }
            case 34: {
                DECODE("ID");
                break;
            }
            case 35: {
                DECODE("ID");
                break;
            }
            case 36: {
                DECODE("ID");
                break;
            }
            case 37: {
                DECODE("COUNT");
                break;
            }
            case 41: {
                DECODE("ID");
                break;
            }
            case 90: {
                DECODE("INTERNAL");
                break;
            }
            case 91: {
                DECODE("INTERNAL");
                break;
            }
            case 92: {
                DECODE("INTERNAL");
                break;
            }
            case 93: {
                DECODE("INTERNAL");
                break;
            }
            case 94: {
                DECODE("INTERNAL");
                break;
            }
            case 95: {
                DECODE("INTERNAL");
                break;
            }
            case 96: {
                DECODE("INTERNAL");
                break;
            }
            case 97: {
                DECODE("INTERNAL");
                break;
            }
            case 98: {
                DECODE("INTERNAL");
                break;
            }
            case 99: {
                DECODE("INTERNAL");
                break;
            }
            case 235: {
                DECODE("TPX");
                break;
            }
            case 240: {
                DECODE("ADDITIONAL ID");
                break;
            }
            case 241: {
                DECODE("CUST PART No");
                break;
            }
            case 242: {
                DECODE("MTO VARIANT");
                break;
            }
            case 243: {
                DECODE("PCN");
                break;
            }
            case 250: {
                DECODE("SECONDARY SERIAL");
                break;
            }
            case 251: {
                DECODE("REF TO SOURCE");
                break;
            }
            case 253: {
                if (debug == 1) printf("GDTI ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GDTI: ");
                    if ((int)strlen(data_str) > 4) {
                        int gdti_start_index = 0;
                        if (data_str[0] == '0') {
                            gdti_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gdti_start_index, data_str);
                    }
                }
                break;
            }
            case 254: {
                DECODE("GLN EXTENSION COMPONENT");
                break;
            }
            case 255: {
                if (debug == 1) printf("GCN ");
                if (is_digital_link == 0) {
                    coupon_str = get_coupon(data_str);
                    if (coupon_str == NULL) {
                        decode_strcat(gs1_result, "GCN: ");
                    }
                    if ((int)strlen(data_str) > 4) {
                        int gcn_start_index = 0;
                        if (data_str[0] == '0') {
                            gcn_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gcn_start_index, data_str);
                    }
                }
                break;
            }
            case 310: {
                DECODE("NET WEIGHT (kg)");
                break;
            }
            case 3100: {
                DECODE("NET WEIGHT (kg)");
                break;
            }
            case 3101: {
                DECODE_DECIMAL("NET WEIGHT (kg)");
                break;
            }
            case 3102: {
                DECODE_DECIMAL("NET WEIGHT (kg)");
                break;
            }
            case 3103: {
                DECODE_DECIMAL("NET WEIGHT (kg)");
                break;
            }
            case 3104: {
                DECODE_DECIMAL("NET WEIGHT (kg)");
                break;
            }
            case 3105: {
                DECODE_DECIMAL("NET WEIGHT (kg)");
                break;
            }
            case 311: {
                DECODE("LENGTH (m)");
                break;
            }
            case 3110: {
                DECODE("LENGTH (m)");
                break;
            }
            case 3111: {
                DECODE_DECIMAL("LENGTH (m)");
                break;
            }
            case 3112: {
                DECODE_DECIMAL("LENGTH (m)");
                break;
            }
            case 3113: {
                DECODE_DECIMAL("LENGTH (m)");
                break;
            }
            case 3114: {
                DECODE_DECIMAL("LENGTH (m)");
                break;
            }
            case 3115: {
                DECODE_DECIMAL("LENGTH (m)");
                break;
            }
            case 312: {
                DECODE("WIDTH (m)");
                break;
            }
            case 3120: {
                DECODE("WIDTH (m)");
                break;
            }
            case 3121: {
                DECODE_DECIMAL("WIDTH (m)");
                break;
            }
            case 3122: {
                DECODE_DECIMAL("WIDTH (m)");
                break;
            }
            case 3123: {
                DECODE_DECIMAL("WIDTH (m)");
                break;
            }
            case 3124: {
                DECODE_DECIMAL("WIDTH (m)");
                break;
            }
            case 3125: {
                DECODE_DECIMAL("WIDTH (m)");
                break;
            }
            case 313: {
                DECODE("HEIGHT (m)");
                break;
            }
            case 3130: {
                DECODE("HEIGHT (m)");
                break;
            }
            case 3131: {
                DECODE_DECIMAL("HEIGHT (m)");
                break;
            }
            case 3132: {
                DECODE_DECIMAL("HEIGHT (m)");
                break;
            }
            case 3133: {
                DECODE_DECIMAL("HEIGHT (m)");
                break;
            }
            case 3134: {
                DECODE_DECIMAL("HEIGHT (m)");
                break;
            }
            case 3135: {
                DECODE_DECIMAL("HEIGHT (m)");
                break;
            }
            case 314: {
                DECODE("AREA (m2)");
                break;
            }
            case 3140: {
                DECODE("AREA (m2)");
                break;
            }
            case 3141: {
                DECODE_DECIMAL("AREA (m2)");
                break;
            }
            case 3142: {
                DECODE_DECIMAL("AREA (m2)");
                break;
            }
            case 3143: {
                DECODE_DECIMAL("AREA (m2)");
                break;
            }
            case 3144: {
                DECODE_DECIMAL("AREA (m2)");
                break;
            }
            case 3145: {
                DECODE_DECIMAL("AREA (m2)");
                break;
            }
            case 315: {
                DECODE("NET VOLUME (l)");
                break;
            }
            case 3150: {
                DECODE("NET VOLUME (l)");
                break;
            }
            case 3151: {
                DECODE_DECIMAL("NET VOLUME (l)");
                break;
            }
            case 3152: {
                DECODE_DECIMAL("NET VOLUME (l)");
                break;
            }
            case 3153: {
                DECODE_DECIMAL("NET VOLUME (l)");
                break;
            }
            case 3154: {
                DECODE_DECIMAL("NET VOLUME (l)");
                break;
            }
            case 3155: {
                DECODE_DECIMAL("NET VOLUME (l)");
                break;
            }
            case 316: {
                DECODE("NET VOLUME (m3)");
                break;
            }
            case 3160: {
                DECODE("NET VOLUME (m3)");
                break;
            }
            case 3161: {
                DECODE_DECIMAL("NET VOLUME (m3)");
                break;
            }
            case 3162: {
                DECODE_DECIMAL("NET VOLUME (m3)");
                break;
            }
            case 3163: {
                DECODE_DECIMAL("NET VOLUME (m3)");
                break;
            }
            case 3164: {
                DECODE_DECIMAL("NET VOLUME (m3)");
                break;
            }
            case 3165: {
                DECODE_DECIMAL("NET VOLUME (m3)");
                break;
            }
            case 320: {
                DECODE("NET WEIGHT (lb)");
                break;
            }
            case 3200: {
                DECODE("NET WEIGHT (lb)");
                break;
            }
            case 3201: {
                DECODE_DECIMAL("NET WEIGHT (lb)");
                break;
            }
            case 3202: {
                DECODE_DECIMAL("NET WEIGHT (lb)");
                break;
            }
            case 3203: {
                DECODE_DECIMAL("NET WEIGHT (lb)");
                break;
            }
            case 3204: {
                DECODE_DECIMAL("NET WEIGHT (lb)");
                break;
            }
            case 3205: {
                DECODE_DECIMAL("NET WEIGHT (lb)");
                break;
            }
            case 321: {
                DECODE("LENGTH (in)");
                break;
            }
            case 3210: {
                DECODE("LENGTH (in)");
                break;
            }
            case 3211: {
                DECODE_DECIMAL("LENGTH (in)");
                break;
            }
            case 3212: {
                DECODE_DECIMAL("LENGTH (in)");
                break;
            }
            case 3213: {
                DECODE_DECIMAL("LENGTH (in)");
                break;
            }
            case 3214: {
                DECODE_DECIMAL("LENGTH (in)");
                break;
            }
            case 3215: {
                DECODE_DECIMAL("LENGTH (in)");
                break;
            }
            case 322: {
                DECODE("LENGTH (ft)");
                break;
            }
            case 3220: {
                DECODE("LENGTH (ft)");
                break;
            }
            case 3221: {
                DECODE_DECIMAL("LENGTH (ft)");
                break;
            }
            case 3222: {
                DECODE_DECIMAL("LENGTH (ft)");
                break;
            }
            case 3223: {
                DECODE_DECIMAL("LENGTH (ft)");
                break;
            }
            case 3224: {
                DECODE_DECIMAL("LENGTH (ft)");
                break;
            }
            case 3225: {
                DECODE_DECIMAL("LENGTH (ft)");
                break;
            }
            case 323: {
                DECODE("LENGTH (yd)");
                break;
            }
            case 3230: {
                DECODE("LENGTH (yd)");
                break;
            }
            case 3231: {
                DECODE_DECIMAL("LENGTH (yd)");
                break;
            }
            case 3232: {
                DECODE_DECIMAL("LENGTH (yd)");
                break;
            }
            case 3233: {
                DECODE_DECIMAL("LENGTH (yd)");
                break;
            }
            case 3234: {
                DECODE_DECIMAL("LENGTH (yd)");
                break;
            }
            case 3235: {
                DECODE_DECIMAL("LENGTH (yd)");
                break;
            }
            case 324: {
                DECODE("WIDTH (in)");
                break;
            }
            case 3240: {
                DECODE("WIDTH (in)");
                break;
            }
            case 3241: {
                DECODE_DECIMAL("WIDTH (in)");
                break;
            }
            case 3242: {
                DECODE_DECIMAL("WIDTH (in)");
                break;
            }
            case 3243: {
                DECODE_DECIMAL("WIDTH (in)");
                break;
            }
            case 3244: {
                DECODE_DECIMAL("WIDTH (in)");
                break;
            }
            case 3245: {
                DECODE_DECIMAL("WIDTH (in)");
                break;
            }
            case 325: {
                DECODE("WIDTH (ft)");
                break;
            }
            case 3250: {
                DECODE("WIDTH (ft)");
                break;
            }
            case 3251: {
                DECODE_DECIMAL("WIDTH (ft)");
                break;
            }
            case 3252: {
                DECODE_DECIMAL("WIDTH (ft)");
                break;
            }
            case 3253: {
                DECODE_DECIMAL("WIDTH (ft)");
                break;
            }
            case 3254: {
                DECODE_DECIMAL("WIDTH (ft)");
                break;
            }
            case 3255: {
                DECODE_DECIMAL("WIDTH (ft)");
                break;
            }
            case 326: {
                DECODE("WIDTH (yd)");
                break;
            }
            case 3260: {
                DECODE("WIDTH (yd)");
                break;
            }
            case 3261: {
                DECODE_DECIMAL("WIDTH (yd)");
                break;
            }
            case 3262: {
                DECODE_DECIMAL("WIDTH (yd)");
                break;
            }
            case 3263: {
                DECODE_DECIMAL("WIDTH (yd)");
                break;
            }
            case 3264: {
                DECODE_DECIMAL("WIDTH (yd)");
                break;
            }
            case 3265: {
                DECODE_DECIMAL("WIDTH (yd)");
                break;
            }
            case 327: {
                DECODE("HEIGHT (in)");
                break;
            }
            case 3270: {
                DECODE("HEIGHT (in)");
                break;
            }
            case 3271: {
                DECODE_DECIMAL("HEIGHT (in)");
                break;
            }
            case 3272: {
                DECODE_DECIMAL("HEIGHT (in)");
                break;
            }
            case 3273: {
                DECODE_DECIMAL("HEIGHT (in)");
                break;
            }
            case 3274: {
                DECODE_DECIMAL("HEIGHT (in)");
                break;
            }
            case 3275: {
                DECODE_DECIMAL("HEIGHT (in)");
                break;
            }
            case 328: {
                DECODE("HEIGHT (ft)");
                break;
            }
            case 3280: {
                DECODE("HEIGHT (ft)");
                break;
            }
            case 3281: {
                DECODE_DECIMAL("HEIGHT (ft)");
                break;
            }
            case 3282: {
                DECODE_DECIMAL("HEIGHT (ft)");
                break;
            }
            case 3283: {
                DECODE_DECIMAL("HEIGHT (ft)");
                break;
            }
            case 3284: {
                DECODE_DECIMAL("HEIGHT (ft)");
                break;
            }
            case 3285: {
                DECODE_DECIMAL("HEIGHT (ft)");
                break;
            }
            case 329: {
                DECODE("HEIGHT (yd)");
                break;
            }
            case 3290: {
                DECODE("HEIGHT (yd)");
                break;
            }
            case 3291: {
                DECODE_DECIMAL("HEIGHT (yd)");
                break;
            }
            case 3292: {
                DECODE_DECIMAL("HEIGHT (yd)");
                break;
            }
            case 3293: {
                DECODE_DECIMAL("HEIGHT (yd)");
                break;
            }
            case 3294: {
                DECODE_DECIMAL("HEIGHT (yd)");
                break;
            }
            case 3295: {
                DECODE_DECIMAL("HEIGHT (yd)");
                break;
            }
            case 330: {
                DECODE("GROSS WEIGHT (kg)");
                break;
            }
            case 3300: {
                DECODE("GROSS WEIGHT (kg)");
                break;
            }
            case 3301: {
                DECODE_DECIMAL("GROSS WEIGHT (kg)");
                break;
            }
            case 3302: {
                DECODE_DECIMAL("GROSS WEIGHT (kg)");
                break;
            }
            case 3303: {
                DECODE_DECIMAL("GROSS WEIGHT (kg)");
                break;
            }
            case 3304: {
                DECODE_DECIMAL("GROSS WEIGHT (kg)");
                break;
            }
            case 3305: {
                DECODE_DECIMAL("GROSS WEIGHT (kg)");
                break;
            }
            case 331: {
                DECODE("LENGTH (m), log");
                break;
            }
            case 3310: {
                DECODE("LENGTH (m), log");
                break;
            }
            case 3311: {
                DECODE_DECIMAL("LENGTH (m), log");
                break;
            }
            case 3312: {
                DECODE_DECIMAL("LENGTH (m), log");
                break;
            }
            case 3313: {
                DECODE_DECIMAL("LENGTH (m), log");
                break;
            }
            case 3314: {
                DECODE_DECIMAL("LENGTH (m), log");
                break;
            }
            case 3315: {
                DECODE_DECIMAL("LENGTH (m), log");
                break;
            }
            case 332: {
                DECODE("WIDTH (m), log");
                break;
            }
            case 3320: {
                DECODE("WIDTH (m), log");
                break;
            }
            case 3321: {
                DECODE_DECIMAL("WIDTH (m), log");
                break;
            }
            case 3322: {
                DECODE_DECIMAL("WIDTH (m), log");
                break;
            }
            case 3323: {
                DECODE_DECIMAL("WIDTH (m), log");
                break;
            }
            case 3324: {
                DECODE_DECIMAL("WIDTH (m), log");
                break;
            }
            case 3325: {
                DECODE_DECIMAL("WIDTH (m), log");
                break;
            }
            case 333: {
                DECODE("HEIGHT (m), log");
                break;
            }
            case 3330: {
                DECODE("HEIGHT (m), log");
                break;
            }
            case 3331: {
                DECODE_DECIMAL("HEIGHT (m), log");
                break;
            }
            case 3332: {
                DECODE_DECIMAL("HEIGHT (m), log");
                break;
            }
            case 3333: {
                DECODE_DECIMAL("HEIGHT (m), log");
                break;
            }
            case 3334: {
                DECODE_DECIMAL("HEIGHT (m), log");
                break;
            }
            case 3335: {
                DECODE_DECIMAL("HEIGHT (m), log");
                break;
            }
            case 334: {
                DECODE("AREA (m2), log");
                break;
            }
            case 3340: {
                DECODE("AREA (m2), log");
                break;
            }
            case 3341: {
                DECODE_DECIMAL("AREA (m2), log");
                break;
            }
            case 3342: {
                DECODE_DECIMAL("AREA (m2), log");
                break;
            }
            case 3343: {
                DECODE_DECIMAL("AREA (m2), log");
                break;
            }
            case 3344: {
                DECODE_DECIMAL("AREA (m2), log");
                break;
            }
            case 3345: {
                DECODE_DECIMAL("AREA (m2), log");
                break;
            }
            case 335: {
                DECODE("VOLUME (l), log");
                break;
            }
            case 3350: {
                DECODE("VOLUME (l), log");
                break;
            }
            case 3351: {
                DECODE_DECIMAL("VOLUME (l), log");
                break;
            }
            case 3352: {
                DECODE_DECIMAL("VOLUME (l), log");
                break;
            }
            case 3353: {
                DECODE_DECIMAL("VOLUME (l), log");
                break;
            }
            case 3354: {
                DECODE_DECIMAL("VOLUME (l), log");
                break;
            }
            case 3355: {
                DECODE_DECIMAL("VOLUME (l), log");
                break;
            }
            case 336: {
                DECODE("VOLUME (m3), log");
                break;
            }
            case 3360: {
                DECODE("VOLUME (m3), log");
                break;
            }
            case 3361: {
                DECODE_DECIMAL("VOLUME (m3), log");
                break;
            }
            case 3362: {
                DECODE_DECIMAL("VOLUME (m3), log");
                break;
            }
            case 3363: {
                DECODE_DECIMAL("VOLUME (m3), log");
                break;
            }
            case 3364: {
                DECODE_DECIMAL("VOLUME (m3), log");
                break;
            }
            case 3365: {
                DECODE_DECIMAL("VOLUME (m3), log");
                break;
            }
            case 3370: {
                DECODE("KG PER m2");
                break;
            }
            case 3371: {
                DECODE_DECIMAL("KG PER m2");
                break;
            }
            case 3372: {
                DECODE_DECIMAL("KG PER m2");
                break;
            }
            case 3373: {
                DECODE_DECIMAL("KG PER m2");
                break;
            }
            case 3374: {
                DECODE_DECIMAL("KG PER m2");
                break;
            }
            case 3375: {
                DECODE_DECIMAL("KG PER m2");
                break;
            }
            case 340: {
                DECODE("GROSS WEIGHT (lb)");
                break;
            }
            case 3400: {
                DECODE("GROSS WEIGHT (lb)");
                break;
            }
            case 3401: {
                DECODE_DECIMAL("GROSS WEIGHT (lb)");
                break;
            }
            case 3402: {
                DECODE_DECIMAL("GROSS WEIGHT (lb)");
                break;
            }
            case 3403: {
                DECODE_DECIMAL("GROSS WEIGHT (lb)");
                break;
            }
            case 3404: {
                DECODE_DECIMAL("GROSS WEIGHT (lb)");
                break;
            }
            case 3405: {
                DECODE_DECIMAL("GROSS WEIGHT (lb)");
                break;
            }
            case 341: {
                DECODE("LENGTH (in), log");
                break;
            }
            case 3410: {
                DECODE("LENGTH (in), log");
                break;
            }
            case 3411: {
                DECODE_DECIMAL("LENGTH (in), log");
                break;
            }
            case 3412: {
                DECODE_DECIMAL("LENGTH (in), log");
                break;
            }
            case 3413: {
                DECODE_DECIMAL("LENGTH (in), log");
                break;
            }
            case 3414: {
                DECODE_DECIMAL("LENGTH (in), log");
                break;
            }
            case 3415: {
                DECODE_DECIMAL("LENGTH (in), log");
                break;
            }
            case 342: {
                DECODE("LENGTH (ft), log");
                break;
            }
            case 3420: {
                DECODE("LENGTH (ft), log");
                break;
            }
            case 3421: {
                DECODE_DECIMAL("LENGTH (ft), log");
                break;
            }
            case 3422: {
                DECODE_DECIMAL("LENGTH (ft), log");
                break;
            }
            case 3423: {
                DECODE_DECIMAL("LENGTH (ft), log");
                break;
            }
            case 3424: {
                DECODE_DECIMAL("LENGTH (ft), log");
                break;
            }
            case 3425: {
                DECODE_DECIMAL("LENGTH (ft), log");
                break;
            }
            case 343: {
                DECODE("LENGTH (yd), log");
                break;
            }
            case 3430: {
                DECODE("LENGTH (yd), log");
                break;
            }
            case 3431: {
                DECODE_DECIMAL("LENGTH (yd), log");
                break;
            }
            case 3432: {
                DECODE_DECIMAL("LENGTH (yd), log");
                break;
            }
            case 3433: {
                DECODE_DECIMAL("LENGTH (yd), log");
                break;
            }
            case 3434: {
                DECODE_DECIMAL("LENGTH (yd), log");
                break;
            }
            case 3435: {
                DECODE_DECIMAL("LENGTH (yd), log");
                break;
            }
            case 344: {
                DECODE("WIDTH (in), log");
                break;
            }
            case 3440: {
                DECODE("WIDTH (in), log");
                break;
            }
            case 3441: {
                DECODE_DECIMAL("WIDTH (in), log");
                break;
            }
            case 3442: {
                DECODE_DECIMAL("WIDTH (in), log");
                break;
            }
            case 3443: {
                DECODE_DECIMAL("WIDTH (in), log");
                break;
            }
            case 3444: {
                DECODE_DECIMAL("WIDTH (in), log");
                break;
            }
            case 3445: {
                DECODE_DECIMAL("WIDTH (in), log");
                break;
            }
            case 345: {
                DECODE("WIDTH (ft), log");
                break;
            }
            case 3450: {
                DECODE("WIDTH (ft), log");
                break;
            }
            case 3451: {
                DECODE_DECIMAL("WIDTH (ft), log");
                break;
            }
            case 3452: {
                DECODE_DECIMAL("WIDTH (ft), log");
                break;
            }
            case 3453: {
                DECODE_DECIMAL("WIDTH (ft), log");
                break;
            }
            case 3454: {
                DECODE_DECIMAL("WIDTH (ft), log");
                break;
            }
            case 3455: {
                DECODE_DECIMAL("WIDTH (ft), log");
                break;
            }
            case 346: {
                DECODE("WIDTH (yd), log");
                break;
            }
            case 3460: {
                DECODE("WIDTH (yd), log");
                break;
            }
            case 3461: {
                DECODE_DECIMAL("WIDTH (yd), log");
                break;
            }
            case 3462: {
                DECODE_DECIMAL("WIDTH (yd), log");
                break;
            }
            case 3463: {
                DECODE_DECIMAL("WIDTH (yd), log");
                break;
            }
            case 3464: {
                DECODE_DECIMAL("WIDTH (yd), log");
                break;
            }
            case 3465: {
                DECODE_DECIMAL("WIDTH (yd), log");
                break;
            }
            case 347: {
                DECODE("HEIGHT (in), log");
                break;
            }
            case 3470: {
                DECODE("HEIGHT (in), log");
                break;
            }
            case 3471: {
                DECODE_DECIMAL("HEIGHT (in), log");
                break;
            }
            case 3472: {
                DECODE_DECIMAL("HEIGHT (in), log");
                break;
            }
            case 3473: {
                DECODE_DECIMAL("HEIGHT (in), log");
                break;
            }
            case 3474: {
                DECODE_DECIMAL("HEIGHT (in), log");
                break;
            }
            case 3475: {
                DECODE_DECIMAL("HEIGHT (in), log");
                break;
            }
            case 348: {
                DECODE("HEIGHT (ft), log");
                break;
            }
            case 3480: {
                DECODE("HEIGHT (ft), log");
                break;
            }
            case 3481: {
                DECODE_DECIMAL("HEIGHT (ft), log");
                break;
            }
            case 3482: {
                DECODE_DECIMAL("HEIGHT (ft), log");
                break;
            }
            case 3483: {
                DECODE_DECIMAL("HEIGHT (ft), log");
                break;
            }
            case 3484: {
                DECODE_DECIMAL("HEIGHT (ft), log");
                break;
            }
            case 3485: {
                DECODE_DECIMAL("HEIGHT (ft), log");
                break;
            }
            case 349: {
                DECODE("HEIGHT (yd), log");
                break;
            }
            case 3490: {
                DECODE("HEIGHT (yd), log");
                break;
            }
            case 3491: {
                DECODE_DECIMAL("HEIGHT (yd), log");
                break;
            }
            case 3492: {
                DECODE_DECIMAL("HEIGHT (yd), log");
                break;
            }
            case 3493: {
                DECODE_DECIMAL("HEIGHT (yd), log");
                break;
            }
            case 3494: {
                DECODE_DECIMAL("HEIGHT (yd), log");
                break;
            }
            case 3495: {
                DECODE_DECIMAL("HEIGHT (yd), log");
                break;
            }
            case 350: {
                DECODE("AREA (in2)");
                break;
            }
            case 3500: {
                DECODE("AREA (in2)");
                break;
            }
            case 3501: {
                DECODE_DECIMAL("AREA (in2)");
                break;
            }
            case 3502: {
                DECODE_DECIMAL("AREA (in2)");
                break;
            }
            case 3503: {
                DECODE_DECIMAL("AREA (in2)");
                break;
            }
            case 3504: {
                DECODE_DECIMAL("AREA (in2)");
                break;
            }
            case 3505: {
                DECODE_DECIMAL("AREA (in2)");
                break;
            }
            case 351: {
                DECODE("AREA (ft2)");
                break;
            }
            case 3510: {
                DECODE("AREA (ft2)");
                break;
            }
            case 3511: {
                DECODE_DECIMAL("AREA (ft2)");
                break;
            }
            case 3512: {
                DECODE_DECIMAL("AREA (ft2)");
                break;
            }
            case 3513: {
                DECODE_DECIMAL("AREA (ft2)");
                break;
            }
            case 3514: {
                DECODE_DECIMAL("AREA (ft2)");
                break;
            }
            case 3515: {
                DECODE_DECIMAL("AREA (ft2)");
                break;
            }
            case 352: {
                DECODE("AREA (yd2)");
                break;
            }
            case 3520: {
                DECODE("AREA (yd2)");
                break;
            }
            case 3521: {
                DECODE_DECIMAL("AREA (yd2)");
                break;
            }
            case 3522: {
                DECODE_DECIMAL("AREA (yd2)");
                break;
            }
            case 3523: {
                DECODE_DECIMAL("AREA (yd2)");
                break;
            }
            case 3524: {
                DECODE_DECIMAL("AREA (yd2)");
                break;
            }
            case 3525: {
                DECODE_DECIMAL("AREA (yd2)");
                break;
            }
            case 353: {
                DECODE("AREA (in2), log");
                break;
            }
            case 3530: {
                DECODE("AREA (in2), log");
                break;
            }
            case 3531: {
                DECODE_DECIMAL("AREA (in2), log");
                break;
            }
            case 3532: {
                DECODE_DECIMAL("AREA (in2), log");
                break;
            }
            case 3533: {
                DECODE_DECIMAL("AREA (in2), log");
                break;
            }
            case 3534: {
                DECODE_DECIMAL("AREA (in2), log");
                break;
            }
            case 3535: {
                DECODE_DECIMAL("AREA (in2), log");
                break;
            }
            case 354: {
                DECODE("AREA (ft2), log");
                break;
            }
            case 3540: {
                DECODE("AREA (ft2), log");
                break;
            }
            case 3541: {
                DECODE_DECIMAL("AREA (ft2), log");
                break;
            }
            case 3542: {
                DECODE_DECIMAL("AREA (ft2), log");
                break;
            }
            case 3543: {
                DECODE_DECIMAL("AREA (ft2), log");
                break;
            }
            case 3544: {
                DECODE_DECIMAL("AREA (ft2), log");
                break;
            }
            case 3545: {
                DECODE_DECIMAL("AREA (ft2), log");
                break;
            }
            case 355: {
                DECODE("AREA (yd2), log");
                break;
            }
            case 3550: {
                DECODE("AREA (yd2), log");
                break;
            }
            case 3551: {
                DECODE_DECIMAL("AREA (yd2), log");
                break;
            }
            case 3552: {
                DECODE_DECIMAL("AREA (yd2), log");
                break;
            }
            case 3553: {
                DECODE_DECIMAL("AREA (yd2), log");
                break;
            }
            case 3554: {
                DECODE_DECIMAL("AREA (yd2), log");
                break;
            }
            case 3555: {
                DECODE_DECIMAL("AREA (yd2), log");
                break;
            }
            case 356: {
                DECODE("NET WEIGHT (tr oz)");
                break;
            }
            case 3560: {
                DECODE("NET WEIGHT (tr oz)");
                break;
            }
            case 3561: {
                DECODE_DECIMAL("NET WEIGHT (tr oz)");
                break;
            }
            case 3562: {
                DECODE_DECIMAL("NET WEIGHT (tr oz)");
                break;
            }
            case 3563: {
                DECODE_DECIMAL("NET WEIGHT (tr oz)");
                break;
            }
            case 3564: {
                DECODE_DECIMAL("NET WEIGHT (tr oz)");
                break;
            }
            case 3565: {
                DECODE_DECIMAL("NET WEIGHT (tr oz)");
                break;
            }
            case 357: {
                DECODE("NET VOLUME (oz)");
                break;
            }
            case 3570: {
                DECODE("NET VOLUME (oz)");
                break;
            }
            case 3571: {
                DECODE_DECIMAL("NET VOLUME (oz)");
                break;
            }
            case 3572: {
                DECODE_DECIMAL("NET VOLUME (oz)");
                break;
            }
            case 3573: {
                DECODE_DECIMAL("NET VOLUME (oz)");
                break;
            }
            case 3574: {
                DECODE_DECIMAL("NET VOLUME (oz)");
                break;
            }
            case 3575: {
                DECODE_DECIMAL("NET VOLUME (oz)");
                break;
            }
            case 360: {
                DECODE("NET VOLUME (qt US)");
                break;
            }
            case 3600: {
                DECODE("NET VOLUME (qt US)");
                break;
            }
            case 3601: {
                DECODE_DECIMAL("NET VOLUME (qt US)");
                break;
            }
            case 3602: {
                DECODE_DECIMAL("NET VOLUME (qt US)");
                break;
            }
            case 3603: {
                DECODE_DECIMAL("NET VOLUME (qt US)");
                break;
            }
            case 3604: {
                DECODE_DECIMAL("NET VOLUME (qt US)");
                break;
            }
            case 3605: {
                DECODE_DECIMAL("NET VOLUME (qt US)");
                break;
            }
            case 361: {
                DECODE("NET VOLUME (gal US)");
                break;
            }
            case 3610: {
                DECODE("NET VOLUME (gal US)");
                break;
            }
            case 3611: {
                DECODE_DECIMAL("NET VOLUME (gal US)");
                break;
            }
            case 3612: {
                DECODE_DECIMAL("NET VOLUME (gal US)");
                break;
            }
            case 3613: {
                DECODE_DECIMAL("NET VOLUME (gal US)");
                break;
            }
            case 3614: {
                DECODE_DECIMAL("NET VOLUME (gal US)");
                break;
            }
            case 3615: {
                DECODE_DECIMAL("NET VOLUME (gal US)");
                break;
            }
            case 362: {
                DECODE("VOLUME (qt US), log");
                break;
            }
            case 3620: {
                DECODE("VOLUME (qt US), log");
                break;
            }
            case 3621: {
                DECODE_DECIMAL("VOLUME (qt US), log");
                break;
            }
            case 3622: {
                DECODE_DECIMAL("VOLUME (qt US), log");
                break;
            }
            case 3623: {
                DECODE_DECIMAL("VOLUME (qt US), log");
                break;
            }
            case 3624: {
                DECODE_DECIMAL("VOLUME (qt US), log");
                break;
            }
            case 3625: {
                DECODE_DECIMAL("VOLUME (qt US), log");
                break;
            }
            case 363: {
                DECODE("VOLUME (gal US), log");
                break;
            }
            case 3630: {
                DECODE("VOLUME (gal US), log");
                break;
            }
            case 3631: {
                DECODE_DECIMAL("VOLUME (gal US), log");
                break;
            }
            case 3632: {
                DECODE_DECIMAL("VOLUME (gal US), log");
                break;
            }
            case 3633: {
                DECODE_DECIMAL("VOLUME (gal US), log");
                break;
            }
            case 3634: {
                DECODE_DECIMAL("VOLUME (gal US), log");
                break;
            }
            case 3635: {
                DECODE_DECIMAL("VOLUME (gal US), log");
                break;
            }
            case 364: {
                DECODE("VOLUME (in3)");
                break;
            }
            case 3640: {
                DECODE("VOLUME (in3)");
                break;
            }
            case 3641: {
                DECODE_DECIMAL("VOLUME (in3)");
                break;
            }
            case 3642: {
                DECODE_DECIMAL("VOLUME (in3)");
                break;
            }
            case 3643: {
                DECODE_DECIMAL("VOLUME (in3)");
                break;
            }
            case 3644: {
                DECODE_DECIMAL("VOLUME (in3)");
                break;
            }
            case 3645: {
                DECODE_DECIMAL("VOLUME (in3)");
                break;
            }
            case 365: {
                DECODE("VOLUME (ft3)");
                break;
            }
            case 3650: {
                DECODE("VOLUME (ft3)");
                break;
            }
            case 3651: {
                DECODE_DECIMAL("VOLUME (ft3)");
                break;
            }
            case 3652: {
                DECODE_DECIMAL("VOLUME (ft3)");
                break;
            }
            case 3653: {
                DECODE_DECIMAL("VOLUME (ft3)");
                break;
            }
            case 3654: {
                DECODE_DECIMAL("VOLUME (ft3)");
                break;
            }
            case 3655: {
                DECODE_DECIMAL("VOLUME (ft3)");
                break;
            }
            case 366: {
                DECODE("VOLUME (yd3)");
                break;
            }
            case 3660: {
                DECODE("VOLUME (yd3)");
                break;
            }
            case 3661: {
                DECODE_DECIMAL("VOLUME (yd3)");
                break;
            }
            case 3662: {
                DECODE_DECIMAL("VOLUME (yd3)");
                break;
            }
            case 3663: {
                DECODE_DECIMAL("VOLUME (yd3)");
                break;
            }
            case 3664: {
                DECODE_DECIMAL("VOLUME (yd3)");
                break;
            }
            case 3665: {
                DECODE_DECIMAL("VOLUME (yd3)");
                break;
            }
            case 367: {
                DECODE("VOLUME (in3), log");
                break;
            }
            case 3670: {
                DECODE("VOLUME (in3), log");
                break;
            }
            case 3671: {
                DECODE_DECIMAL("VOLUME (in3), log");
                break;
            }
            case 3672: {
                DECODE_DECIMAL("VOLUME (in3), log");
                break;
            }
            case 3673: {
                DECODE_DECIMAL("VOLUME (in3), log");
                break;
            }
            case 3674: {
                DECODE_DECIMAL("VOLUME (in3), log");
                break;
            }
            case 3675: {
                DECODE_DECIMAL("VOLUME (in3), log");
                break;
            }
            case 368: {
                DECODE("VOLUME (ft3), log");
                break;
            }
            case 3680: {
                DECODE("VOLUME (ft3), log");
                break;
            }
            case 3681: {
                DECODE_DECIMAL("VOLUME (ft3), log");
                break;
            }
            case 3682: {
                DECODE_DECIMAL("VOLUME (ft3), log");
                break;
            }
            case 3683: {
                DECODE_DECIMAL("VOLUME (ft3), log");
                break;
            }
            case 3684: {
                DECODE_DECIMAL("VOLUME (ft3), log");
                break;
            }
            case 3685: {
                DECODE_DECIMAL("VOLUME (ft3), log");
                break;
            }
            case 369: {
                DECODE("VOLUME (yd3), log");
                break;
            }
            case 3690: {
                DECODE("VOLUME (yd3), log");
                break;
            }
            case 3691: {
                DECODE_DECIMAL("VOLUME (yd3), log");
                break;
            }
            case 3692: {
                DECODE_DECIMAL("VOLUME (yd3), log");
                break;
            }
            case 3693: {
                DECODE_DECIMAL("VOLUME (yd3), log");
                break;
            }
            case 3694: {
                DECODE_DECIMAL("VOLUME (yd3), log");
                break;
            }
            case 3695: {
                DECODE_DECIMAL("VOLUME (yd3), log");
                break;
            }
            case 3900: {
                DECODE_DECIMAL("AMOUNT");
                break;
            }
            case 3901: {
                DECODE_DECIMAL("AMOUNT");
                break;
            }
            case 3902: {
                DECODE_DECIMAL("AMOUNT");
                break;
            }
            case 3910: {
                DECODE_CURRENCY("AMOUNT");
                break;
            }
            case 3911: {
                DECODE_CURRENCY("AMOUNT");
                break;
            }
            case 3912: {
                DECODE_CURRENCY("AMOUNT");
                break;
            }
            case 3920: {
                DECODE_DECIMAL("PRICE");
                break;
            }
            case 3921: {
                DECODE_DECIMAL("PRICE");
                break;
            }
            case 3922: {
                DECODE_DECIMAL("PRICE");
                break;
            }
            case 3930: {
                DECODE_CURRENCY("PRICE");
                break;
            }
            case 3931: {
                DECODE_CURRENCY("PRICE");
                break;
            }
            case 3932: {
                DECODE_CURRENCY("PRICE");
                break;
            }
            case 394: {
                DECODE("PRCNT OFF");
                break;
            }
            case 3940: {
                DECODE("PRCNT OFF");
                break;
            }
            case 3941: {
                DECODE_DECIMAL("PRCNT OFF");
                break;
            }
            case 3942: {
                DECODE_DECIMAL("PRCNT OFF");
                break;
            }
            case 3943: {
                DECODE_DECIMAL("PRCNT OFF");
                break;
            }
            case 3950: {
                DECODE_DECIMAL("PRICE/UoM");
                break;
            }
            case 3951: {
                DECODE_DECIMAL("PRICE/UoM");
                break;
            }
            case 3952: {
                DECODE_DECIMAL("PRICE/UoM");
                break;
            }
            case 3953: {
                DECODE_DECIMAL("PRICE/UoM");
                break;
            }
            case 400: {
                DECODE("ORDER NUMBER");
                break;
            }
            case 401: {
                if (debug == 1) printf("GINC ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GINC: ");
                    if ((int)strlen(data_str) > 4) {
                        int ginc_start_index = 0;
                        if (data_str[0] == '0') {
                            ginc_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   ginc_start_index, data_str);
                    }
                }
                break;
            }
            case 402: {
                if (debug == 1) printf("GSIN ");
                if (is_digital_link == 0) {
                    /* see https://edi.gs1si.org/cashedi/doc/gsin_intro.pdf */
                    decode_strcat(gs1_result, "GSIN: ");
                    if ((int)strlen(data_str) > 4) {
                        int gsin_start_index = 0;
                        if (data_str[0] == '0') {
                            gsin_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gsin_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gsin_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gsin_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gsin_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 403: {
                DECODE("ROUTE");
                break;
            }
            case 410: {
                if (debug == 1) printf("SHIP TO LOC ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "SHIP TO LOC: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 411: {
                if (debug == 1) printf("BILL TO ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "BILL TO: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 412: {
                if (debug == 1) printf("PURCHASE FROM ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "PURCHASE FROM: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 413: {
                if (debug == 1) printf("SHIP FOR LOC ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "SHIP FOR LOC: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 414: {
                if (debug == 1) printf("LOCN NO ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "LOCN NO: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 415: {
                if (debug == 1) printf("PAY TO ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "PAY TO: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 416: {
                if (debug == 1) printf("PROD/SERV LOC ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "PROD/SERV LOC: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 417: {
                if (debug == 1) printf("PARTY ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "PARTY: ");
                    if ((int)strlen(data_str) > 4) {
                        int gln_start_index = 0;
                        if (data_str[0] == '0') {
                            gln_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gln_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gln_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gln_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gln_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 420: {
                DECODE("SHIP TO POST");
                break;
            }
            case 421: {
                DECODE_COUNTRY("SHIP TO POST");
                break;
            }
            case 422: {
                DECODE("ORIGIN");
                break;
            }
            case 423: {
                DECODE_COUNTRY("COUNTRY – INITIAL PROCESS");
                break;
            }
            case 424: {
                DECODE_COUNTRY("COUNTRY – PROCESS");
                break;
            }
            case 425: {
                DECODE_COUNTRY("COUNTRY – DISASSEMBLY");
                break;
            }
            case 426: {
                DECODE_COUNTRY("COUNTRY – FULL PROCESS");
                break;
            }
            case 703: {
                DECODE_COUNTRY("PROCESSOR # s4");
                break;
            }
            case 710: {
                DECODE("NHRN GERMANY IFA");
                break;
            }
            case 711: {
                DECODE("NHRN FRANCE CIP");
                break;
            }
            case 712: {
                DECODE("NHRN SPAIN NATIONAL CODE");
                break;
            }
            case 713: {
                DECODE("NHRN BRASIL ANVISA");
                break;
            }
            case 714: {
                DECODE("NHRN PORTUGAL INFARMED");
                break;
            }
            case 715: {
                DECODE("NHRN US FDA");
                break;
            }
            case 716: {
                DECODE("NHRN ITALY AIFA");
                break;
            }
            case 4300: {
                DECODE("SHIP TO COMP");
                break;
            }
            case 4301: {
                DECODE("SHIP TO NAME");
                break;
            }
            case 4302: {
                DECODE("SHIP TO ADD1");
                break;
            }
            case 4303: {
                DECODE("SHIP TO ADD2");
                break;
            }
            case 4304: {
                DECODE("SHIP TO SUB");
                break;
            }
            case 4305: {
                DECODE("SHIP TO LOC");
                break;
            }
            case 4306: {
                DECODE("SHIP TO REG");
                break;
            }
            case 4307: {
                DECODE_COUNTRY_ALPHA2("SHIP TO COUNTRY");
                break;
            }
            case 4308: {
                DECODE("SHIP TO PHONE");
                break;
            }
            case 4309: {
                if (debug == 1) printf("SHIP TO GEO ");
                if (is_digital_link == 0) {
                    /* See section 7.14 of GS1 General Specifications */
                    decode_strcat(gs1_result, "SHIP TO GEO: ");
                    if ((int)strlen(data_str) == 20) {
                        char latlon_str[11];
                        int dec_ctr = 0;
                        /* first ten characters are latitude */
                        for (dec_ctr = 0; dec_ctr < 10; dec_ctr++) {
                            if ((data_str[dec_ctr] < '0') ||
                                    (data_str[dec_ctr] > '9')) break;
                            latlon_str[dec_ctr] = data_str[dec_ctr];
                        }
                        latlon_str[dec_ctr] = 0;
                        if ((int)strlen(latlon_str) == 10) {
                            latitude = (atof(latlon_str) / 10000000.0f) - 90.0f;

                            /* second ten characters are longitude */
                            dec_ctr = 0;
                            for (dec_ctr = 0; dec_ctr < 10; dec_ctr++) {
                                if ((data_str[dec_ctr] < '0') ||
                                        (data_str[dec_ctr] > '9')) break;
                                latlon_str[dec_ctr] = data_str[dec_ctr + 10];
                            }
                            latlon_str[dec_ctr] = 0;
                            if ((int)strlen(latlon_str) == 10) {
                                longitude =
                                    (float)fmod((double)((atof(latlon_str) /
                                                          10000000.0f) + 180.0f), 360.0) -
                                    180.0f;
                            }
                        }
                    }
                }
                break;
            }
            case 4310: {
                DECODE("RTN TO COMP");
                break;
            }
            case 4311: {
                DECODE("RTN TO NAME");
                break;
            }
            case 4312: {
                DECODE("RTN TO ADD1");
                break;
            }
            case 4313: {
                DECODE("RTN TO ADD2");
                break;
            }
            case 4314: {
                DECODE("RTN TO SUB");
                break;
            }
            case 4315: {
                DECODE("RTN TO LOC");
                break;
            }
            case 4316: {
                DECODE("RTN TO REG");
                break;
            }
            case 4317: {
                DECODE_COUNTRY_ALPHA2("RTN TO COUNTRY");
                break;
            }
            case 4318: {
                DECODE("RTN TO POST");
                break;
            }
            case 4319: {
                DECODE("RTN TO PHONE");
                break;
            }
            case 4320: {
                DECODE("SRV DESCRIPTION");
                break;
            }
            case 4321: {
                DECODE("DANGEROUS GOODS");
                break;
            }
            case 4322: {
                if (debug == 1) printf("AUTH LEAVE ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "AUTH LEAVE: ");
                    authority_to_leave = 0;
                    if (data_str[0] == '1') authority_to_leave = 1;
                }
                break;
            }
            case 4323: {
                if (debug == 1) printf("SIG REQUIRED ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "SIG REQUIRED: ");
                    signature_required = 0;
                    if (data_str[0] == '1') signature_required = 1;
                }
                break;
            }
            case 4324: {
                DECODE_DATE("NBEF DEL DT", "YYMMDDHHMM");
                break;
            }
            case 4325: {
                DECODE_DATE("NAFT DEL DT", "YYMMDDHHMM");
                break;
            }
            case 4326: {
                DECODE_DATE("REL DATE", "YYMMDD");
                break;
            }
            case 4330: {
                DECODE_TEMPERATURE("MAX TEMP F");
                break;
            }
            case 4331: {
                DECODE_TEMPERATURE("MAX TEMP C");
                break;
            }
            case 4332: {
                DECODE_TEMPERATURE("MIN TEMP F");
                break;
            }
            case 4333: {
                DECODE_TEMPERATURE("MIN TEMP C");
                break;
            }
            case 7001: {
                DECODE("NSN");
                break;
            }
            case 7002: {
                DECODE("MEAT CUT");
                break;
            }
            case 7003: {
                DECODE_DATE("EXPIRY TIME", "YYMMDDHHMM");
                break;
            }
            case 7004: {
                DECODE("ACTIVE POTENCY");
                break;
            }
            case 7005: {
                DECODE("CATCH AREA");
                break;
            }
            case 7006: {
                DECODE_DATE("FIRST FREEZE DATE", "YYMMDD");
                break;
            }
            case 7007: {
                if (debug == 1) printf("HARVEST DATE ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "HARVEST DATE: ");
                    date_str = data_id_convert_date("YYMMDD", data_str);
                    if ((date_str != NULL) && ((int)strlen(data_str) == 12)) {
                        end_date_str = data_id_convert_date("YYMMDD", &data_str[6]);
                    }
                }
                break;
            }
            case 7008: {
                DECODE("AQUATIC SPECIES");
                break;
            }
            case 7009: {
                DECODE("FISHING GEAR TYPE");
                break;
            }
            case 7010: {
                DECODE("PROD METHOD");
                break;
            }
            case 7011: {
                if (debug == 1) printf("TEST BY DATE ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "TEST BY DATE: ");
                    if ((int)strlen(data_str) == 6) {
                        date_str = data_id_convert_date("YYMMDD", data_str);
                    }
                    else if ((int)strlen(data_str) == 10) {
                        date_str = data_id_convert_date("YYMMDDHHMM", data_str);
                    }
                }
                break;
            }
            case 7020: {
                DECODE("REFURB LOT");
                break;
            }
            case 7021: {
                DECODE("FUNC STAT");
                break;
            }
            case 7022: {
                DECODE("REV STAT");
                break;
            }
            case 7023: {
                if (debug == 1) printf("GIAI – ASSEMBLY ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GIAI – ASSEMBLY: ");
                    if ((int)strlen(data_str) > 4) {
                        int giai_start_index = 0;
                        if (data_str[0] == '0') {
                            giai_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   giai_start_index, data_str);
                    }
                }
                break;
            }
            case 7030: {
                DECODE_PROCESSOR("PROCESSOR # 0");
                break;
            }
            case 7031: {
                DECODE_PROCESSOR("PROCESSOR # 1");
                break;
            }
            case 7032: {
                DECODE_PROCESSOR("PROCESSOR # 2");
                break;
            }
            case 7033: {
                DECODE_PROCESSOR("PROCESSOR # 3");
                break;
            }
            case 7034: {
                DECODE_PROCESSOR("PROCESSOR # 4");
                break;
            }
            case 7035: {
                DECODE_PROCESSOR("PROCESSOR # 5");
                break;
            }
            case 7036: {
                DECODE_PROCESSOR("PROCESSOR # 6");
                break;
            }
            case 7037: {
                DECODE_PROCESSOR("PROCESSOR # 7");
                break;
            }
            case 7038: {
                DECODE_PROCESSOR("PROCESSOR # 8");
                break;
            }
            case 7039: {
                DECODE_PROCESSOR("PROCESSOR # 9");
                break;
            }
            case 7040: {
                DECODE("UIC+EXT");
                break;
            }
            case 7041: {
                if (debug == 1) printf("UFRGT UNIT TYPE ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "UFRGT UNIT TYPE: ");
                    package_type_str = get_package_type(data_str);
                }
                break;
            }
            case 7240: {
                DECODE("PROTOCOL");
                break;
            }
            case 7241: {
                if (debug == 1) printf("AIDC MEDIA TYPE ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "AIDC MEDIA TYPE: ");
                    if ((int)strlen(data_str) == 2) {
                        if ((data_str[0] >= '0') && (data_str[0] <= '9') &&
                                (data_str[1] >= '0') && (data_str[1] <= '9')) {
                            aidc_media_type = atoi(data_str);
                        }
                    }
                }
                break;
            }
            case 7242: {
                DECODE("VCN");
                break;
            }
            case 7250: {
                DECODE_DATE("DOB", "YYYYMMDD");
                break;
            }
            case 7251: {
                DECODE_DATE("DOB TIME", "YYYYMMDDHHMM");
                break;
            }
            case 7252: {
                if (debug == 1) printf("BIO SEX ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "BIO SEX: ");
                    if ((int)strlen(data_str) == 1) {
                        if ((data_str[0] >= '0') && (data_str[0] <= '9')) {
                            bio_sex = atoi(data_str);
                        }
                    }
                }
                break;
            }
            case 7253: {
                DECODE("FAMILY NAME");
                break;
            }
            case 7254: {
                DECODE("GIVEN NAME");
                break;
            }
            case 7255: {
                DECODE("SUFFIX");
                break;
            }
            case 7256: {
                DECODE("FULL NAME");
                break;
            }
            case 7257: {
                DECODE("PERSON ADDR");
                break;
            }
            case 7258: {
                if (debug == 1) printf("BIRTH SEQUENCE ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "BIRTH SEQUENCE: ");
                    if ((int)strlen(data_str) == 3) {
                        if ((data_str[0] >= '0') && (data_str[0] <= '9') &&
                                (data_str[2] >= '0') && (data_str[2] <= '9')) {
                            birth_sequence[0] = data_str[0];
                            birth_sequence[1] = data_str[2];
                        }
                    }
                }
                break;
            }
            case 7259: {
                DECODE("BABY");
                break;
            }
            case 8001: {
                if (debug == 1) printf("DIMENSIONS ");
                if (is_digital_link == 0) {
                    /* GS1 General Specifications section 3.9.1 */
                    if ((int)strlen(data_str) == 14) {
                        unsigned char all_numbers = 1;
                        for (int dim_ctr = 0; dim_ctr < 14; dim_ctr++) {
                            if ((data_str[dim_ctr] < '0') ||
                                    (data_str[dim_ctr] > '9')) {
                                all_numbers = 0;
                                break;
                            }
                        }
                        if (all_numbers == 1) {
                            char roll_width_mm_str[5];
                            roll_width_mm_str[0] = data_str[0];
                            roll_width_mm_str[1] = data_str[1];
                            roll_width_mm_str[2] = data_str[2];
                            roll_width_mm_str[3] = data_str[3];
                            roll_width_mm_str[4] = 0;
                            roll_width_mm = atoi(roll_width_mm_str);

                            char roll_length_m_str[6];
                            roll_length_m_str[0] = data_str[4];
                            roll_length_m_str[1] = data_str[5];
                            roll_length_m_str[2] = data_str[6];
                            roll_length_m_str[3] = data_str[7];
                            roll_length_m_str[4] = data_str[8];
                            roll_length_m_str[5] = 0;
                            roll_length_metres = atoi(roll_length_m_str);

                            char roll_diameter_mm_str[4];
                            roll_diameter_mm_str[0] = data_str[9];
                            roll_diameter_mm_str[1] = data_str[10];
                            roll_diameter_mm_str[2] = data_str[11];
                            roll_diameter_mm_str[3] = 0;
                            roll_diameter_mm = atoi(roll_diameter_mm_str);

                            char roll_winding_direction_str[2];
                            roll_winding_direction_str[0] = data_str[12];
                            roll_winding_direction_str[1] = 0;
                            roll_winding_direction =
                                atoi(&roll_winding_direction_str[0]);

                            char roll_no_of_splices_str[2];
                            roll_no_of_splices_str[0] = data_str[13];
                            roll_no_of_splices_str[1] = 0;
                            roll_no_of_splices = atoi(&roll_no_of_splices_str[0]);
                        }
                    }
                    if ((roll_width_mm == -1) ||
                            (roll_length_metres == -1) ||
                            (roll_diameter_mm == -1) ||
                            (roll_no_of_splices == -1) ||
                            (roll_winding_direction == -1)) {
                        decode_strcat(gs1_result, "DIMENSIONS: ");
                    }
                }
                break;
            }
            case 8002: {
                DECODE("MOBILE TEL NO");
                break;
            }
            case 8003: {
                if (debug == 1) printf("GRAI ");
                if (is_digital_link == 0) {
                    /* GS1 General Specifications section 3.9.3 */
                    decode_strcat(gs1_result, "GRAI: ");
                    if ((int)strlen(data_str) > 5) {
                        /* first digit is always zero */
                        int grai_start_index = 1;
                        if (data_str[1] == '0') {
                            grai_start_index = 2;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   grai_start_index, data_str);
                        /* TODO handle optional serial component */
                        int check_digit =
                            get_gtin_check_digit(&data_str[grai_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                grai_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    grai_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 8004: {
                if (debug == 1) printf("GIAI ");
                if (is_digital_link == 0) {
                    /* GS1 General Specifications section 3.9.4 */
                    decode_strcat(gs1_result, "GIAI: ");
                    if ((int)strlen(data_str) > 5) {
                        /* first digit is always zero */
                        int grai_start_index = 0;
                        if (data_str[1] == '0') {
                            grai_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   grai_start_index, data_str);
                    }
                }
                break;
            }
            case 8005: {
                DECODE("PRICE PER UNIT");
                break;
            }
            case 8006: {
                if (debug == 1) printf("ITIP ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "ITIP: ");
                    if ((int)strlen(data_str) > 5) {
                        /* first digit is always zero */
                        int itip_start_index = 0;
                        if (data_str[1] == '0') {
                            itip_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   itip_start_index, data_str);
                    }
                    if ((int)strlen(data_str) == 18) {
                        if ((data_str[14] >= '0') && (data_str[14] <= '9') &&
                                (data_str[15] >= '0') && (data_str[15] <= '9')) {
                            itip_piece_number_str[0] = data_str[14];
                            itip_piece_number_str[1] = data_str[15];
                            itip_piece_number_str[2] = 0;
                        }
                        if ((data_str[16] >= '0') && (data_str[16] <= '9') &&
                                (data_str[17] >= '0') && (data_str[17] <= '9')) {
                            itip_total_count_str[0] = data_str[16];
                            itip_total_count_str[1] = data_str[17];
                            itip_total_count_str[2] = 0;
                        }
                    }
                }
                break;
            }
            case 8007: {
                DECODE("IBAN");
                break;
            }
            case 8008: {
                DECODE_DATE("PROD TIME", "YYMMDDHH");
                break;
            }
            case 8010: {
                DECODE("CPID");
                break;
            }
            case 8011: {
                DECODE("CPID SERIAL");
                break;
            }
            case 8012: {
                DECODE("VERSION");
                break;
            }
            case 8013: {
                if (debug == 1) printf("GMN ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GMN: ");
                    if ((int)strlen(data_str) > 4) {
                        int gmn_start_index = 0;
                        if (data_str[0] == '0') {
                            gmn_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gmn_start_index, data_str);
                    }
                }
                break;
            }
            case 8017: {
                if (debug == 1) printf("GSRN - PROVIDER ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GSRN - PROVIDER: ");
                    if ((int)strlen(data_str) > 4) {
                        int gsrn_start_index = 0;
                        if (data_str[0] == '0') {
                            gsrn_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gsrn_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gsrn_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gsrn_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gsrn_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 8018: {
                if (debug == 1) printf("GSRN - RECIPIENT ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "GSRN - RECIPIENT: ");
                    if ((int)strlen(data_str) > 4) {
                        int gsrn_start_index = 0;
                        if (data_str[0] == '0') {
                            gsrn_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   gsrn_start_index, data_str);

                        int check_digit =
                            get_gtin_check_digit(&data_str[gsrn_start_index], 1);
                        if (check_digit != -1) {
                            char last_char = data_str[(int)strlen(data_str)-1];
                            last_str[0] = last_char;
                            last_str[1] = 0;
                            if ((last_char >= '0') && (last_char <= '9')) {
                                gsrn_check_digit_passed = 0;
                                if (atoi(last_str) == check_digit) {
                                    gsrn_check_digit_passed = 1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            case 8019: {
                DECODE("SRIN");
                break;
            }
            case 8020: {
                DECODE("REF NO");
                break;
            }
            case 8026: {
                if (debug == 1) printf("ITIP CONTENT ");
                if (is_digital_link == 0) {
                    decode_strcat(gs1_result, "ITIP CONTENT: ");
                    if ((int)strlen(data_str) > 5) {
                        /* first digit is always zero */
                        int itip_start_index = 0;
                        if (data_str[1] == '0') {
                            itip_start_index = 1;
                        }
                        company_prefix_str =
                            get_company_prefix_str(&company_prefix_code[0],
                                                   itip_start_index, data_str);
                    }
                    if ((int)strlen(data_str) == 18) {
                        if ((data_str[14] >= '0') && (data_str[14] <= '9') &&
                                (data_str[15] >= '0') && (data_str[15] <= '9')) {
                            itip_piece_number_str[0] = data_str[14];
                            itip_piece_number_str[1] = data_str[15];
                            itip_piece_number_str[2] = 0;
                        }
                        if ((data_str[16] >= '0') && (data_str[16] <= '9') &&
                                (data_str[17] >= '0') && (data_str[17] <= '9')) {
                            itip_total_count_str[0] = data_str[16];
                            itip_total_count_str[1] = data_str[17];
                            itip_total_count_str[2] = 0;
                        }
                    }
                }
                break;
            }
            case 8030: {
                DECODE("DIGSIG");
                break;
            }
            case 8110: {
                DECODE("COUPON");
                break;
            }
            case 8111: {
                DECODE("POINTS");
                break;
            }
            case 8112: {
                DECODE("COUPON");
                break;
            }
            case 8200: {
                DECODE("PRODUCT URL");
                break;
            }
            case 977: {
                if (debug == 1) printf("ISSN ");
                if (is_digital_link == 0) {
                    issn_str = get_issn(data_str);
                    if (issn_str == NULL) {
                        decode_strcat(gs1_result, "ISSN: ");
                    }
                }
                break;
            }
            case 978: {
                DECODE("ISBN");
                break;
            }
            case 979: {
                DECODE("ISBN");
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
                    if (end_date_str != NULL) {
                        /* date span */
                        decode_strcat(gs1_result, " - ");
                        decode_strcat(gs1_result, end_date_str);
                        free(end_date_str);
                    }
                }
                else if (aidc_media_type != -1) {
                    decode_strcat(gs1_result, data_str);
                    switch(aidc_media_type) {
                    case 0: {
                        decode_strcat(gs1_result, "NOT USED");
                        break;
                    }
                    case 1: {
                        decode_strcat(gs1_result, "WRISTBAND");
                        break;
                    }
                    case 2: {
                        decode_strcat(gs1_result, "ORDER FORM");
                        break;
                    }
                    case 3: {
                        decode_strcat(gs1_result, "SAMPLE TUBE");
                        break;
                    }
                    case 4: {
                        decode_strcat(gs1_result, "WORKING LIST / LAB LIST");
                        break;
                    }
                    case 5: {
                        decode_strcat(gs1_result, "TEST REPORT");
                        break;
                    }
                    case 6: {
                        decode_strcat(gs1_result, "DELIVERY NOTE");
                        break;
                    }
                    case 7: {
                        decode_strcat(gs1_result, "INTENDED RECIPIENT LABEL");
                        break;
                    }
                    case 8: {
                        decode_strcat(gs1_result, "LABEL AFFIXED TO PRODUCT");
                        break;
                    }
                    case 9: {
                        decode_strcat(gs1_result, "IDENTITY CARD");
                        break;
                    }
                    case 10: {
                        decode_strcat(gs1_result, "CLINICAL OR PROGRESS NOTES");
                        break;
                    }
                    }
                    if (aidc_media_type > 10) {
                        decode_strcat(gs1_result, "RESERVED");
                    }
                }
                else if (curr_str != NULL) {
                    decode_strcat(gs1_result, curr_str);
                    free(curr_str);
                }
                else if (decimal_str != NULL) {
                    decode_strcat(gs1_result, decimal_str);
                    free(decimal_str);
                }
                else if (country_str != NULL) {
                    decode_strcat(gs1_result, country_str);
                    free(country_str);
                }
                else if (processor_country_str != NULL) {
                    decode_strcat(gs1_result, processor_country_str);
                    free(processor_country_str);
                    decode_strcat_char(gs1_result, ' ');
                    decode_strcat(gs1_result, &data_str[3]);
                }
                else if (coupon_str != NULL) {
                    decode_strcat(gs1_result, coupon_str);
                    free(coupon_str);
                }
                else if (issn_str != NULL) {
                    decode_strcat(gs1_result, issn_str);
                    free(issn_str);
                }
                else if (company_prefix_str != NULL) {
                    decode_strcat(gs1_result, data_str);
                    decode_strcat_char(gs1_result, '\n');
                    decode_strcat(gs1_result, "COUNTRY: ");
                    decode_strcat(gs1_result, company_prefix_str);
                    free(company_prefix_str);
                }
                else if (temperature != UNKNOWN_VALUE) {
                    decode_strcat(gs1_result, data_str);
                    char * temp_str =
                        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
                    sprintf(temp_str, "%.2f", temperature);
                    decode_strcat(gs1_result, temp_str);
                    free(temp_str);
                }
                else if ((latitude != UNKNOWN_VALUE) &&
                         (longitude != UNKNOWN_VALUE)) {
                    decode_strcat(gs1_result, data_str);
                    char * geo_str =
                        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
                    sprintf(geo_str, "LAT %.4f LON %.4f", latitude, longitude);
                    decode_strcat(gs1_result, geo_str);
                    free(geo_str);
                }
                else if (authority_to_leave != -1) {
                    decode_strcat(gs1_result, data_str);
                    if (authority_to_leave == 0) {
                        decode_strcat(gs1_result, "NO");
                    }
                    else {
                        decode_strcat(gs1_result, "YES");
                    }
                }
                else if (signature_required != -1) {
                    decode_strcat(gs1_result, data_str);
                    if (signature_required == 0) {
                        decode_strcat(gs1_result, "NO");
                    }
                    else {
                        decode_strcat(gs1_result, "YES");
                    }
                }
                else if ((roll_width_mm != -1) &&
                         (roll_length_metres != -1) &&
                         (roll_diameter_mm != -1) &&
                         (roll_winding_direction != -1) &&
                         (roll_no_of_splices != -1)) {
                    char roll_dimensions[32];
                    sprintf(&roll_dimensions[0], "WIDTH %dmm, ",
                            roll_width_mm);
                    decode_strcat(gs1_result, &roll_dimensions[0]);
                    sprintf(&roll_dimensions[0], "LENGTH %dm, ",
                            roll_length_metres);
                    decode_strcat(gs1_result, &roll_dimensions[0]);
                    sprintf(&roll_dimensions[0], "DIAM %dmm\n",
                            roll_diameter_mm);
                    decode_strcat(gs1_result, &roll_dimensions[0]);
                    if (roll_winding_direction == 0) {
                        decode_strcat(gs1_result,
                                      "WINDING DIRECTION: FACE OUT\n");
                    }
                    else if (roll_winding_direction == 1) {
                        decode_strcat(gs1_result,
                                      "WINDING DIRECTION: FACE IN\n");
                    }
                    else {
                        decode_strcat(gs1_result,
                                      "WINDING DIRECTION: UNDEFINED\n");
                    }
                    if (roll_no_of_splices != 9) {
                        sprintf(&roll_dimensions[0], "SPLICES: %d",
                                roll_no_of_splices);
                        decode_strcat(gs1_result, &roll_dimensions[0]);
                    }
                    else {
                        decode_strcat(gs1_result, "SPLICES: UNKNOWN");
                    }
                }
                else if (birth_sequence[0] != 0) {
                    decode_strcat(gs1_result, data_str);
                    if ((birth_sequence[0] == 1) &&
                            (birth_sequence[1] == 1)) {
                        decode_strcat(gs1_result, "ONE BABY");
                    }
                    else if ((birth_sequence[0] == 1) &&
                             (birth_sequence[1] == 2)) {
                        decode_strcat(gs1_result, "TWIN ONE");
                    }
                    else if ((birth_sequence[0] == 2) &&
                             (birth_sequence[1] == 2)) {
                        decode_strcat(gs1_result, "TWIN TWO");
                    }
                    else if ((birth_sequence[0] == 1) &&
                             (birth_sequence[1] == 3)) {
                        decode_strcat(gs1_result, "TRIPLET ONE");
                    }
                    else if ((birth_sequence[0] == 2) &&
                             (birth_sequence[1] == 3)) {
                        decode_strcat(gs1_result, "TRIPLET TWO");
                    }
                    else if ((birth_sequence[0] == 3) &&
                             (birth_sequence[1] == 3)) {
                        decode_strcat(gs1_result, "TRIPLET THREE");
                    }
                }
                else if ((itip_piece_number_str[0] != 0) &&
                         (itip_total_count_str[0] != 0)) {
                    decode_strcat(gs1_result, data_str);
                    decode_strcat_char(gs1_result, '\n');
                    decode_strcat(gs1_result, "PIECE NUM: ");
                    decode_strcat(gs1_result, &itip_piece_number_str[0]);
                    decode_strcat_char(gs1_result, '\n');
                    decode_strcat(gs1_result, "TOTAL COUNT: ");
                    decode_strcat(gs1_result, &itip_total_count_str[0]);
                }
                else if (bio_sex != -1) {
                    switch(bio_sex) {
                    case 1: {
                        decode_strcat(gs1_result, "MALE");
                        break;
                    }
                    case 2: {
                        decode_strcat(gs1_result, "FEMALE");
                        break;
                    }
                    case 9: {
                        decode_strcat(gs1_result, "NOT APPLICABLE");
                        break;
                    }
                    }
                    if ((bio_sex != 1) && (bio_sex != 2) && (bio_sex != 9)) {
                        decode_strcat(gs1_result, "NOT KNOWN");
                    }
                }
                else if (package_type_str != NULL) {
                    decode_strcat(gs1_result, package_type_str);
                }
                else {
                    decode_strcat(gs1_result, data_str);
                }
                decode_strcat_char(gs1_result, '\n');

                /* show the status of a GTIN check digit */
                if (gtin_check_digit_passed == 0) {
                    decode_strcat(gs1_result, "GTIN CHECK DIGIT: FAIL\n");
                }
                else if (gtin_check_digit_passed == 1) {
                    decode_strcat(gs1_result, "GTIN CHECK DIGIT: PASS\n");
                }

                /* show the status of a GSIN check digit */
                if (gsin_check_digit_passed == 0) {
                    decode_strcat(gs1_result, "GSIN CHECK DIGIT: FAIL\n");
                }
                else if (gsin_check_digit_passed == 1) {
                    decode_strcat(gs1_result, "GSIN CHECK DIGIT: PASS\n");
                }

                /* show the status of a GSRN check digit */
                if (gsrn_check_digit_passed == 0) {
                    decode_strcat(gs1_result, "GSRN CHECK DIGIT: FAIL\n");
                }
                else if (gsrn_check_digit_passed == 1) {
                    decode_strcat(gs1_result, "GSRN CHECK DIGIT: PASS\n");
                }

                /* show the status of a GLN check digit */
                if (gln_check_digit_passed == 0) {
                    decode_strcat(gs1_result, "GLN CHECK DIGIT: FAIL\n");
                }
                else if (gln_check_digit_passed == 1) {
                    decode_strcat(gs1_result, "GLN CHECK DIGIT: PASS\n");
                }

                /* show the status of a GRAI check digit */
                if (grai_check_digit_passed == 0) {
                    decode_strcat(gs1_result, "GRAI CHECK DIGIT: FAIL\n");
                }
                else if (grai_check_digit_passed == 1) {
                    decode_strcat(gs1_result, "GRAI CHECK DIGIT: PASS\n");
                }

                /* show the status of a SSCC check digit */
                if (sscc_check_digit_passed == 0) {
                    decode_strcat(gs1_result, "SSCC CHECK DIGIT: FAIL\n");
                }
                else if (sscc_check_digit_passed == 1) {
                    decode_strcat(gs1_result, "SSCC CHECK DIGIT: PASS\n");
                }
                /* show the SSCC extension digit */
                if (sscc_package_type != ' ') {
                    if (sscc_package_type == '0') {
                        decode_strcat(gs1_result,
                                      "SSCC EXTENSION DIGIT: 0 (CARTON)\n");
                    }
                    else {
                        decode_strcat(gs1_result, "SSCC EXTENSION DIGIT: ");
                        decode_strcat_char(gs1_result, sscc_package_type);
                        decode_strcat(gs1_result, "\n");
                    }
                }
            }
            if (debug == 1) {
                printf("| (%d)%s | ",
                       *application_identifier,
                       &result[*application_data_start]);
            }
        }
        *application_identifier = 0;
    }
    *application_data_start = curr_pos;
}
