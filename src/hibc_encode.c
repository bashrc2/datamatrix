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
 * \brief encodes a HIBC application identifier into the given encoded string
 * \param application_identifier Application identifier string
 * \param data_str Data to be used with the application identifier
 * \param encode_text Returned encoded string
 * \param encode_description Returned description shown beneath the datamatrix
 * \return zero on success
 */
int hibc_encode(char * application_identifier, char data_str[],
                char encode_text[], char encode_description[])
{
    int data_len = (int)strlen(data_str);

    if ((int)strlen(encode_text) == 0) {
        decode_strcat_char(encode_text, '+');
    }

    if (strcmp(application_identifier, "LABELER ID") == 0) {
        if (data_len == 4) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, data_str);

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
        }
        else {
            printf("LABELER ID should be 4 characters\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "PRODUCT ID") == 0) {
        if (data_len == 4) {
            if (strstr("LABELER ID", encode_description) != 0) {
                printf("LABELER ID should have been set before PRODUCT ID\n");
                return 1;
            }
            decode_strcat(encode_text, data_str);

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
        }
        else {
            printf("PRODUCT ID should be 4 characters\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "UNIT OF MEASURE") == 0) {
        if (data_len == 1) {
            if (strstr("PRODUCT ID", encode_description) != 0) {
                printf("PRODUCT ID should have been set before UNIT OF MEASURE\n");
                return 1;
            }
            decode_strcat(encode_text, data_str);

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
        }
        else {
            printf("UNIT OF MEASURE should be 1 character\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "EXPIRY") == 0) {
        if (data_len == 5) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "$$5");
            decode_strcat(encode_text, data_str);

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
        }
        else {
            printf("EXPIRY should be 5 characters YYDDD\n");
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

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
        }
        else {
            printf("EXPIRATION DATE should be 8 characters YYYYMMDD\n");
            return 1;
        }
    }
    else if (strcmp(application_identifier, "LOT NUMBER") == 0) {
        if (data_len == 4) {
            if ((int)strlen(encode_text) > 1) {
                decode_strcat_char(encode_text, '/');
            }
            decode_strcat(encode_text, "$");
            decode_strcat(encode_text, data_str);

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
        }
        else {
            printf("LOT NUMBER should be 4 characters\n");
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

            decode_strcat(encode_description, application_identifier);
            decode_strcat(encode_description, ": ");
            decode_strcat(encode_description, data_str);
            decode_strcat_char(encode_description, '\n');
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

        decode_strcat(encode_description, application_identifier);
        decode_strcat(encode_description, ": ");
        decode_strcat(encode_description, data_str);
        decode_strcat_char(encode_description, '\n');
    }
    else if (strcmp(application_identifier, "QUANTITY") == 0) {
        if ((data_len > 0) && (data_len < 6)) {
			if ((int)strlen(encode_text) > 1) {
				decode_strcat_char(encode_text, '/');
			}
			decode_strcat(encode_text, "Q");
			decode_strcat(encode_text, data_str);

			decode_strcat(encode_description, application_identifier);
			decode_strcat(encode_description, ": ");
			decode_strcat(encode_description, data_str);
			decode_strcat_char(encode_description, '\n');
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
