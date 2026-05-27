/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Various utility functionf for processing GS1 semantics
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
#include "gs1_lookup_tables.h"

/* the number of items (rows) in a lookup table */
#define LOOKUP_TABLE_ROWS(lookup_table, columns) \
    ((int)sizeof(lookup_table) / (int)sizeof(lookup_table[0]))/columns

/**
 * \brief returns a check digit for the given GTIN
 * \param gtin GTIN code number as a string
 * \param includes_check_digit Non zero if the given GTIN includes the check digit at the end
 * \return check digit in the range 0-9
 */
int get_gtin_check_digit(char gtin[], unsigned char includes_check_digit) {
    int i, even_sum = 0, odd_sum = 0;
    char num_str[2];
    int gtin_len = (int)strlen(gtin);

    if (includes_check_digit != 0) gtin_len--;

    num_str[1] = 0;
    for (i = 0; i < gtin_len; i++) {
        if ((gtin[i] < '0') || (gtin[i] > '9')) return -1;
        num_str[0] = gtin[i];
        int num = atoi(num_str);
        if (i % 2 == 0) {
            /* even */
            even_sum += num;
        }
        else {
            /* odd */
            odd_sum += num;
        }
    }
    int total = (odd_sum*3) + even_sum;
    int remainder = total % 10;
    if (remainder == 0) return 0;
    return 10 - remainder;
}

/**
 * \brief returns a temperature with two decimal places
 * \param data_str String to be decoded
 * \return temperature value
 */
float get_temperature(char data_str[])
{
    int i, ctr=0, data_len = strlen(data_str);
    char * temp_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    for (i = 0; i < data_len; i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) {
            free(temp_str);
            return UNKNOWN_VALUE;
        }
        if (i == data_len - 2) temp_str[ctr++] = '.';
        temp_str[ctr++] = data_str[i];
    }
    temp_str[ctr] = 0;
    float temperature = atof(&temp_str[0]);
    free(temp_str);
    return temperature;
}

/**
 * \brief returns human readable details for an ISSN
 * \param data_str String to be decoded
 * \return decoded ISSN string or NULL
 */
char * get_issn(char data_str[])
{
    int i, data_len = strlen(data_str);
    if (data_len < 10) return NULL;
    char * issn_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    issn_str[0] = 0;
    decode_strcat(issn_str, "ISSN: ");
    for (i = 0; i < 4; i++) {
        decode_strcat_char(issn_str, data_str[i]);
    }
    decode_strcat_char(issn_str, '-');
    for (i = 4; i < 7; i++) {
        decode_strcat_char(issn_str, data_str[i]);
    }
    decode_strcat_char(issn_str, data_str[9]);
    decode_strcat_char(issn_str, '\n');
    decode_strcat(issn_str, "VARIANT: ");
    decode_strcat_char(issn_str, data_str[7]);
    decode_strcat_char(issn_str, data_str[8]);
    if (data_len > 9) {
        decode_strcat_char(issn_str, '\n');
        decode_strcat(issn_str, "CHECK DIGIT: ");
        decode_strcat_char(issn_str, data_str[9]);
    }
    return issn_str;
}

/**
 * \brief Returns the package type description
 * \param package_code Package code
 * \return package description
 */
char * get_package_type(char package_code[])
{
    int i;
    if ((int)strlen(package_code) > 3) return NULL;
    int no_of_package_types = LOOKUP_TABLE_ROWS(package_type_code, 2);
    int data_len = strlen(package_code);
    for (i = 0; i < no_of_package_types; i++) {
        if ((int)strlen(package_type_code[i*2]) != data_len) continue;
        if (strstr(package_type_code[i*2], package_code) == 0) {
            return package_type_code[i*2 + 1];
        }
    }
    return NULL;
}

/**
 * \brief returns the GS1 Company Prefix for the given code number
 * \param data_str String containing a code number to be decoded
 * \return decoded Company Prefix or NULL
 */
char * get_gs1_company_prefix(char data_str[])
{
    int i;
    char data_str_code[16];
    unsigned char code_range = 0;

    int data_len = strlen(data_str);
    if ((data_len < 2) || (data_len > 15)) return NULL;
    for (i = 0; i < data_len; i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) return NULL;
        data_str_code[i] = data_str[i];
    }
    data_str_code[i] = 0;

    /* is this a GS1 company prefix? */
    int no_of_company_prefixes = LOOKUP_TABLE_ROWS(gs1_company_prefix, 3);
    int company_prefix_index = -1;
    int test_code = atoi(data_str_code);
    for (i = 0; i < no_of_company_prefixes; i++) {
        if ((int)strlen(gs1_company_prefix[i*3]) != data_len) continue;
        int min_code = atoi(gs1_company_prefix[i*3]);
        int max_code = min_code;
        code_range = 0;
        if (strlen(gs1_company_prefix[i*3 + 1]) > 0) {
            max_code = atoi(gs1_company_prefix[i*3 + 1]);
            code_range = 1;
        }
        if ((test_code >= min_code) && (test_code <= max_code)) {
            company_prefix_index = i;
            break;
        }
    }
    if (company_prefix_index == -1) return NULL;

    char * company_prefix_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    company_prefix_str[0] = 0;
    decode_strcat(company_prefix_str,
                  gs1_company_prefix[company_prefix_index*3+2]);

    if (code_range == 1) {
        decode_strcat(company_prefix_str, ", Code ");
        for (i = 0; i < data_len; i++) {
            decode_strcat_char(company_prefix_str, data_str[i]);
        }
    }

    return company_prefix_str;
}

/**
 * \brief returns the company prefix
 * \param company_prefix_code GS1 company prefix code number
 * \param start_index Start index of the data
 * \param data_str String containing data
 */
char * get_company_prefix_str(char company_prefix_code[],
                              int start_index, char data_str[])
{
    company_prefix_code[0] = data_str[start_index];
    company_prefix_code[1] = data_str[start_index+1];
    company_prefix_code[2] = data_str[start_index+2];
    company_prefix_code[3] = 0;
    return get_gs1_company_prefix(company_prefix_code);
}

/**
 * \brief returns the country for the given 3 digit code number
 * \param data_str String containing a code number to be decoded
 * \return decoded country string or NULL
 */
