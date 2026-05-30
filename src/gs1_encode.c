/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  GS1 encoding
 *  Copyright (c) 2026, Bob Mottram
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
 * \brief encodes a GS1 application identifier into the given encoded string
 * \param application_identifier Application identifier number
 * \param data_str Data to be used with the application identifier
 * \param encoded_text Returned encoded string
 * \return zero on success
 */
int gs1_encode(int application_identifier, char data_str[],
               char encode_text[])
{
    char app_id_str[16];
    int data_len = (int)strlen(data_str);

    /* convert the application identifier to a string */
    app_id_str[0] = 0;
    if (application_identifier >= 10) {
        sprintf(&app_id_str[0], "%d", application_identifier);
    }
    else {
        sprintf(&app_id_str[0], "0%d", application_identifier);
    }

    switch(application_identifier){
    case 0: { /* SSCC */
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        int check_digit = get_gtin_check_digit(data_str, 0);
        decode_strcat_char(encode_text, '0' + check_digit);
        break;
    }
    case 1: { /* GTIN */
        if (data_len > 14) {
            printf("GTIN should contain no more than 14 digits\n");
            return -1;
        }
        else if (data_len < 8) {
            printf("GTIN should contain no less than 8 digits\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        /* pad with zeros */
        for (int i = 0; i < 14 - data_len;  i++) {
            decode_strcat_char(encode_text, '0');
        }
        decode_strcat(encode_text, data_str);
        break;
    }
    case 2: { /* CONTENT */
        if (data_len > 13) {
            printf("CONTENT should contain no more than 13 characters\n");
            return -1;
        }
        else if (data_len < 8) {
            printf("CONTENT should contain no less than 8 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        /* pad with zeros */
        for (int i = 0; i < 13 - data_len;  i++) {
            decode_strcat_char(encode_text, '0');
        }
        decode_strcat(encode_text, data_str);
        int check_digit = get_gtin_check_digit(data_str, 0);
        decode_strcat_char(encode_text, '0' + check_digit);
        break;
    }
    case 3: { /* MTO GTIN */
        if (data_len < 13) {
            printf("MTO GTIN should contain at least 13 digits\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        /* pad with zeros */
        for (int i = 0; i < 14 - data_len;  i++) {
            decode_strcat_char(encode_text, '0');
        }
        decode_strcat(encode_text, data_str);
        break;
    }
    case 10: { /* BATCH/LOT */
        if (data_len > 20) {
            printf("BATCH/LOT should contain no more than 20 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        break;
    }
    case 11: { /* PROD DATE */
        if (data_len != 6) {
            printf("PROD DATE should contain 6 digits YYMMDD\n");
            return -1;
        }
        for (int i = 0; i < 6;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within PROD DATE\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        break;
    }
    case 12: { /* DUE DATE */
        if (data_len != 6) {
            printf("DUE DATE should contain 6 digits YYMMDD\n");
            return -1;
        }
        for (int i = 0; i < 6;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within DUE DATE\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        break;
    }
    default: {
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
    }
    }

    return 0;
}
