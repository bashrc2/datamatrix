/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Data Identifier functions
 *  Copyright (c) 2025, Bob Mottram
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

/* from American National Standard Data Identifier and
   Application Identifier Standard */
const char * data_identifier[] = {
  "B", "CONTAINER TYPE",
  "1B", "RETURNABLE CONTAINER ID",
  "2B", "GAS CYLINDER CONTAINER CODE",
  "3B", "MOTOR FREIGHT TRANSPORT EQUIPMENT ID",
  "4B", "STANDARD CARRIER ALPHA CODE",
  "5B", "RECEPTACLE ASSET NUMBER",
  "6B", "RESERVED",
  "7B", "CONTAINER SERIAL",
  "8B", "RETURNABLE CONTAINER OWNER",
  "9B", "CONTAINER SIZE/TYPE CODE",
  "10B", "CONTAINER OWNERSHIP CODE",
  "11B", "VAN NUMBER",
  "12B", "VAN NUMBER CHECK DIGIT",
  "13B", "CONTAINER NUMBER CODE",
  "14B", "TAG STATUS",
  "15B", "DANGEROUS CARGO CLASS",
  "16B", "UN DANGEROUS GOODS CODE",
  "17B", "NAME OF TRANSPORT SUBJECT",
  "18B", "VESSEL REGISTRATION NUMBER",
  "19B", "VOYAGE/TRIP NUMBER",
  "20B", "VESSEL COUNTRY",
  "21B", "ELECTRONIC SEAL NUMBER",
  "22B", "ENTRY NUMBER/TYPE",
  "23B", "SURETY NUMBER",
  "24B", "FOREIGN PORT OF LADING",
  "25B", "PARTY TO TRANSACTION",
  "26B", "RETURNABLE TRANSPORT ITEM ID",
  "27B", "ASSET ID LARGE LOAD CARRIER",
  "28B", "ASSET ID SMALL LOAD CARRIER",
  "29B", "RETURNABLE PACKAGING ITEM",
  "30B", "PACKAGING ITEM NUMBER",
  "31B", "GLOBAL UNIQUE PACKAGING NUMBER",
  "55B", "GLOBAL UNIQUE RETURNABLE PACKAGING ITEM",
  "C", "CONTINUATION OF ITEM CODE",
  "1C", "CONTINUATION OF TRACEABILITY CODE",
  "2C", "CONTINUATION OF SERIAL",
  "3C", "CONTINUATION OF FREE TEXT",
  "4C", "CONTINUATION OF TRANSACTION REF",
  "5C", "CONTINUATION OF ITEM CODE",
  "D", "DATE YYMMDD",
  "1D", "DATE DDMMYY",
  "2D", "DATE MMDDYY",
  "3D", "JULIAN DATE YDDD",
  "4D", "JULIAN DATE YYDDD",
  "5D", "ISO DATE YYMMDD",
  "6D", "ISO DATE YYYYMMDD",
  "7D", "MONTH MMYY",
  "8D", "ISO TIMESTAMP YYYYMMDDHHMM UTC",
  "9D", "DATE",
  "10D", "WEEK YYWW",
  "11D", "WEEK YYYYWW",
  "12D", "DATE YYYYMMDD",
  "13D", "OLDEST AND NEWEST MANUFACTURING WEEK YYWWYYWW",
  "14D", "EXPIRATION DATE YYYYMMDD",
  "15D", "EXPIRATION DATE DDMMYYYY",
  "16D", "MANUFACTURE DATE YYYYMMDD",
  "17D", "PRODUCTION DATE DDMMYYYY",
  "18D", "TAG ACTIVATION TIME YYYYMMDDHHMM UTC",
  "19D", "TAG DEACTIVATION TIME YYYYMMDDHHMM UTC",
  "20D", "INSPECTION DATE DDMMYYYY",
  "21D", "REQUIRED DELIVERY DATE JULIAN",
  "22D", "RECORD TIME YYYYMMDDHHMM UTC",
  "23D", "DATE",
  "24D", "QUALIFIED DATE",
  "25D", "BEST BEFORE YYYYMMDD",
  "26D", "FIRST FREEZE DATE YYYYMMDD",
  "27D", "HARVEST DATE YYYYMMDD",
  "28D", "HARVEST DATE RANGE YYYYMMDDYYYYMMDD",
  "E", "RESTRICTED SUBSTANCES CLASS",
  "1E", "AIR PRESSURE PASCALS",
  "2E", "MAX TEMPERATURE C",
  "3E", "MIN TEMPERATURE C",
  "4E", "MAX RELATIVE HUMIDITY %",
  "5E", "MIN RELATIVE HUMIDITY %",
  "6E", "REFRIGERATOR CONTAINER TEMPERATURE C",
  "10E", "CUMULATIVE TIME TEMPERATURE INDEX",
  "11E", "TIME TEMPERATURE INDEX",
  "12E", "PACKAGING CATEGORY/CODE/WEIGHT",
  "13E", "MSL INDICATOR",
  "F", "LOOPING HEADER",
  "1F", "PARENT",
  "2F", "RESERVED",
  "3F", "NUMBER OF CHILDREN",
  "4F", "PAGE NUMBER/COUNT",
  "5F", "NUMBER OF CHILDREN",
  "H", "NAME OF PARTY",
  "1H", "EMPLOYEE IDENTIFICATION CODE",
  "2H", "US SOCIAL SECURITY NUMBER",
  "3H", "NON EMPLOYEE ID",
  "4H", "NATIONAL SOCIAL SECURITY NUMBER",
  "5H", "LAST NAME",
  "6H", "PARTY NAME",
  "7H", "CONTACT PHONE",
  "8H", "CONTACT EMAIL",
  "9H", "CONSIGNEE NUMBER",
  "10H", "PERSONAL IDENTIFICATION CODE",
  "11H", "FIRST NAME AND MIDDLE INITIAL",
  "12H", "MILITARY GRADE",
  "15H", "NATIONAL IDENTIFICATION NUMBER",
  "25H", "GLOBAL UNIQUE PERSONAL ID",
  "26H", "GLOBAL UNIQUE PERSONAL ID",
  "I", "VEHICLE IDENTIFICATION NUMBER",
  "1I", "RESERVED",
  "2I", "ABBREVIATED VEHICLE IDENTIFICATION NUMBER",
  "3I", "RESERVED",
  "4I", "TRANSPORT VEHICLE IDENTIFIER",
  "5I", "PRODUCTION VEHICLE IDENTIFIER",
  "J", "LICENSE PLATE NUMBER",
  "1J", "LICENSE PLATE NUMBER LOW LEVEL",
  "2J", "LICENSE PLATE NUMBER MULTI-PACKAGE",
  "3J", "LICENSE PLATE NUMBER WITH EDI DATA",
  "4J", "LICENSE PLATE NUMBER MULTI-PACKAGE WITH EDI DATA",
  "5J", "LICENSE PLATE NUMBER MIXED TRANSPORT",
  "6J", "LICENSE PLATE NUMBER MASTER TRANSPORT",
  "7J", "LICENSE PLATE NUMBER NON-UNIQUE",
  "8J", "MARITIME MOBILE SERVICE IDENTITY",
  "K", "CUSTOMER PURCHASE ORDER NUMBER",
  "1K", "SUPPLIERS ORDER NUMBER",
  "2K", "SHIPPER BILL OF LADING/SHIPMENT ID",
  "3K", "CARRIER BILL OF LADING/SHIPMENT ID",
  "4K", "PURCHASE LINE NUMBER",
  "5K", "SHIPMENT REF",
  "6K", "CARRIER PRO NUMBER",
  "7K", "CARRIER MODE",
  "8K", "CONTRACT NUMBER",
  "9K", "TRANSACTION REF",
  "10K", "INVOICE NUMBER",
  "11K", "PACKING LIST NUMBER",
  "12K", "CARRIER ALPHA CODE",
  "13K", "BILL OF LADING/TRANSPORT RECEIPT",
  "14K", "COMBINED ORDER/LINE NUMBER",
  "15K", "KANBAN NUMBER",
  "16K", "DELINS NUMBER",
  "17K", "CHECK NUMBER",
  "18K", "STRUCTURED REFERENCE",
  "19K", "FOREIGN MILITARY SALES CASE NUMBER",
  "20K", "LICENSE ID",
  "21K", "CUSTOMER DATA",
  "22K", "TRANSACTION AUTHENTICATION",
  "25K", "CARRIER TRANSPORT GROUP",
  "26K", "SHIPPER TRANSPORT GROUP",
  "27K", "SUPPLIER QUOTATION NUMBER",
  "L", "STORAGE LOCATION",
  "1L", "LOCATION",
  "2L", "SHIP TO LOCATION",
  "3L", "SHIP FROM LOCATION",
  "4L", "COUNTRY OF ORIGIN",
  "5L", "SHIP FOR CODE",
  "6L", "ROUTE CODE",
  "7L", "DOD ACTIVITY CODE",
  "8L", "PORT OF EMBARKATION",
  "9L", "PORT OF DEBARKATION",
  "10L", "RESERVED",
  "11L", "GEOLOCATION LAT/LONG/ALT",
  "12L", "SHIP TO GEOLOCATION LAT/LONG/ALT",
  "13L", "SHIP FROM GEOLOCATION LAT/LONG/ALT",
  "14L", "RESERVED",
  "15L", "SHIP FOR GEOLOCATION LAT/LONG/ALT",
  "16L", "TAG ACTIVATION LOCATION",
  "17L", "TAG DEACTIVATION LOCATION",
  "18L", "FAO FISHING AREA",
  "20L", "FIRST LEVEL",
  "21L", "SECOND LEVEL",
  "22L", "THIRD LEVEL",
  "23L", "FOURTH LEVEL",
  "24L", "FIFTH LEVEL",
  "25L", "PARTY TO TRANSACTION",
  "26L", "LOCATION CODE",
  "27L", "EVENT LOCATION",
  "28L", "NUMBER AND STREET ADDRESS",
  "29L", "CITY NAME",
  "30L", "COUNTRY SUB-ENTITY",
  "31L", "POSTAL CODE",
  "32L", "COUNTRY CODE",
  "33L", "URL",
  "34L", "PROCESS URL",
  "35L", "GOVERNMENT APPROVAL NUMBER",
  "36L", "GOVERNMENT APPROVAL NUMBER",
  "51L", "SHIP FROM LOCATION CODE",
  "52L", "SHIP TO LOCATION CODE",
  "53L", "RESERVED",
  "54L", "SHIP FROM LOCATION CODE",
  "55L", "SHIP TO LOCATION CODE",
  "M", "RESERVED",
  "10M", "ARMY FORM 2410 DATA",
  "11M", "ARMY FORM 2408 DATA",
  "12M", "ARMY FORM 2407 DATA",
  "13M", "AIR FORCE FORM 95 DATA",
  "14M", "NAVY FORM 4790 DATA",
  "N", "NATO STOCK NUMBER",
  "1N", "CHEMICAL CHARACTERISTIC DATA",
  "2N", "RESERVED",
  "3N", "CODING STRUCTURE",
  "4N", "GS1 CODING STRUCTURE",
  "5N", "AIAG CODING STRUCTURE",
  "6N", "DOD REQUISITION CODE",
  "7N", "TRANSPORTATION REGULATION CODE",
  "8N", "PRODUCTION ANIMAL ID",
  "9N", "PHARMACY PRODUCT NUMBER",
  "10N", "COMPANY DATA",
  "11N", "RLA DATA CONSTRUCT",
  "12N", "RLA DATA CONSTRUCT",
  "P", "CUSTOMER ITEM ID",
  "1P", "SUPPLIER ITEM ID",
  "2P", "REVISION LEVEL",
  "3P", "MANUFACTURER/ITEM CODE",
  "4P", "GS1 ITEM CODE",
  "5P", "CARRIER FREIGHT CLASSIFICATION ITEM NUMBER",
  "6P", "SUPPLIER/ITEM CODE",
  "7P", "COMMON LANGUAGE EQUIPMENT ID",
  "8P", "GS1 GTIN",
  "9P", "MANUFACTURER/ITEM CODE",
  "10P", "HAZARDOUS MATERIAL CODE",
  "11P", "TELECOM CODE",
  "12P", "DOCUMENT TYPE",
  "13P", "VMRS SYSTEM CODE",
  "14P", "VMRS SYSTEM AND ASSEMBLY CODE",
  "15P", "VMRS SYSTEM/ASSEMBLY/PART CODE",
  "16P", "MODIFIED VMRS SYSTEM/ASSEMBLY/PART CODE",
  "17P", "GS1 SUPPLIER/ITEM CODE",
  "18P", "VMRS SUPPLIER/PART NUMBER",
  "19P", "COMPONENT OF AN ITEM",
  "20P", "CUSTOMER FIRST LEVEL",
  "21P", "CUSTOMER SECOND LEVEL",
  "22P", "CUSTOMER THIRD LEVEL",
  "23P", "CUSTOMER FOURTH LEVEL",
  "24P", "CUSTOMER FIFTH LEVEL",
  "25P", "PARTY TO TRANSACTION",
  "26P", "PART NUMBER OF HIGHER ASSEMBLY",
  "27P", "COMMODITY CODE",
  "28P", "CARGO NAME",
  "29P", "PRODUCT CLASSIFICATION CODE",
  "30P", "SUPPLIER FIRST LEVEL",
  "31P", "SUPPLIER SECOND LEVEL",
  "32P", "SUPPLIER THIRD LEVEL",
  "33P", "SUPPLIER FOURTH LEVEL",
  "34P", "SUPPLIER FIFTH LEVEL",
  "40P", "MATERIAL SAFETY CODE",
  "49P", "EXPORT CONTROLLED ITEM",
  "50P", "MANUFACTURER ITEM ID",
  "51P", "ITEM ID",
  "52P", "COLOR CODE",
  "53P", "EU MARINE EQUIPMENT ID",
  "54P", "MEDICAL DEVICE ID",
  "Q", "QUANTITY",
  "1Q", "LENGTH/WEIGHT",
  "2Q", "WEIGHT",
  "3Q", "UNIT OF MEASURE",
  "4Q", "GROSS AMOUNT",
  "5Q", "NET AMOUNT",
  "6Q", "MULTI CONTAINER",
  "7Q", "QUANTITY",
  "8Q", "CONTAINER RATED WEIGHT",
  "9Q", "PIECE WEIGHT",
  "10Q", "RESERVED",
  "11Q", "TARE WEIGHT",
  "12Q", "MONETARY VALUE",
  "13Q", "NUMBER WITHIN SET",
  "14Q", "BEGINNING SECONDARY QUANTITY",
  "15Q", "ENDING SECONDARY QUANTITY",
  "16Q", "NUMBER OF PIECES IN VAN",
  "17Q", "NUMBER OF SHIPMENTS IN VAN",
  "18Q", "CUBIC METRES/FEET",
  "19Q", "WIDTH",
  "20Q", "HEIGHT",
  "21Q", "LENGTH",
  "22Q", "NET WEIGHT",
  "23Q", "VAN LENGTH",
  "24Q", "INSIDE CUBE OF A VAN",
  "25Q", "NET EXPLOSIVE WEIGHT",
  "26Q", "PACKAGING LEVEL",
  "27Q", "SINGLE PRODUCT PRICE",
  "28Q", "SINGLE POSTAGE CHARGE",
  "29Q", "DISCOUNT %",
  "30Q", "VAT %",
  "31Q", "CURRENCY",
  "R", "RESERVED",
  "1R", "SUPPLIER RETURN AUTHORIZATION CODE",
  "2R", "CUSTOMER RETURN CODE",
  "3R", "RESERVED",
  "4R", "US DOD ID",
  "5R", "DATA",
  "6R", "ISO20248 DIGITAL SIGNATURE",
  "7R", "AQUATIC SCIENCES AND FISHERIES",
  "8R", "FISHING GEARS CODE",
  "9R", "FISH/SEAFOOD PRODUCTION METHOD",
  "S", "SUPPLIER SERIAL NUMBER",
  "1S", "SUPPLIER LIFETIME CODE",
  "2S", "ADVANCE SHIPMENT NOTIFICATION",
  "3S", "SUPPLIER PACKAGE ID",
  "4S", "SUPPLIER PACKAGE ID",
  "5S", "SUPPLIER PACKAGE ID",
  "6S", "SUPPLIER PACKAGE ID",
  "7S", "SUPPLIER PACKAGE ID",
  "8S", "SUPPLIER ID",
  "9S", "PACKAGE ID",
  "10S", "MACHINE/CELL/TOOL ID",
  "11S", "FIXED ASSET ID",
  "12S", "DOCUMENT NUMBER",
  "13S", "CONTAINER SECURITY SEAL",
  "14S", "4TH CLASS PARCEL",
  "15S", "VENDOR SERIAL",
  "16S", "VERSION NUMBER",
  "17S", "GS1 SUPPLIER ID",
  "18S", "CAGE CODE",
  "19S", "COMPANY ID",
  "20S", "CUSTOMER TRACEABILITY CODE",
  "21S", "TIRE ID",
  "22S", "CELLULAR MOBILE ID",
  "23S", "MAC ADDRESS",
  "24S", "FREIGHT CONTAINER TAG",
  "25S", "PARTY TO TRANSACTION",
  "26S", "EQUIPMENT ID",
  "27S", "BATCH ITEM NUMBER",
  "28S", "BATCH AND ITEM NUMBER",
  "29S", "RESERVED",
  "30S", "SUPPLIER TRACEABILITY CODE",
  "31S", "BEGINNING SERIAL",
  "32S", "ENDING SERIAL",
  "33S", "SERIAL NUMBER OF HIGHER ASSEMBLY",
  "34S", "SERIAL/PART NUMBER OF END ITEM",
  "35S", "BUMPER NUMBER",
  "36S", "PALLET IDENTIFIER",
  "37S", "UNIQUE ITEM IDENTIFIER",
  "42S", "UNIQUE ITEM IDENTIFIER",
  "43S", "INTEGRATED CIRCUIT CARD ID",
  "50S", "SUPPLIER FIRST LEVEL",
  "51S", "SUPPLIER SECOND LEVEL",
  "52S", "SUPPLIER THIRD LEVEL",
  "53S", "SUPPLIER FOURTH LEVEL",
  "54S", "SUPPLIER FIFTH LEVEL",
  "96S", "EPC NUMBER",
  "T", "CUSTOMER TRACEABILITY NUMBER",
  "1T", "SUPPLIER TRACEABILITY NUMBER",
  "2T", "RESERVED",
  "3T", "EXCLUSIVE ASSIGNMENT",
  "20T", "CUSTOMER FIRST LEVEL",
  "21T", "CUSTOMER SECOND LEVEL",
  "22T", "CUSTOMER THIRD LEVEL",
  "23T", "CUSTOMER FOURTH LEVEL",
  "24T", "CUSTOMER FIFTH LEVEL",
  "25T", "PARTY TO TRANSACTION",
  "26T", "BATCH ID",
  "27T", "BATCH NUMBER",
  "30T", "SUPPLIER FIRST LEVEL",
  "31T", "SUPPLIER SECOND LEVEL",
  "32T", "SUPPLIER THIRD LEVEL",
  "33T", "SUPPLIER FOURTH LEVEL",
  "34T", "SUPPLIER FIFTH LEVEL",
  "5U", "POSTAL SERVICE",
  "6U", "LICENSING POST DATA",
  "15U", "POSTAL SERVICE",
  "16U", "POSTAL ADMINISTRATION ID",
  "17U", "UPU LOCATION CODE",
  "18U", "QUALIFIED UPU LOCATION CODE",
  "19U", "COMPOUND LICENSE PLATE",
  "55U", "OCR DATA LOCATOR",
  "V", "CUSTOMER SUPPLIER CODE",
  "1V", "SUPPLIERS SUPPLIER CODE",
  "2V", "UPC COMPANY PREFIX",
  "3V", "GS1 COMPANY PREFIX",
  "4V", "CARRIER ID",
  "5V", "FINANCIAL INSTITUTION ID",
  "6V", "MANUFACTURERS ID",
  "7V", "LIABLE PARTY",
  "8V", "CUSTOMERS CUSTOMER CODE",
  "9V", "SUPPLIER CUSTOMER CODE",
  "10V", "MANUFACTURER ID",
  "11V", "RESPONSIBLE ORGANIZATION",
  "12V", "MANUFACTURER DUNS NUMBER",
  "13V", "SUPPLIER DUNS NUMBER",
  "14V", "CUSTOMER DUNS NUMBER",
  "15V", "CARRIER SHIPPER NUMBER",
  "16V", "VMRS SUPPLIER ID",
  "17V", "US DOD CAGE CODE",
  "18V", "PARTY TO TRANSACTION",
  "19V", "ROLE",
  "20V", "PARTY TO TRANSACTION",
  "21V", "PARTY TO TRANSACTION",
  "22V", "CARRIER STANDARD CARRIER ALPHA CODE",
  "23V", "SUPPLIER VAT ID",
  "24V", "CUSTOMER VAT ID",
  "W", "WORK ORDER NUMBER",
  "1W", "OPERATION SEQUENCE NUMBER",
  "2W", "OPERATION/WORK CODE",
  "3W", "COMBINED WORK ORDER NUMBER",
  "4W", "STATUS CODE",
  "5W", "WORK UNIT CODE",
  "6W", "NOMENCLATURE",
  "10W", "FORM CONTROL NUMBER",
  "11W", "QUALITY ASSURANCE INSPECTOR",
  "12W", "PHONE NUMBER",
  "Z", "DEFINED BETWEEN CUSTOMER AND SUPPLIER",
  "1Z", "DEFINED BETWEEN CARRIER AND SUPPLIER",
  "2Z", "DEFINED BETWEEN CUSTOMER AND CARRIER",
  "3Z", "FREE TEXT",
  "4Z", "DEFINED BETWEEN CARRIER AND TRADING PARTNER",
  "10Z", "HEADER DATA"
};