char * get_country(char data_str[])
{
    int i;
    char data_str_country_code[4];

    int data_len = strlen(data_str);
    if (data_len < 3) return NULL;
    for (i = 0; i < 3; i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) return NULL;
        data_str_country_code[i] = data_str[i];
    }
    data_str_country_code[3] = 0;

    /* is this an iso3166 code? */
    int no_of_countries = LOOKUP_TABLE_ROWS(iso3166_country_codes, 3);
    int country_index = -1;
    for (i = 0; i < no_of_countries; i++) {
        if (strcmp(iso3166_country_codes[i*3 + 2],
                   &data_str_country_code[0]) == 0) {
            country_index = i;
            break;
        }
    }
    if (country_index == -1) return NULL;

    char * country_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    country_str[0] = 0;
    decode_strcat(country_str, iso3166_country_codes[country_index*3]);

    if (data_len > 3) {
        decode_strcat_char(country_str, ' ');
        for (i = 3; i < data_len; i++) {
            decode_strcat_char(country_str, data_str[i]);
        }
    }

    return country_str;
}

/**
 * \brief returns the country for the given two letter alphabetic code
 *        https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
 * \param data_str String containing a code number to be decoded
 * \return decoded country string or NULL
 */
char * get_country_alpha2(char data_str[])
{
    int i;
    char data_str_country_code[3];

    int data_len = strlen(data_str);
    if (data_len != 2) return NULL;
    for (i = 0; i < 2; i++) {
        if ((data_str[i] < 'A') || (data_str[i] > 'Z')) return NULL;
        data_str_country_code[i] = data_str[i];
    }
    data_str_country_code[2] = 0;

    /* is this an iso3166 code? */
    int no_of_countries = LOOKUP_TABLE_ROWS(iso3166_2_country_codes, 2);
    int country_index = -1;
    for (i = 0; i < no_of_countries; i++) {
        if (strcmp(iso3166_2_country_codes[i*2],
                   &data_str_country_code[0]) == 0) {
            country_index = i;
            break;
        }
    }
    if (country_index == -1) return NULL;

    char * country_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    country_str[0] = 0;
    decode_strcat(country_str, iso3166_2_country_codes[country_index*2+1]);

    return country_str;
}

/**
 * \brief returns decoded human readable currency
 * \param application_identifier GS1 application identifier
 * \param data_str String to be decoded
 * \return decoded currency string or NULL
 */
char * get_currency_value(int application_identifier,
                          char data_str[])
{
    int i;
    char data_str_currency_code[4];

    if (application_identifier < 3910) return NULL;
    if (strlen(data_str) < 4) return NULL;
    int decimal_places = application_identifier % 10;
    if (decimal_places > 2) return NULL;
    /* check that the data is all numeric */
    int data_len = strlen(data_str);
    for (i = 0; i < data_len; i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) return NULL;
        if (i < 3) {
            /* get the currency code */
            data_str_currency_code[i] = data_str[i];
        }
    }
    data_str_currency_code[3] = 0;

    /* is this an iso4217 code? */
    int no_of_currencies = LOOKUP_TABLE_ROWS(iso4217_currency_codes, 4);
    int currency_index = -1;
    for (i = 0; i < no_of_currencies; i++) {
        if (strcmp(iso4217_currency_codes[i*4 + 3],
                   &data_str_currency_code[0]) == 0) {
            currency_index = i;
            break;
        }
    }
    if (currency_index == -1) return NULL;

    char * currency_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    currency_str[0] = 0;

    for (i = 3; i < data_len; i++) {
        decode_strcat_char(currency_str, data_str[i]);
        if (i == data_len - 1 - decimal_places) {
            decode_strcat_char(currency_str, '.');
        }
    }
    for (i = 0; i < 2 - decimal_places; i++) {
        decode_strcat_char(currency_str, '0');
    }
    decode_strcat_char(currency_str, ' ');
    decode_strcat(currency_str, iso4217_currency_codes[currency_index*4 + 1]);
    return currency_str;
}

/**
 * \brief returns decoded human readable decimal value
 * \param application_identifier GS1 application identifier
 * \param data_str String to be decoded
 * \return decoded decimal string or NULL
 */
char * get_decimal_value(int application_identifier,
                         char data_str[])
{
    int i;

    if (application_identifier < 3100) return NULL;
    if (strlen(data_str) < 1) return NULL;
    int decimal_places = application_identifier % 10;
    if (decimal_places > 3) return NULL;
    /* check that the data is all numeric */
    int data_len = strlen(data_str);
    for (i = 0; i < data_len; i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) return NULL;
    }

    char * decimal_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    decimal_str[0] = 0;

    for (i = 0; i < data_len; i++) {
        decode_strcat_char(decimal_str, data_str[i]);
        if (i == data_len - 1 - decimal_places) {
            decode_strcat_char(decimal_str, '.');
        }
    }
    for (i = 0; i < 2 - decimal_places; i++) {
        decode_strcat_char(decimal_str, '0');
    }
    return decimal_str;
}

/**
 * \brief returns UNECE meat carcasses and cuts code
 *        https://unece.org/sites/default/files/2024-03/Bovine_2007_e_0.pdf
 * \param data_str String to be decoded
 * \return decoded UNECE meat carcasses and cuts code
 */
