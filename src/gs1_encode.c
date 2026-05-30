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
 * \param encode_text Returned encoded string
 * \param encode_description Returned description shown beneath the datamatrix
 * \return zero on success
 */
int gs1_encode(int application_identifier, char data_str[],
               char encode_text[], char encode_description[])
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

    decode_strcat(encode_description, "  (");
    decode_strcat(encode_description, &app_id_str[0]);
    decode_strcat(encode_description, ") ");

    switch(application_identifier){
    case 0: { /* SSCC */
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        int check_digit = get_gtin_check_digit(data_str, 0);
        decode_strcat_char(encode_text, '0' + check_digit);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '0' + check_digit);
        decode_strcat_char(encode_description, '\n');
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
            decode_strcat_char(encode_description, '0');
        }
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
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
            decode_strcat_char(encode_description, '0');
        }
        decode_strcat(encode_text, data_str);
        int check_digit = get_gtin_check_digit(data_str, 0);
        decode_strcat_char(encode_text, '0' + check_digit);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '0' + check_digit);
        decode_strcat_char(encode_description, '\n');
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
            decode_strcat_char(encode_description, '0');
        }
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 10: { /* BATCH/LOT */
        if (data_len > 20) {
            printf("BATCH/LOT should contain no more than 20 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
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

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
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

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 13: { /* PACK DATE */
        if (data_len != 6) {
            printf("PACK DATE should contain 6 digits YYMMDD\n");
            return -1;
        }
        for (int i = 0; i < 6;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within PACK DATE\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 15: { /* BEST BEFORE */
        if (data_len != 6) {
            printf("BEST BEFORE should contain 6 digits YYMMDD\n");
            return -1;
        }
        for (int i = 0; i < 6;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within BEST BEFORE\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 16: { /* SELL BY */
        if (data_len != 6) {
            printf("SELL BY should contain 6 digits YYMMDD\n");
            return -1;
        }
        for (int i = 0; i < 6;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within SELL BY\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 17: { /* USE BY */
        if (data_len != 6) {
            printf("USE BY should contain 6 digits YYMMDD\n");
            return -1;
        }
        for (int i = 0; i < 6;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within USE BY\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 20: { /* VARIANT */
        if (data_len != 6) {
            printf("VARIANT should contain 2 digits\n");
            return -1;
        }
        for (int i = 0; i < 2;  i++) {
            if ((data_str[i] < '0') || (data_str[i] > '9')) {
                printf("Only numbers are permitted within VARIANT\n");
                return -1;
            }
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 21: { /* SERIAL */
        if (data_len > 20) {
            printf("SERIAL should contain no more than 20 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 241: { /* CUST PART NO */
        if (data_len > 30) {
            printf("CUST PART NO should contain no more than 30 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 242: { /* MTO VARIANT */
        if (data_len > 6) {
            printf("MTO VARIANT should contain no more than 6 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 253: { /* GDTI */
        if (data_len != 12) {
            printf("GDTI should contain 12 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 254: { /* GLN */
        if (data_len > 20) {
            printf("GLN should contain no more than 20 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 255: { /* GCN */
        if (data_len != 12) {
            printf("GCN should contain 12 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 400: { /* ORDER NUMBER */
        if (data_len > 30) {
            printf("ORDER NUMBER should contain no more than 30 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 402: { /* GSIN */
        if (data_len != 16) {
            printf("GSIN should contain 16 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        int check_digit = get_gtin_check_digit(data_str, 0);
        decode_strcat_char(encode_text, '0' + check_digit);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '0' + check_digit);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    case 410: { /* SHIP TO LOC */
        if (data_len != 12) {
            printf("SHIP TO LOC should contain 12 characters\n");
            return -1;
        }
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);
        int check_digit = get_gtin_check_digit(data_str, 0);
        decode_strcat_char(encode_text, '0' + check_digit);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '0' + check_digit);
        decode_strcat_char(encode_description, '\n');
        break;
    }
    default: {
        decode_strcat(encode_text, &app_id_str[0]);
        decode_strcat(encode_text, data_str);

        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
    }
    }

    return 0;
}