/**
 * \brief locates a data identifier within the given data string
 * \param data_str data string
 * \param id returned short identifier, up to 3 characters
 * \param id_human_readable returned human readable identifier
 * \param value returned identifier value
 * \return 1 if a data identifier was found, 0 otherwise
 */
int get_data_identifier(char data_str[],
                        char id[], char id_human_readable[], char value[])
{
  int no_of_identifiers, i, id_index=-1;
  char search_id[4];

  id[0] = 0;
  id_human_readable[0] = 0;
  value[0] = 0;

  if (strlen(data_str) < 4) return 0;

  for (i = 0; i <= 2; i++) {
    search_id[i] = data_str[i];
    if ((data_str[i] >= 'A') && (data_str[i] <= 'Z')) {
      search_id[i+1] = 0;
      id_index = i;
      break;
    }
  }

  if (id_index == -1) return 0;
  no_of_identifiers = (int)(sizeof(data_identifier)/sizeof(data_identifier[0]));
  for (i = 0; i < no_of_identifiers; i+=2) {
    if (strcmp(&search_id[0], data_identifier[i]) == 0) {
      decode_strcat(id_human_readable, (char*)data_identifier[i+1]);
      break;
    }
  }

  if (strlen(id_human_readable) > 0) {
    decode_strcat(value, &data_str[id_index+1]);
    decode_strcat(id, &search_id[0]);
    if (strlen(value) > 0) {
      return 1;
    }
  }
  return 0;
}