char * get_meat_cut(char data_str[])
{
    int i, data_len = strlen(data_str);
    if (data_len < 20) return NULL;
    char * meat_cut_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    meat_cut_str[0] = 0;
    decode_strcat(meat_cut_str, "MEAT CUT: ");
    decode_strcat(meat_cut_str, data_str);

    /* field 1 */
    int no_of_species = LOOKUP_TABLE_ROWS(unece_species_code, 2);
    for (i = 0; i < no_of_species; i++) {
        if ((unece_species_code[i*2][0] == data_str[0]) &&
                (unece_species_code[i*2][1] == data_str[1])) {
            decode_strcat(meat_cut_str, "\nSPECIES: ");
            decode_strcat(meat_cut_str, unece_species_code[i*2 + 1]);
            break;
        }
    }

    /* field 2 */
    int no_of_product_codes = LOOKUP_TABLE_ROWS(unece_product_code, 3);
    char prod_code_str[5];
    /* check that all product code characters are numbers */
    for (i = 2; i <= 5; i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) {
            return NULL;
        }
    }
    prod_code_str[0] = data_str[2];
    prod_code_str[1] = data_str[3];
    prod_code_str[2] = data_str[4];
    prod_code_str[3] = data_str[5];
    prod_code_str[4] = 0;
    int prod_code = atoi(prod_code_str);
    for (i = 0; i < no_of_product_codes; i++) {
        int prod_code_start = atoi(unece_product_code[i*3]);
        if ((int)strlen(unece_product_code[i*3+1]) > 0) {
            int prod_code_end = atoi(unece_product_code[i*3+1]);
            if ((prod_code >= prod_code_start) &&
                    (prod_code <= prod_code_end)) {
                decode_strcat(meat_cut_str, "\nPROD CODE: ");
                decode_strcat(meat_cut_str, unece_product_code[i*3+2]);
                break;
            }
        }
        else {
            if (prod_code == prod_code_start) {
                decode_strcat(meat_cut_str, "\nPROD CODE: ");
                decode_strcat(meat_cut_str, unece_product_code[i*3+2]);
                break;
            }
        }
    }

    /* field 3 is 2 characters and not used */

    /* field 4 */
    switch(data_str[8]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nREFRIGERATION: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nREFRIGERATION: CHILLED");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nREFRIGERATION: FROZEN");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nREFRIGERATION: DEEP-FROZEN");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nREFRIGERATION: OTHER");
        break;
    }
    }

    /* field 5 */
    switch(data_str[9]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: INTACT MALE");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: YOUNG INTACT MALE");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: STEER");
        break;
    }
    case '4': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: HEIFER");
        break;
    }
    case '5': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: STEER AND/OR HEIFER");
        break;
    }
    case '6': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: COW");
        break;
    }
    case '7': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: YOUNG BOVINE");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nBOVINE CATEGORY: OTHER");
        break;
    }
    }

    /* field 6 */
    switch(data_str[10]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nPRODUCTION SYSTEM: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nPRODUCTION SYSTEM: INTENSIVE");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nPRODUCTION SYSTEM: EXTENSIVE");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nPRODUCTION SYSTEM: ORGANIC");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nPRODUCTION SYSTEM: OTHER");
        break;
    }
    }

    /* field 7a */
    switch(data_str[11]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nFEEDING SYSTEM: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nFEEDING SYSTEM: GRAIN FED");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nFEEDING SYSTEM: FORAGE FED");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nFEEDING SYSTEM: EXCLUSIVELY FORAGE FED");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nFEEDING SYSTEM: OTHER");
        break;
    }
    }

    /* field 7b not used */

    /* field 8 */
    switch(data_str[13]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nSLAUGHTER SYSTEM: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nSLAUGHTER SYSTEM: CONVENTIONAL");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nSLAUGHTER SYSTEM: KOSHER");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nSLAUGHTER SYSTEM: HALAL");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nSLAUGHTER SYSTEM: OTHER");
        break;
    }
    }

    /* field 9 */
    switch(data_str[14]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nPOST-SLAUGHTER PROCESSING: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nPOST-SLAUGHTER PROCESSING: SPECIFIED");
        break;
    }
    }

    /* field 10 */
    switch(data_str[15]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: PEELED, DENUDED, SURFACE MEMBRANE REMOVED");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: PEELED, DENUDED");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: PRACTICALLY FREE");
        break;
    }
    case '4': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: 3MM");
        break;
    }
    case '5': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: 6MM");
        break;
    }
    case '6': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: 13MM");
        break;
    }
    case '7': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: 25MM");
        break;
    }
    case '8': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: CHEMICAL LEAN SPECIFIED");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nFAT THICKNESS: OTHER");
        break;
    }
    }

    /* field 11 */
    switch(data_str[16]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nBOVINE QUALITY SYSTEM: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nBOVINE QUALITY SYSTEM: OFFICIAL STANDARDS");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nBOVINE QUALITY SYSTEM: COMPANY STANDARDS");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nBOVINE QUALITY SYSTEM: INDUSTRY STANDARDS");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nBOVINE QUALITY SYSTEM: OTHER");
        break;
    }
    }

    /* field 12 */
    switch(data_str[17]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nWEIGHT RANGE: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nWEIGHT RANGE: SPECIFIED");
        break;
    }
    }

    /* field 13 */
    switch(data_str[18]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nPACKING: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nPACKING: CARCASES, HALF CARCASES AND QUARTERS - WITHOUT PACKING");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nPACKING: CARCASES, HALF CARCASES AND QUARTERS - WITH PACKING");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nPACKING: CUTS - INDIVIDUALLY WRAPPED");
        break;
    }
    case '4': {
        decode_strcat(meat_cut_str, "\nPACKING: CUTS - BULK PACKAGED");
        break;
    }
    case '5': {
        decode_strcat(meat_cut_str, "\nPACKING: CUTS - VACUUM PACKED");
        break;
    }
    case '6': {
        decode_strcat(meat_cut_str, "\nPACKING: CUTS - MODIFIED ATMOSPHERE PACKAGING");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nPACKING: OTHER");
        break;
    }
    }

    /* field 14 */
    switch(data_str[19]) {
    case '0': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: NOT SPECIFIED");
        break;
    }
    case '1': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: QUALITY/GRADE/CLASSIFICATION");
        break;
    }
    case '2': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: TRADE STANDARD");
        break;
    }
    case '3': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: BOVINE/BATCH ID");
        break;
    }
    case '4': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: QUALITY AND TRADE");
        break;
    }
    case '5': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: QUALITY AND BOVINE/BATCH ID");
        break;
    }
    case '6': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: TRADE STANDARD AND BOVINE/BATCH ID");
        break;
    }
    case '7': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: QUALITY, TRADE STANDARD AND BOVINE/BATCH ID");
        break;
    }
    case '9': {
        decode_strcat(meat_cut_str, "\nCONFORMITY: OTHER");
        break;
    }
    }

    return meat_cut_str;
}

