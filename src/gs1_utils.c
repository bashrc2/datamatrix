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
 * \brief returns human readable details for a coupon
 * \param data_str String to be decoded
 * \return decoded coupon string or NULL
 */
char * get_coupon(char data_str[])
{
    int i, data_len = strlen(data_str);
    if (data_len < 12) return NULL;
    char company[13];
    for (i = 0; i < 12; i++) {
        company[i] = data_str[i];
    }
    company[12] = 0;
    char * coupon_str =
        (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
    coupon_str[0] = 0;
    decode_strcat(coupon_str, "COMPANY/COUPON REF: ");
    decode_strcat(coupon_str, company);
    if (data_len > 12) {
        decode_strcat_char(coupon_str, '\n');
        decode_strcat(coupon_str, "SERIAL: ");
        for (i = 12; i < data_len; i++) {
            decode_strcat_char(coupon_str, data_str[i]);
        }
    }
    return coupon_str;
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
    int no_of_package_types =
        ((int)sizeof(package_type_code) /
         (int)sizeof(package_type_code[0]))/2;
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
    int no_of_company_prefixes =
        ((int)sizeof(gs1_company_prefix) /
         (int)sizeof(gs1_company_prefix[0]))/3;
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
    int no_of_countries =
        ((int)sizeof(iso3166_country_codes) /
         (int)sizeof(iso3166_country_codes[0]))/3;
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
    int no_of_countries =
        ((int)sizeof(iso3166_2_country_codes) /
         (int)sizeof(iso3166_2_country_codes[0]))/2;
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
    int no_of_currencies =
        ((int)sizeof(iso4217_currency_codes) /
         (int)sizeof(iso4217_currency_codes[0]))/4;
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
    int no_of_species =
        ((int)sizeof(unece_species_code) /
         (int)sizeof(unece_species_code[0]))/2;
    for (i = 0; i < no_of_species; i++) {
        if ((unece_species_code[i*2][0] == data_str[0]) &&
            (unece_species_code[i*2][1] == data_str[1])) {
            decode_strcat(meat_cut_str, "\nSPECIES: ");
            decode_strcat(meat_cut_str, unece_species_code[i*2 + 1]);
            break;
        }
    }

    /* field 2 */
    int no_of_product_codes =
        ((int)sizeof(unece_product_code) /
         (int)sizeof(unece_product_code[0]))/3;
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