/**
 * \brief returns a text equivalent to a month number
 * \param month_number Month number as a string
 * \return month text
 */
static char * month_number_to_text(char month_number[])
{
  if ((month_number[0] < '0') || (month_number[0] > '9')) return NULL;
  if ((month_number[1] < '0') || (month_number[1] > '9')) return NULL;
  char mnth_str[3];
  mnth_str[0] = month_number[0];
  mnth_str[1] = month_number[1];
  mnth_str[2] = 0;
  int month_no = atoi(&mnth_str[0]);
  if ((month_no < 1) || (month_no > 12)) return NULL;
  char * month_text[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  return month_text[month_no-1];
}

/**
 * \brief converts dates into a more human readable style
 * \param id_human_readable data identifier description
 * \param id_value the date string to be converted
 * \return human readable date, or NULL
 */
char * data_id_convert_date(char * id_human_readable, char * id_value)
{
  if (strstr(id_human_readable, "YYYYMMDDHHMM") != NULL) {
    if (strlen(id_value) >= 12) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat_char(date_value, '-');
      /* month */
      char * mnth = month_number_to_text(&id_value[4]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[4]);
        decode_strcat_char(date_value, id_value[5]);
      }
      decode_strcat_char(date_value, '-');
      /* day */
      decode_strcat_char(date_value, id_value[6]);
      decode_strcat_char(date_value, id_value[7]);
      decode_strcat_char(date_value, ' ');
      /* hour */
      decode_strcat_char(date_value, id_value[8]);
      decode_strcat_char(date_value, id_value[9]);
      decode_strcat_char(date_value, ':');
      /* min */
      decode_strcat_char(date_value, id_value[10]);
      decode_strcat_char(date_value, id_value[11]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYYYMMDDYYYYMMDD") != NULL) {
    if (strlen(id_value) >= 16) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat_char(date_value, '-');
      /* month */
      char * mnth = month_number_to_text(&id_value[4]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[4]);
        decode_strcat_char(date_value, id_value[5]);
      }
      decode_strcat_char(date_value, '-');
      /* day */
      decode_strcat_char(date_value, id_value[6]);
      decode_strcat_char(date_value, id_value[7]);
      decode_strcat(date_value, " - ");
      /* year */
      decode_strcat_char(date_value, id_value[8]);
      decode_strcat_char(date_value, id_value[9]);
      decode_strcat_char(date_value, id_value[10]);
      decode_strcat_char(date_value, id_value[11]);
      decode_strcat_char(date_value, '-');
      /* month */
      decode_strcat_char(date_value, id_value[12]);
      decode_strcat_char(date_value, id_value[13]);
      decode_strcat_char(date_value, '-');
      /* day */
      decode_strcat_char(date_value, id_value[14]);
      decode_strcat_char(date_value, id_value[15]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYYYMMDD") != NULL) {
    if (strlen(id_value) >= 8) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[6]);
      decode_strcat_char(date_value, id_value[7]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[4]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[4]);
        decode_strcat_char(date_value, id_value[5]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYYYDDMM") != NULL) {
    if (strlen(id_value) >= 8) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[6]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[6]);
        decode_strcat_char(date_value, id_value[7]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYYYWW") != NULL) {
    if (strlen(id_value) >= 6) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat(date_value, " WEEK ");
      /* week */
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYWWYYWW") != NULL) {
    if (strlen(id_value) >= 8) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat(date_value, " WEEK ");
      /* week */
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat(date_value, " - ");
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      decode_strcat(date_value, " WEEK ");
      /* week */
      decode_strcat_char(date_value, id_value[6]);
      decode_strcat_char(date_value, id_value[7]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYMMDDHH") != NULL) {
    if (strlen(id_value) >= 8) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[2]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[2]);
        decode_strcat_char(date_value, id_value[3]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, ' ');
      /* hour */
      decode_strcat_char(date_value, id_value[6]);
      decode_strcat_char(date_value, id_value[7]);
      decode_strcat(date_value, ":00");
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYMMDD") != NULL) {
    if (strlen(id_value) >= 6) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[2]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[2]);
        decode_strcat_char(date_value, id_value[3]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYDDMM") != NULL) {
    if (strlen(id_value) >= 6) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[4]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[4]);
        decode_strcat_char(date_value, id_value[5]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "DDMMYY") != NULL) {
    if (strlen(id_value) >= 6) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[2]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[2]);
        decode_strcat_char(date_value, id_value[3]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "MMDDYY") != NULL) {
    if (strlen(id_value) >= 6) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* day */
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat_char(date_value, ' ');
      /* month */
      char * mnth = month_number_to_text(&id_value[0]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[0]);
        decode_strcat_char(date_value, id_value[1]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat_char(date_value, id_value[5]);
      return date_value;
    }
  }

  if ((strstr(id_human_readable, "YYJJJHH") != NULL) ||
      (strstr(id_human_readable, "YYDDDHH") != NULL)) {
    if (strlen(id_value) >= 7) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat(date_value, " DAY ");
      /* Julian day */
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat_char(date_value, id_value[4]);
      decode_strcat(date_value, " ");
      /* hour */
      decode_strcat_char(date_value, id_value[5]);
      decode_strcat_char(date_value, id_value[6]);
      decode_strcat(date_value, ":00");
      return date_value;
    }
  }

  if ((strstr(id_human_readable, "YYJJJ") != NULL) ||
      (strstr(id_human_readable, "YYDDD") != NULL)) {
    if (strlen(id_value) >= 5) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat(date_value, " DAY ");
      /* Julian day */
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      decode_strcat_char(date_value, id_value[4]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "MMYY") != NULL) {
    if (strlen(id_value) >= 4) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* month */
      char * mnth = month_number_to_text(&id_value[0]);
      if (mnth != NULL) {
        decode_strcat(date_value, mnth);
      }
      else {
        decode_strcat_char(date_value, id_value[0]);
        decode_strcat_char(date_value, id_value[1]);
      }
      decode_strcat_char(date_value, ' ');
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      return date_value;
    }
  }

  if (strstr(id_human_readable, "YYWW") != NULL) {
    if (strlen(id_value) >= 4) {
      char * date_value = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));
      date_value[0] = 0;
      /* year */
      decode_strcat(date_value, "20");
      decode_strcat_char(date_value, id_value[0]);
      decode_strcat_char(date_value, id_value[1]);
      decode_strcat(date_value, " WEEK ");
      /* week */
      decode_strcat_char(date_value, id_value[2]);
      decode_strcat_char(date_value, id_value[3]);
      return date_value;
    }
  }

  return NULL;
}