/**
 * \brief returns data from a North American coupon
 *        see https://www.gs1us.org/content/dam/gs1us/documents/
 *        industries-insights/standards/
 *        North-American-Coupon-Application-Guideline-Using-GS1-DataBar.pdf
 * \param data_str String to be decoded
 * \param company_prefix_code array used for company prefix
 * \return Decoded string representing the coupon
 */
char * get_north_american_coupon(char data_str[],
                                 char company_prefix_code[])
{
    if ((data_str[0] < '0') || (data_str[0] > '9')) return NULL;

    char * coupon_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    coupon_str[0] = 0;

    /* variable length indicator */
    char vli_str[2];
    vli_str[0] = data_str[0];
    vli_str[1] = 0;
    int vli = 6 + atoi(&vli_str[0]);
    int coupon_start_index = 1;
    if (data_str[1] == '0') {
        coupon_start_index = 2;
    }
    decode_strcat(coupon_str, "COUPON: ");
    decode_strcat(coupon_str, data_str);

    /* GS1 company prefix */
    char * company_prefix_str =
        get_company_prefix_str(company_prefix_code,
                               coupon_start_index,
                               data_str);
    if (company_prefix_str == NULL) {
        return coupon_str;
    }
    int company_ctr = 0;
    decode_strcat(coupon_str, "\nCOMPANY: ");
    for (company_ctr = 1; company_ctr < 1 + vli; company_ctr++) {
        decode_strcat_char(coupon_str, data_str[company_ctr]);
    }
    decode_strcat(coupon_str, "\nCOUNTRY: ");
    decode_strcat(coupon_str, company_prefix_str);
    free(company_prefix_str);

    /* offer code */
    int idx = 1 + vli;
    if ((int)strlen(data_str) < idx + 7) {
        return coupon_str;
    }
    char offer_code_str[7];
    int offer_ctr = 0;
    offer_code_str[0] = 0;
    for(offer_ctr = 0; offer_ctr < 6; offer_ctr++, idx++) {
        offer_code_str[offer_ctr] = data_str[idx];
    }
    offer_code_str[offer_ctr] = 0;
    decode_strcat(coupon_str, "\nOFFER CODE: ");
    decode_strcat(coupon_str, &offer_code_str[0]);

    /* save value variable length indicator */
    if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
        return coupon_str;
    }
    vli_str[0] = data_str[idx++];
    int save_value_vli = atoi(&vli_str[0]);
    int save_value = 0;
    if (save_value_vli > 0) {
        if (save_value_vli > 5) save_value_vli = 5;

        if ((int)strlen(data_str) <= idx + save_value_vli) {
            return coupon_str;
        }

        /* save value */
        char save_value_str[6];
        int save_value_ctr = 0;
        for(save_value_ctr = 0;
                save_value_ctr < save_value_vli; save_value_ctr++, idx++) {
            save_value_str[save_value_ctr] = data_str[idx];
        }
        save_value_str[save_value_ctr] = 0;
        decode_strcat(coupon_str, "\nSAVE VALUE: ");
        decode_strcat(coupon_str, &save_value_str[0]);
        save_value = atoi(&save_value_str[0]);
    }

    /* primary purchase requirement variable length indicator */
    if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
        return coupon_str;
    }
    vli_str[0] = data_str[idx++];
    int primary_purchase_requirement_vli = atoi(&vli_str[0]);
    if (primary_purchase_requirement_vli > 0) {
        if (primary_purchase_requirement_vli > 5) {
            primary_purchase_requirement_vli = 5;
        }

        if ((int)strlen(data_str) <= idx + primary_purchase_requirement_vli) {
            return coupon_str;
        }

        /* primary purchase requirement */
        char primary_purchase_requirement_str[6];
        int primary_purchase_requirement_ctr = 0;
        for(primary_purchase_requirement_ctr = 0;
                primary_purchase_requirement_ctr < primary_purchase_requirement_vli;
                primary_purchase_requirement_ctr++, idx++) {
            primary_purchase_requirement_str[primary_purchase_requirement_ctr] =
                data_str[idx];
        }
        primary_purchase_requirement_str[primary_purchase_requirement_ctr] = 0;

        /* primary purchase requirement code */
        if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
            return coupon_str;
        }
        char primary_purchase_requirement_code = data_str[idx++];
        int decimal_places = 0;
        switch(primary_purchase_requirement_code) {
        case '0': {
            decode_strcat(coupon_str, "\nTHRESHOLD NO OF UNITS");
            break;
        }
        case '1': {
            decode_strcat(coupon_str, "\nTHRESHOLD CASH VALUE OF ACCUMULATED TOTAL QUALIFYING ITEMS");
            decimal_places = 2;
            break;
        }
        case '2': {
            decode_strcat(coupon_str, "\nTHRESHOLD CASH VALUE OF TOTAL TRANSACTION");
            decimal_places = 2;
            break;
        }
        case '3': {
            decode_strcat(coupon_str, "\nTHRESHOLD WEIGHT LBS");
            decimal_places = 2;
            break;
        }
        case '4': {
            decode_strcat(coupon_str, "\nTHRESHOLD WEIGHT KG");
            decimal_places = 3;
            break;
        }
        case '9': {
            decode_strcat(coupon_str, "\nCASHIER INTERVENTION REQUIRED");
            break;
        }
        }
        decode_strcat(coupon_str, "\nPRIMARY PURCHASE REQUIREMENT: ");
        int digit_ctr = 0;
        for (digit_ctr = 0;
                digit_ctr < (int)strlen(&primary_purchase_requirement_str[0]) - decimal_places;
                digit_ctr++) {
            decode_strcat_char(coupon_str, primary_purchase_requirement_str[digit_ctr]);
        }
        if (decimal_places > 0) {
            if (digit_ctr > decimal_places) {
                decode_strcat_char(coupon_str, '.');
            }
            while(digit_ctr < (int)strlen(&primary_purchase_requirement_str[0])) {
                decode_strcat_char(coupon_str, primary_purchase_requirement_str[digit_ctr++]);
            }
        }
    }

    /* primary purchase family code */
    if ((int)strlen(data_str) <= idx + 3) {
        return coupon_str;
    }
    char primary_purchase_family_code[4];
    int ctr = 0;
    for (ctr=0; ctr < 3; ctr++, idx++) {
        primary_purchase_family_code[ctr] = data_str[idx];
    }
    primary_purchase_family_code[ctr] = 0;
    decode_strcat(coupon_str, "\nPRIMARY PURCHASE FAMILY CODE: ");
    decode_strcat(coupon_str, &primary_purchase_family_code[0]);

    /* data field 1 - second qualifying purchase */
    if ((int)strlen(data_str) <= idx + 2) {
        return coupon_str;
    }
    if (data_str[idx] == '1') {
        char data_field_1 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 1 - SECOND QUALIFYING PURCHASE: ");
        decode_strcat_char(coupon_str, data_field_1);
        char additional_purchase_rules_code = data_str[idx++];
        switch(additional_purchase_rules_code) {
        case '0': {
            decode_strcat(coupon_str, "\nPRIMARY, 2ND OR 3RD ITEM CAN VALIDATE");
            break;
        }
        case '1': {
            decode_strcat(coupon_str, "\nALL ITEMS LISTED PURCHASED TO VALIDATE");
            break;
        }
        case '2': {
            decode_strcat(coupon_str, "\nPRIMARY AND 2ND OR 3RD ITEM PURCHASED TO VALIDATE");
            break;
        }
        case '3': {
            decode_strcat(coupon_str, "\n2ND OR 3RD FAMILY CODE OR COMPANY TO VALIDATE");
            break;
        }
        }

        /* second purchase requirement VLI */
        vli_str[0] = data_str[idx++];
        int second_purchase_requirement_vli = atoi(&vli_str[0]);
        if (second_purchase_requirement_vli > 0) {
            if (second_purchase_requirement_vli > 5) {
                second_purchase_requirement_vli = 5;
            }

            if ((int)strlen(data_str) <= idx + second_purchase_requirement_vli) {
                return coupon_str;
            }

            /* second purchase requirement */
            char second_purchase_requirement_str[6];
            int second_purchase_requirement_ctr = 0;
            for(second_purchase_requirement_ctr = 0;
                    second_purchase_requirement_ctr < second_purchase_requirement_vli;
                    second_purchase_requirement_ctr++, idx++) {
                second_purchase_requirement_str[second_purchase_requirement_ctr] =
                    data_str[idx];
            }
            second_purchase_requirement_str[second_purchase_requirement_ctr] = 0;

            /* second purchase requirement code */
            if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
                return coupon_str;
            }
            char second_purchase_requirement_code = data_str[idx++];
            decode_strcat(coupon_str, "\nSECOND PURCHASE REQUIREMENT CODE: ");
            decode_strcat_char(coupon_str, second_purchase_requirement_code);
            int decimal_places = 0;
            switch(second_purchase_requirement_code) {
            case '0': {
                decode_strcat(coupon_str, "\nTHRESHOLD NO OF UNITS");
                break;
            }
            case '1': {
                decode_strcat(coupon_str, "\nTHRESHOLD CASH VALUE OF ACCUMULATED TOTAL QUALIFYING ITEMS");
                decimal_places = 2;
                break;
            }
            case '2': {
                decode_strcat(coupon_str, "\nTHRESHOLD CASH VALUE OF TOTAL TRANSACTION");
                decimal_places = 2;
                break;
            }
            case '3': {
                decode_strcat(coupon_str, "\nTHRESHOLD WEIGHT LBS");
                decimal_places = 2;
                break;
            }
            case '4': {
                decode_strcat(coupon_str, "\nTHRESHOLD WEIGHT KG");
                decimal_places = 3;
                break;
            }
            case '9': {
                decode_strcat(coupon_str, "\nCASHIER INTERVENTION REQUIRED");
                break;
            }
            }
            decode_strcat(coupon_str, "\nSECOND PURCHASE REQUIREMENT: ");
            int digit_ctr = 0;
            for (digit_ctr = 0;
                    digit_ctr < (int)strlen(&second_purchase_requirement_str[0]) - decimal_places;
                    digit_ctr++) {
                decode_strcat_char(coupon_str, second_purchase_requirement_str[digit_ctr]);
            }
            if (decimal_places > 0) {
                if (digit_ctr > decimal_places) {
                    decode_strcat_char(coupon_str, '.');
                }
                while(digit_ctr < (int)strlen(&second_purchase_requirement_str[0])) {
                    decode_strcat_char(coupon_str, second_purchase_requirement_str[digit_ctr++]);
                }
            }
        }

        /* second purchase family code */
        if ((int)strlen(data_str) <= idx + 3) {
            return coupon_str;
        }
        char second_purchase_family_code[4];
        ctr = 0;
        for (ctr=0; ctr < 3; ctr++, idx++) {
            second_purchase_family_code[ctr] = data_str[idx];
        }
        second_purchase_family_code[ctr] = 0;
        decode_strcat(coupon_str, "\nSECOND PURCHASE FAMILY CODE: ");
        decode_strcat(coupon_str, &second_purchase_family_code[0]);

        /* second GS1 company prefix VLI */
        if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
            return coupon_str;
        }
        vli_str[0] = data_str[idx++];
        vli = atoi(&vli_str[0]);
        if (vli != 9) {
            vli += 6;
            if ((int)strlen(data_str) <= idx + vli) {
                return coupon_str;
            }
            coupon_start_index = 0;
            if (data_str[idx] == '0') {
                coupon_start_index = 1;
            }

            /* second GS1 company prefix */
            company_prefix_str =
                get_company_prefix_str(company_prefix_code,
                                       idx + coupon_start_index, data_str);
            if (company_prefix_str == NULL) {
                return coupon_str;
            }
            company_ctr = 0;
            decode_strcat(coupon_str, "\nCOMPANY: ");
            for (company_ctr = idx; company_ctr < idx + vli; company_ctr++) {
                decode_strcat_char(coupon_str, data_str[company_ctr]);
            }
            decode_strcat(coupon_str, "\nCOUNTRY: ");
            decode_strcat(coupon_str, company_prefix_str);
            free(company_prefix_str);

            idx += vli;
        }
    }

    /* data field 2 - third qualifying purchase */
    if (data_str[idx] == '2') {
        if ((int)strlen(data_str) <= idx + 2) {
            return coupon_str;
        }
        char data_field_2 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 2 - THIRD QUALIFYING PURCHASE: ");
        decode_strcat_char(coupon_str, data_field_2);
        /* third purchase requirement VLI */
        vli_str[0] = data_str[idx++];
        int third_purchase_requirement_vli = atoi(&vli_str[0]);
        if (third_purchase_requirement_vli > 0) {
            if (third_purchase_requirement_vli > 5) {
                third_purchase_requirement_vli = 5;
            }

            if ((int)strlen(data_str) <= idx + third_purchase_requirement_vli) {
                return coupon_str;
            }

            /* third purchase requirement */
            char third_purchase_requirement_str[6];
            int third_purchase_requirement_ctr = 0;
            for(third_purchase_requirement_ctr = 0;
                    third_purchase_requirement_ctr < third_purchase_requirement_vli;
                    third_purchase_requirement_ctr++, idx++) {
                third_purchase_requirement_str[third_purchase_requirement_ctr] =
                    data_str[idx];
            }
            third_purchase_requirement_str[third_purchase_requirement_ctr] = 0;

            /* third purchase requirement code */
            if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
                return coupon_str;
            }
            char third_purchase_requirement_code = data_str[idx++];
            decode_strcat(coupon_str, "\nTHIRD PURCHASE REQUIREMENT CODE: ");
            decode_strcat_char(coupon_str, third_purchase_requirement_code);
            int decimal_places = 0;
            switch(third_purchase_requirement_code) {
            case '0': {
                decode_strcat(coupon_str, "\nTHRESHOLD NO OF UNITS");
                break;
            }
            case '1': {
                decode_strcat(coupon_str, "\nTHRESHOLD CASH VALUE OF ACCUMULATED TOTAL QUALIFYING ITEMS");
                decimal_places = 2;
                break;
            }
            case '2': {
                decode_strcat(coupon_str, "\nTHRESHOLD CASH VALUE OF TOTAL TRANSACTION");
                decimal_places = 2;
                break;
            }
            case '3': {
                decode_strcat(coupon_str, "\nTHRESHOLD WEIGHT LBS");
                decimal_places = 2;
                break;
            }
            case '4': {
                decode_strcat(coupon_str, "\nTHRESHOLD WEIGHT KG");
                decimal_places = 3;
                break;
            }
            case '9': {
                decode_strcat(coupon_str, "\nCASHIER INTERVENTION REQUIRED");
                break;
            }
            }
            decode_strcat(coupon_str, "\nTHIRD PURCHASE REQUIREMENT: ");
            int digit_ctr = 0;
            for (digit_ctr = 0;
                    digit_ctr < (int)strlen(&third_purchase_requirement_str[0]) - decimal_places;
                    digit_ctr++) {
                decode_strcat_char(coupon_str, third_purchase_requirement_str[digit_ctr]);
            }
            if (decimal_places > 0) {
                if (digit_ctr > decimal_places) {
                    decode_strcat_char(coupon_str, '.');
                }
                while(digit_ctr < (int)strlen(&third_purchase_requirement_str[0])) {
                    decode_strcat_char(coupon_str, third_purchase_requirement_str[digit_ctr++]);
                }
            }
        }

        /* third purchase family code */
        if ((int)strlen(data_str) <= idx + 3) {
            return coupon_str;
        }
        char third_purchase_family_code[4];
        ctr = 0;
        for (ctr=0; ctr < 3; ctr++, idx++) {
            third_purchase_family_code[ctr] = data_str[idx];
        }
        third_purchase_family_code[ctr] = 0;
        decode_strcat(coupon_str, "\nTHIRD PURCHASE FAMILY CODE: ");
        decode_strcat(coupon_str, &third_purchase_family_code[0]);

        /* third GS1 company prefix VLI */
        if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
            return coupon_str;
        }
        vli_str[0] = data_str[idx++];
        vli = atoi(&vli_str[0]);
        if (vli != 9) {
            vli += 6;
            if ((int)strlen(data_str) <= idx + vli) {
                return coupon_str;
            }
            coupon_start_index = 0;
            if (data_str[idx] == '0') {
                coupon_start_index = 1;
            }

            /* third GS1 company prefix */
            company_prefix_str =
                get_company_prefix_str(company_prefix_code,
                                       idx + coupon_start_index, data_str);
            if (company_prefix_str == NULL) {
                return coupon_str;
            }
            company_ctr = 0;
            decode_strcat(coupon_str, "\nCOMPANY: ");
            for (company_ctr = idx; company_ctr < idx + vli; company_ctr++) {
                decode_strcat_char(coupon_str, data_str[company_ctr]);
            }
            decode_strcat(coupon_str, "\nCOUNTRY: ");
            decode_strcat(coupon_str, company_prefix_str);
            free(company_prefix_str);

            idx += vli;
        }
    }

    /* data field 3 - expiration date */
    char * date_str;
    if (data_str[idx] == '3') {
        if ((int)strlen(data_str) <= idx + 6) {
            return coupon_str;
        }
        char data_field_3 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 3 - EXPIRATION DATE: ");
        decode_strcat_char(coupon_str, data_field_3);

        /* expiration date */
        date_str = data_id_convert_date("YYMMDD", &data_str[idx]);
        if (date_str != NULL) {
            decode_strcat(coupon_str, "\nEXPIRATION DATE: ");
            decode_strcat(coupon_str, date_str);
            free(date_str);
        }
        idx += 6;
    }

    /* data field 4 - start date */
    if (data_str[idx] == '4') {
        if ((int)strlen(data_str) <= idx + 6) {
            return coupon_str;
        }
        char data_field_4 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 4 - START DATE: ");
        decode_strcat_char(coupon_str, data_field_4);

        /* start date */
        date_str = data_id_convert_date("YYMMDD", &data_str[idx]);
        if (date_str != NULL) {
            decode_strcat(coupon_str, "\nSTART DATE: ");
            decode_strcat(coupon_str, date_str);
            free(date_str);
        }
        idx += 6;
    }

    /* data field 5 - serial number */
    if (data_str[idx] == '5') {
        if ((int)strlen(data_str) <= idx + 7) {
            return coupon_str;
        }
        char data_field_5 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 5 - SERIAL NO: ");
        decode_strcat_char(coupon_str, data_field_5);

        /* serial number vli */
        if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
            return coupon_str;
        }
        vli_str[0] = data_str[idx++];
        vli = 6 + atoi(&vli_str[0]);
        int serial_ctr = 0;
        decode_strcat(coupon_str, "\nSERIAL NO: ");
        for (serial_ctr = 0; serial_ctr < vli; serial_ctr++, idx++) {
            decode_strcat_char(coupon_str, data_str[idx]);
        }
    }

    /* data field 6 - retailer id */
    if (data_str[idx] == '6') {
        if ((int)strlen(data_str) <= idx + 2) {
            return coupon_str;
        }
        char data_field_6 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 6 - RETAILER ID: ");
        decode_strcat_char(coupon_str, data_field_6);

        /* Retailer GS1 company prefix VLI */
        if ((data_str[idx] < '0') || (data_str[idx] > '9')) {
            return coupon_str;
        }
        vli_str[0] = data_str[idx++];
        vli = 6 + atoi(&vli_str[0]);
        if ((int)strlen(data_str) <= idx + vli) {
            return coupon_str;
        }
        coupon_start_index = 0;
        if (data_str[idx] == '0') {
            coupon_start_index = 1;
        }

        /* Retailer GS1 company prefix */
        company_prefix_str =
            get_company_prefix_str(company_prefix_code,
                                   idx + coupon_start_index, data_str);
        if (company_prefix_str == NULL) {
            return coupon_str;
        }
        company_ctr = 0;
        decode_strcat(coupon_str, "\nRETAILER COMPANY: ");
        for (company_ctr = idx; company_ctr < idx + vli; company_ctr++) {
            decode_strcat_char(coupon_str, data_str[company_ctr]);
        }
        decode_strcat(coupon_str, "\nRETAILER COUNTRY: ");
        decode_strcat(coupon_str, company_prefix_str);
        free(company_prefix_str);

        idx += vli;
    }

    /* data field 9 - misc elements */
    if (data_str[idx] == '9') {
        if ((int)strlen(data_str) < idx + 5) {
            return coupon_str;
        }
        char data_field_9 = data_str[idx++];
        decode_strcat(coupon_str, "\nDATA FIELD 9 - MISC ELEMENTS: ");
        decode_strcat_char(coupon_str, data_field_9);

        char save_value_code = data_str[idx++];
        decode_strcat(coupon_str, "\nSAVE VALUE CODE: ");
        decode_strcat_char(coupon_str, save_value_code);
        switch(save_value_code) {
        case '0': {
            decode_strcat(coupon_str, "\nCENTS OFF QUALIFYING PURCHASE ITEMS");
            break;
        }
        case '1': {
            if (save_value == 0) {
                decode_strcat(coupon_str, "\nONE QUALIFYING PURCHASE ITEM IS FREE");
            }
            else if (save_value > 0) {
                decode_strcat(coupon_str, "\nONE QUALIFYING PURCHASE ITEM IS FREE UP TO SAVE VALUE");
            }
            break;
        }
        case '2': {
            decode_strcat(coupon_str, "\nNO OF QUALIFYING PURCHASE ITEMS THAT ARE FREE");
            break;
        }
        case '5': {
            decode_strcat(coupon_str, "\nPERCENT OFF QUALIFYING PURCHASE ITEM");
            break;
        }
        case '6': {
            decode_strcat(coupon_str, "\nCENTS OFF FINAL TRANSACTION AMOUNT");
            break;
        }
        }

        char save_value_which_item = data_str[idx++];
        decode_strcat(coupon_str, "\nSAVE VALUE WHICH ITEM: ");
        decode_strcat_char(coupon_str, save_value_which_item);
        switch(save_value_which_item) {
        case '0': {
            decode_strcat(coupon_str, " APPLIES TO PRIMARY QUALIFYING ITEM");
            break;
        }
        case '1': {
            decode_strcat(coupon_str, " APPLIES TO 2ND QUALIFYING ITEM");
            break;
        }
        case '2': {
            decode_strcat(coupon_str, " APPLIES TO 3RD QUALIFYING ITEM");
            break;
        }
        }

        char store_coupon_flag = data_str[idx++];
        decode_strcat(coupon_str, "\nSTORE COUPON FLAG: ");
        decode_strcat_char(coupon_str, store_coupon_flag);
        switch(store_coupon_flag) {
        case '0': {
            decode_strcat(coupon_str, " NOT A STORE COUPON");
            break;
        }
        case '1': {
            decode_strcat(coupon_str, " APPLIES TO 1 QUALIFYING ITEM");
            break;
        }
        case '2': {
            decode_strcat(coupon_str, " APPLIES TO 2 QUALIFYING ITEMS");
            break;
        }
        case '3': {
            decode_strcat(coupon_str, " APPLIES TO 3 QUALIFYING ITEMS");
            break;
        }
        case '4': {
            decode_strcat(coupon_str, " APPLIES TO 4 QUALIFYING ITEMS");
            break;
        }
        case '5': {
            decode_strcat(coupon_str, " APPLIES TO 5 QUALIFYING ITEMS");
            break;
        }
        case '6': {
            decode_strcat(coupon_str, " APPLIES TO 6 QUALIFYING ITEMS");
            break;
        }
        case '7': {
            decode_strcat(coupon_str, " APPLIES TO 7 QUALIFYING ITEMS");
            break;
        }
        case '8': {
            decode_strcat(coupon_str, " APPLIES TO 8 QUALIFYING ITEMS");
            break;
        }
        case '9': {
            decode_strcat(coupon_str, " APPLIES TO ALL QUALIFYING ITEMS");
            break;
        }
        }

        char dont_multiply_flag = data_str[idx++];
        if (dont_multiply_flag == '1') {
            decode_strcat(coupon_str, "\nTHIS OFFER MUST NOT BE MULTIPLIED");
        }
    }

    return coupon_str;
}

