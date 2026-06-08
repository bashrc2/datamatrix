/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  HIBC encoding
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
 * \brief Returns the HIBC check character for the given text
 * \param encode_text Encoded HIBC string
 * \return Check character
 */
char hibc_check_character(char * encode_text)
{
    const char * lookup = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";
    int sum = 0;

    for (int i = 0; i < (int)strlen(encode_text); i++) {
        for (int j = 0; j < (int)strlen(lookup); j++) {
            if (encode_text[i] == lookup[j]) {
                sum += j;
                break;
            }
        }
    }
    return lookup[sum % 43];
}

/**
 * \brief encodes a HIBC application identifier into the given encoded string
 * \param application_identifier Application identifier string
 * \param data_str Data to be used with the application identifier
 * \param encode_text Returned encoded string
 * \return zero on success
 */
int hibc_encode(char * application_identifier, char data_str[],
                char encode_text[])
{
    int data_len = (int)strlen(data_str);

    if ((int)strlen(encode_text) == 0) {
        decode_strcat_char(encode_text, '+');
    }

    if (strcmp(application_identifier, "LABELER ID") == 0) {
        if (data_len == 4) {
            if ((data_str[0] >= 'A') && (data_str[0] <= 'Z')) {
                if ((int)strlen(encode_text) > 1) {
                    decode_strcat_char(encode_text, '/');
                }
                decode_strcat(encode_text, data_str);
            }
            else {
                printf("LABELER ID first character should be a letter A-Z\n");
                return 1;
            }
        }
        else {
            printf("LABELER ID should be 4 characters\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "PRODUCT ID") == 0) {
        if ((data_len >= 1) && (data_len <= 18)) {
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("PRODUCT ID should be 1-18 characters\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "UNIT OF MEASURE") == 0) {
        if (data_len == 1) {
            if ((data_str[0] >= '0') && (data_str[0] <= '9')) {
                decode_strcat(encode_text, data_str);
            }
            else {
                printf("UNIT OF MEASURE should be a number\n");
                return 1;
            }
        }
        else {
            printf("UNIT OF MEASURE should be 1 digit\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "EXPIRY JULIAN") == 0) {
        if (data_len == 5) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "$$5");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("EXPIRY JULIAN should be 5 digits YYJJJ\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "EXPIRY") == 0) {
        if (data_len == 6) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "$$3");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("EXPIRY should be 6 digits YYMMDD\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "EXPIRY HOUR") == 0) {
        if (data_len == 8) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "$$4");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("EXPIRY HOUR should be 8 digits YYMMDDHH\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "EXPIRY JULIAN HOUR") == 0) {
        if (data_len == 7) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "$$6");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("EXPIRY HOUR should be 7 digits YYJJJHH\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "EXPIRATION DATE") == 0) {
        if (data_len == 8) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "14D");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("EXPIRATION DATE should be 8 digits YYYYMMDD\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "LOT NUMBER") == 0) {
        if ((data_len >= 1) && (data_len <= 18)) {
            if (strstr("/$$", encode_text) != 0) {
                if ((int)strlen(encode_text) > 1) {
                    decode_strcat_char(encode_text, '/');
                }
                decode_strcat(encode_text, "$");
            }
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("LOT NUMBER should be 1-18 characters\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "MANUFACTURE DATE") == 0) {
        if (data_len == 8) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "16D");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("MANUFACTURE DATE should be 8 characters YYYYMMDD\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "SUPPLIER SERIAL NUMBER") == 0) {
        if ((int)strlen(encode_text) > 1) {
            decode_strcat_char(encode_text, '/');
        }
        decode_strcat(encode_text, "S");
        decode_strcat(encode_text, data_str);
    }
    else if (strcmp(application_identifier, "QUANTITY") == 0) {
        if ((data_len > 0) && (data_len < 6)) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "Q");
            decode_strcat(encode_text, data_str);
        }
        else {
            printf("QUANTITY should be 1-5 characters\n");
            return 1;
        }
    }
    else {
        return 1;
    }

    return 0;
}