/**
 * \brief Is the given number prime?
 * \param n number to be checked
 * \return 1 if prime, 0 otherwise
 */
static unsigned char is_prime(int n)
{
    /* 0 and 1 are not prime numbers */
    if (n == 1 || n == 0) return 0;

    /* Check for divisibility from 2 to sqrt(n) */
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

/**
 * \brief returns a pair of check characters as described in section
 *        7.9.5 of the GS1 general specifications
 * \param data_str the string of characters to be checked
 * \param check_characters If the string already contains check characters
 *                         this is the number of them at the end, zero
 *                         otherwise
 * \param check_character_pair returned pair of check characters
 */
void calc_check_character(char data_str[], int check_characters,
                          char check_character_pair[])
{
    int data_len = (int)strlen(data_str) - check_characters;
    int encodable_chars_len = LOOKUP_TABLE_ROWS(encodable_char_ref, 2);
    int curr_prime = 2, sum = 0;
    for (int i = data_len-1; i >= 0; i--) {
        for (int j = 0; j < encodable_chars_len; j++) {
            if (encodable_char_ref[j*2][0] == data_str[i]) {
                while(is_prime(curr_prime) == 0) {
                    curr_prime++;
                }
                sum += (atoi(encodable_char_ref[j*2+1]) * curr_prime);
                curr_prime++;
                break;
            }
        }
    }
    int reference_value = sum % 1021;
    int C1 = reference_value / 32;
    int C2 = reference_value % 32;
    check_character_pair[0] = check_char_ref[C1*2][0];
    check_character_pair[1] = check_char_ref[C2*2][0];
    check_character_pair[2] = 0;
}

/**
 * \brief returns the United Nations production method
 *        See section 3.8.11 of GS1 general specifications
 * \param data_str String containing production method code
 * \return Description of prodiction method
 */
char * get_production_method(char data_str[])
{
    if ((int)strlen(data_str) < 2) return NULL;
    for (int i  = 0; i < (int)strlen(data_str); i++) {
        if ((data_str[i] < '0') || (data_str[i] > '9')) return NULL;
    }
    char * prod_method_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    prod_method_str[0] = 0;
    int prod_method_code = atoi(data_str);
    switch(prod_method_code) {
    case 1: {
        decode_strcat(prod_method_str, "01 CAUGHT AT SEA");
        break;
    }
    case 2: {
        decode_strcat(prod_method_str, "02 CAUGHT IN FRESH WATER");
        break;
    }
    case 3: {
        decode_strcat(prod_method_str, "03 FARMED");
        break;
    }
    case 4: {
        decode_strcat(prod_method_str, "04 CULTIVATED");
        break;
    }
    }
    if ((int)strlen(prod_method_str) == 0) {
        /* not found */
        free(prod_method_str);
        return NULL;
    }
    return prod_method_str;
}

/**
 * \brief returns description of fishing gear type
 *        as defined by Food and Agriculture Organisation (FAO) of the
 *        United Nations
 * \param data_str String to be matched
 * \return Description of fishing gear type
 */
char * get_fishing_gear_type(char data_str[])
{
    if ((int)strlen(data_str) < 2) return NULL;
    for (int i  = 0; i < (int)strlen(data_str); i++) {
        if (((data_str[i] < '0') || (data_str[i] > '9')) &&
                (data_str[i] != '.')) return NULL;
    }
    char * fishing_gear_type_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    fishing_gear_type_str[0] = 0;
    int no_of_fishing_gear_types = LOOKUP_TABLE_ROWS(fishing_gear_type, 3);
    for (int i = 0; i < no_of_fishing_gear_types; i++) {
        if (strcmp(fishing_gear_type[i*3 + 2], data_str) == 0) {
            decode_strcat(fishing_gear_type_str, fishing_gear_type[i*3]);
            break;
        }
    }

    if (fishing_gear_type_str[0] == 0) {
        /* not found */
        free(fishing_gear_type_str);
        return NULL;
    }
    return fishing_gear_type_str;
}

/**
 * \brief returns description of aquatic species
 *        as defined by Food and Agriculture Organisation (FAO) of the
 *        United Nations
 * \param data_str String to be matched
 * \return Description of aquatic species
 */
char * get_aquatic_species(char data_str[])
{
    if ((int)strlen(data_str) != 3) return NULL;
    for (int i  = 0; i < (int)strlen(data_str); i++) {
        if ((data_str[i] < 'A') || (data_str[i] > 'Z')) return NULL;
    }
    char * aquatic_species_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    aquatic_species_str[0] = 0;
    int no_of_aquatic_species = LOOKUP_TABLE_ROWS(aquatic_species, 3);
    for (int i = 0; i < no_of_aquatic_species; i++) {
        if (strcmp(aquatic_species[i*3 + 1], data_str) == 0) {
            decode_strcat(aquatic_species_str, aquatic_species[i*3]);
            break;
        }
    }

    if (aquatic_species_str[0] == 0) {
        /* not found */
        free(aquatic_species_str);
        return NULL;
    }
    return aquatic_species_str;
}

/**
 * \brief returns description of the fishing area
 *        as defined by Food and Agriculture Organisation (FAO) of the
 *        United Nations
 * \param data_str String to be matched
 * \return Description of the fishing area
 */
char * get_fishing_area(char data_str[])
{
    if ((int)strlen(data_str) < 4) return NULL;
    for (int i  = 0; i < (int)strlen(data_str); i++) {
        if (((data_str[i] < '0') || (data_str[i] > '9')) &&
                ((data_str[i] < 'a') || (data_str[i] > 'z')) &&
                ((data_str[i] < 'A') || (data_str[i] > 'Z')) &&
                (data_str[i] != '.')) return NULL;
    }
    char * fishing_area_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    fishing_area_str[0] = 0;

    decode_strcat(fishing_area_str, data_str);

    int no_of_fishing_areas = LOOKUP_TABLE_ROWS(fishing_areas, 2);
    for (int i = 0; i < no_of_fishing_areas; i++) {
        if (strcmp(fishing_areas[i*2 + 1], data_str) == 0) {
            decode_strcat_char(fishing_area_str, ' ');
            decode_strcat(fishing_area_str, fishing_areas[i*2]);
            break;
        }
    }

    if (fishing_area_str[0] == 0) {
        /* not found */
        free(fishing_area_str);
        return NULL;
    }
    return fishing_area_str;
}
