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

char * iso3166_country_codes[] = {
  "Jersey", "JEY", "832",
  "Jordan", "JOR", "400",
  "Kazakhstan",	"KAZ", "398",
  "Kenya", "KEN", "404",
  "Kiribati", "KIR", "296",
  "North Korea", "PRK", "408",
  "South Korea", "KOR", "410",
  "Kuwait", "KWT", "414",
  "Kyrgyzstan", "KGZ", "417",
  "Laos", "LAO", "418",
  "Latvia", "LVA", "428",
  "Lebanon", "LBN", "422",
  "Lesotho", "LSO", "426",
  "Liberia", "LBR", "430",
  "Libya", "LBY", "434",
  "Liechtenstein", "LIE", "438",
  "Lithuania", "LTU", "440",
  "Luxembourg", "LUX", "442",
  "Macau", "MAC", "446",
  "Madagascar", "MDG", "450",
  "Malawi", "MWI", "454",
  "Malaysia", "MYS", "458",
  "Maldives", "MDV", "462",
  "Mali", "MLI", "466",
  "Malta", "MLT", "470",
  "Marshall Islands", "MHL", "584",
  "Martinique", "MTQ", "474",
  "Mauritania", "MRT", "478",
  "Mauritius", "MUS", "480",
  "Mayotte", "MYT", "175",
  "Mexico", "MEX", "484",
  "Federated States of Micronesia", "FSM", "583",
  "Moldova", "MDA", "498",
  "Monaco", "MCO", "492",
  "Mongolia",	"MNG", "496",
  "Montenegro", "MNE", "499",
  "Montserrat", "MSR", "500",
  "Morocco", "MAR", "504",
  "Mozambique", "MOZ", "508",
  "Myanmar", "MMR", "104",
  "Namibia", "NAM", "516",
  "Nauru", "NRU", "520",
  "Nepal", "NPL", "524",
  "Netherlands", "NLD", "528",
  "New Caledonia", "NCL", "540",
  "New Zealand", "NZL", "554",
  "Nicaragua", "NIC", "558",
  "Niger", "NER", "562",
  "Nigeria", "NGA", "566",
  "Niue", "NIU", "570",
  "Norfolk Island", "NFK", "574",
  "North Korea", "MKD", "807",
  "Northern Mariana Islands", "MNP", "580",
  "Norway", "NOR", "578",
  "Oman", "OMN", "512",
  "Pakistan", "PAK", "586",
  "Palau", "PLW", "585",
  "Palestine", "PSE", "275",
  "Panama", "PAN", "591",
  "Papua New Guinea", "PNG", "598",
  "Paraguay",	"PRY", "600",
  "Peru", "PER", "604",
  "Philippines", "PHL", "608",
  "Pitcairn Islands", "PCN", "612",
  "Poland", "POL", "616",
  "Portugal", "PRT", "620",
  "Puerto Rico", "PRI", "630",
  "Qatar", "QAT", "634",
  "Réunion", "REU", "638",
  "Romania", "ROU", "642",
  "Russia", "RUS", "643",
  "Rwanda", "RWA", "646",
  "Saint Barthélemy", "BLM", "652",
  "Saint Helena", "SHN", "654",
  "Saint Kitts and Nevis", "KNA", "659",
  "Saint Lucia", "LCA", "662",
  "Collectivity of Saint Martin", "MAF", "663",
  "Saint Pierre and Miquelon", "SPM", "666",
  "Saint Vincent and the Grenadines", "VCT", "670",
  "Samoa", "WSM", "882",
  "San Marino", "SMR", "674",
  "São Tomé and Príncipe", "STP", "678",
  "Saudi Arabia", "SAU", "682",
  "Senegal", "SEN", "686",
  "Serbia", "SRB", "688",
  "Seychelles", "SYC", "690",
  "Sierra Leone", "SLE", "694",
  "Singapore", "SGP", "702",
  "Sint Maarten", "SXM", "534",
  "Slovakia", "SVK", "703",
  "Slovenia", "SVN", "705",
  "Solomon Islands", "SLB", "090",
  "Somalia", "SOM", "706",
  "South Africa", "ZAF", "710",
  "South Georgia and the South Sandwich Islands", "SGS", "239",
  "South Sudan", "SSD", "728",
  "Spain", "ESP", "724",
  "Sri Lanka", "LKA", "144",
  "Sudan", "SDN", "729",
  "Suriname", "SUR", "740",
  "Svalbard", "SJM", "744",
  "Sweden", "SWE", "752",
  "Switzerland", "CHE", "756",
  "Syria", "SYR", "760",
  "Taiwan", "TWN", "158",
  "Tajikistan", "TJK", "762",
  "Tanzania", "TZA", "834",
  "Thailand", "THA", "764",
  "Timor-Leste", "TLS", "626",
  "Togo", "TGO", "768",
  "Tokelau", "TKL", "772",
  "Tonga", "TON", "776",
  "Trinidad and Tobago", "TTO", "780",
  "Tunisia", "TUN", "788",
  "Turkey", "TUR", "792",
  "Turkmenistan", "TKM", "795",
  "Turks and Caicos Islands", "TCA", "796",
  "Tuvalu", "TUV", "798",
  "Uganda", "UGA", "800",
  "Ukraine", "UKR", "804",
  "United Arab Emirates", "ARE", "784",
  "United Kingdom", "GBR", "826",
  "United States", "UMI", "581",
  "United States", "USA", "840",
  "Uruguay", "URY", "858",
  "Uzbekistan", "UZB", "860",
  "Vanuatu", "VUT", "548",
  "Venezuela", "VEN", "862",
  "Vietnam", "VNM", "704",
  "British Virgin Islands", "VGB", "092",
  "United States Virgin Islands", "VIR", "850",
  "Wallis and Futuna", "WLF", "876",
  "Western Sahara", "ESH", "732",
  "Yemen", "YEM", "887",
  "Zambia", "ZMB", "894",
  "Zimbabwe", "ZWE", "716"
};

char * iso4217_currency_codes[] = {
  "AFGHANISTAN",         "Afghani",               "AFN",       "971",
  "ALBANIA",             "Lek",                   "ALL",       "008",
  "ALGERIA",             "Algerian Dinar",        "DZD",       "012",
  "AMERICAN SAMOA",      "US Dollar",             "USD",       "840",
  "ANDORRA",             "Euro",                  "EUR",       "978",
  "ANGOLA", 	           "Kwanza",                "AOA",       "973",
  "ANGUILLA",            "East Caribbean Dollar", "XCD",       "951",
  "ANTIGUA AND BARBUDA", "East Caribbean Dollar", "XCD",       "951",
  "ARGENTINA",           "Argentine Peso",        "ARS",       "032",
  "ARMENIA",             "Armenian Dram",         "AMD",       "051",
  "ARUBA",               "Aruban Florin",         "AWG",       "533",
  "AUSTRALIA",           "Australian Dollar",     "AUD",       "036",
  "AUSTRIA",             "Euro",                  "EUR",       "978",
  "AZERBAIJAN",          "Azerbaijanian Manat",   "AZN",       "944",
  "BAHAMAS (THE)",       "Bahamian Dollar",       "BSD",       "044",
  "BAHRAIN",             "Bahraini Dinar",        "BHD",       "048",
  "BANGLADESH",          "Taka",                  "BDT",       "050",
  "BARBADOS",            "Barbados Dollar",       "BBD",       "052",
  "BELARUS",             "Belarussian Ruble",     "BYN",       "933",
  "BELGIUM",             "Euro",                  "EUR",       "978",
  "BELIZE",              "Belize Dollar",         "BZD",       "084",
  "BENIN",               "CFA Franc BCEAO",       "XOF",       "952",
  "BERMUDA",             "Bermudian Dollar",      "BMD",       "060",
  "BHUTAN",              "Ngultrum",              "BTN",       "064",
  "BHUTAN",              "Indian Rupee",          "INR",       "356",
  "BOLIVIA (PLURINATIONAL STATE OF)", "Boliviano", "BOB", "068",
  "BOLIVIA (PLURINATIONAL STATE OF)", "Mvdol",    "BOV",       "984",
  "BONAIRE, SINT EUSTATIUS AND SABA", "US Dollar", "USD",      "840",
  "BOSNIA AND HERZEGOVINA", "Convertible Mark",   "BAM",       "977",
  "BOTSWANA",            "Pula",                  "BWP",       "072",
  "BOUVET ISLAND",       "Norwegian Krone",       "NOK",       "578",
  "BRAZIL",              "Brazilian Real",        "BRL",       "986",
  "BRITISH INDIAN OCEAN TERRITORY (THE)", "US Dollar", "USD",  "840",
  "BRUNEI DARUSSALAM",   "Brunei Dollar",         "BND",       "096",
  "BULGARIA",            "Bulgarian Lev",         "BGN",       "975",
  "BURKINA FASO",        "CFA Franc BCEAO",       "XOF",       "952",
  "BURUNDI",             "Burundi Franc",         "BIF",       "108",
  "CABO VERDE",          "Cabo Verde Escudo",     "CVE",       "132",
  "CAMBODIA",            "Riel",                  "KHR",       "116",
  "CAMEROON",            "CFA Franc BEAC",        "XAF",       "950",
  "CANADA",              "Canadian Dollar",       "CAD",       "124",
  "CAYMAN ISLANDS (THE)", "Cayman Islands Dollar", "KYD",      "136",
  "CENTRAL AFRICAN REPUBLIC (THE)", "CFA Franc BEAC", "XAF",   "950",
  "CHAD",                "CFA Franc BEAC",        "XAF",       "950",
  "CHILE",               "Unidad de Fomento",     "CLF",       "990",
  "CHILE",               "Chilean Peso",          "CLP",       "152",
  "CHINA",               "Yuan Renminbi",         "CNY",       "156",
  "CHRISTMAS ISLAND",    "Australian Dollar",     "AUD",       "036",
  "COCOS (KEELING) ISLANDS (THE)", "Australian Dollar", "AUD", "036",
  "COLOMBIA",            "Colombian Peso",        "COP",       "170",
  "COLOMBIA",            "Unidad de Valor Real",  "COU",       "970",
  "COMOROS (THE)",       "Comoro Franc",          "KMF",       "174",
  "CONGO (THE DEMOCRATIC REPUBLIC OF THE)", "Congolese Franc", "CDF", "976",
  "CONGO (THE)",         "CFA Franc BEAC",        "XAF",       "950",
  "COOK ISLANDS (THE)",  "New Zealand Dollar",    "NZD",       "554",
  "COSTA RICA",          "Costa Rican Colon",     "CRC",       "188",
  "CROATIA",             "Euro",                  "EUR",       "978",
  "CUBA",                "Peso Convertible",      "CUC",       "931",
  "CUBA",                "Cuban Peso",            "CUP",       "192",
  "CURAÇAO",             "Caribbean Guilder",     "XCG",       "532",
  "CYPRUS",              "Euro",                  "EUR",       "978",
  "CZECH REPUBLIC (THE)", "Czech Koruna",         "CZK",       "203",
  "CÔTE D'IVOIRE",       "CFA Franc BCEAO",       "XOF",       "952",
  "DENMARK",             "Danish Krone",          "DKK",       "208",
  "DJIBOUTI",            "Djibouti Franc",        "DJF",       "262",
  "DOMINICA",            "East Caribbean Dollar", "XCD",       "951",
  "DOMINICAN REPUBLIC (THE)", "Dominican Peso",   "DOP",       "214",
  "ECUADOR",             "US Dollar",             "USD",       "840",
  "EGYPT",               "Egyptian Pound",        "EGP",       "818",
  "EL SALVADOR",         "El Salvador Colon",     "SVC",       "222",
  "EL SALVADOR",         "US Dollar",             "USD",       "840",
  "EQUATORIAL GUINEA",   "CFA Franc BEAC",        "XAF",       "950",
  "ERITREA",             "Nakfa",                 "ERN",       "232",
  "ESTONIA",             "Euro",                  "EUR",       "978",
  "ETHIOPIA",            "Ethiopian Birr",        "ETB",       "230",
  "EUROPEAN UNION",      "Euro",                  "EUR",       "978",
  "FALKLAND ISLANDS (THE)", "Falkland Islands Pound", "FKP",   "238",
  "FAROE ISLANDS (THE)", "Danish Krone",          "DKK",       "208",
  "FIJI",                "Fiji Dollar",           "FJD",       "242",
  "FINLAND",             "Euro",                  "EUR",       "978",
  "FRANCE",              "Euro",                  "EUR",       "978",
  "FRENCH GUIANA",       "Euro",                  "EUR",       "978",
  "FRENCH POLYNESIA",    "CFP Franc",             "XPF",       "953",
  "FRENCH SOUTHERN TERRITORIES (THE)", "Euro",    "EUR",       "978",
  "GABON",               "CFA Franc BEAC",        "XAF",       "950",
  "GAMBIA (THE)",        "Dalasi",                "GMD",       "270",
  "GEORGIA",             "Lari",                  "GEL",       "981",
  "GERMANY",             "Euro",                  "EUR",       "978",
  "GHANA",               "Ghana Cedi",            "GHS",       "936",
  "GIBRALTAR",           "Gibraltar Pound",       "GIP",       "292",
  "GREECE",              "Euro",                  "EUR",       "978",
  "GREENLAND",           "Danish Krone",          "DKK",       "208",
  "GRENADA",             "East Caribbean Dollar", "XCD",       "951",
  "GUADELOUPE",          "Euro",                  "EUR",       "978",
  "GUAM",                "US Dollar",             "USD",       "840",
  "GUATEMALA",           "Quetzal",               "GTQ",       "320",
  "GUERNSEY",            "Pound Sterling",        "GBP",       "826",
  "GUINEA",              "Guinea Franc",          "GNF",       "324",
  "GUINEA-BISSAU",       "CFA Franc BCEAO",       "XOF",       "952",
  "GUYANA",              "Guyana Dollar",         "GYD",       "328",
  "HAITI",               "Gourde",                "HTG",       "332",
  "HAITI",               "US Dollar",             "USD",       "840",
  "HEARD ISLAND AND McDONALD ISLANDS", "Australian Dollar", "AUD", "036",
  "HOLY SEE (THE)",      "Euro",                  "EUR",       "978",
  "HONDURAS",            "Lempira",               "HNL",       "340",
  "HONG KONG",           "Hong Kong Dollar",      "HKD",       "344",
  "HUNGARY",             "Forint",                "HUF",       "348",
  "ICELAND",             "Iceland Krona",         "ISK",       "352",
  "INDIA",               "Indian Rupee",          "INR",       "356",
  "INDONESIA",           "Rupiah",                "IDR",       "360",
  "INTERNATIONAL MONETARY FUND (IMF)", "SDR (Special Drawing Right)", "XDR", "960",
  "IRAN (ISLAMIC REPUBLIC OF)", "Iranian Rial", "IRR", "364",
  "IRAQ",                "Iraqi Dinar",           "IQD",       "368",
  "IRELAND",             "Euro",                  "EUR",       "978",
  "ISLE OF MAN",         "Pound Sterling",        "GBP",       "826",
  "ISRAEL",              "New Israeli Sheqel",    "ILS",       "376",
  "ITALY",               "Euro",                  "EUR",       "978",
  "JAMAICA",             "Jamaican Dollar",       "JMD",       "388",
  "JAPAN",               "Yen",                   "JPY",       "392",
  "JERSEY",              "Pound Sterling",        "GBP",       "826",
  "JORDAN",              "Jordanian Dinar",       "JOD",       "400",
  "KAZAKHSTAN",          "Tenge",                 "KZT",       "398",
  "KENYA",               "Kenyan Shilling",       "KES",       "404",
  "KIRIBATI",            "Australian Dollar",     "AUD",       "036",
  "KOREA (THE DEMOCRATIC PEOPLE’S REPUBLIC OF)", "North Korean Won", "KPW", "408",
  "KOREA (THE REPUBLIC OF)", "Won",               "KRW",       "410",
  "KUWAIT",              "Kuwaiti Dinar",         "KWD",       "414",
  "KYRGYZSTAN",          "Som",                   "KGS",       "417",
  "LAO PEOPLE’S DEMOCRATIC REPUBLIC (THE)", "Kip", "LAK",      "418",
  "LATVIA",              "Euro",                  "EUR",       "978",
  "LEBANON",             "Lebanese Pound",        "LBP",       "422",
  "LESOTHO",             "Loti",                  "LSL",       "426",
  "LESOTHO",             "Rand",                  "ZAR",       "710",
  "LIBERIA",             "Liberian Dollar",       "LRD",       "430",
  "LIBYA",               "Libyan Dinar",          "LYD",       "434",
  "LIECHTENSTEIN",       "Swiss Franc",           "CHF",       "756",
  "LITHUANIA",           "Euro",                  "EUR",       "978",
  "LUXEMBOURG",          "Euro",                  "EUR",       "978",
  "MACAO",               "Pataca",                "MOP",       "446",
  "MADAGASCAR",          "Malagasy Ariary",       "MGA",       "969",
  "MALAWI",              "Kwacha",                "MWK",       "454",
  "MALAYSIA",            "Malaysian Ringgit",     "MYR",       "458",
  "MALDIVES",            "Rufiyaa",               "MVR",       "462",
  "MALI",                "CFA Franc BCEAO",       "XOF",       "952",
  "MALTA",               "Euro",                  "EUR",       "978",
  "MARSHALL ISLANDS (THE)", "US Dollar",          "USD",       "840",
  "MARTINIQUE",          "Euro",                  "EUR",       "978",
  "MAURITANIA",          "Ouguiya",               "MRU",       "929",
  "MAURITIUS",           "Mauritius Rupee",       "MUR",       "480",
  "MAYOTTE",             "Euro",                  "EUR",       "978",
  "MEMBER COUNTRIES OF THE AFRICAN DEVELOPMENT BANK GROUP", "ADB Unit of Account", "XUA", "965",
  "MEXICO",              "Mexican Peso",          "MXN",       "484",
  "MEXICO",              "Mexican Unidad de Inversion (UDI)", "MXV", "979",
  "MICRONESIA (FEDERATED STATES OF)", "US Dollar", "USD",      "840",
  "MOLDOVA (THE REPUBLIC OF)", "Moldovan Leu",    "MDL",       "498",
  "MONACO",              "Euro",                  "EUR",       "978",
  "MONGOLIA",            "Tugrik",                "MNT",       "496",
  "MONTENEGRO",          "Euro",                  "EUR",       "978",
  "MONTSERRAT",          "East Caribbean Dollar", "XCD",       "951",
  "MOROCCO",             "Moroccan Dirham",       "MAD",       "504",
  "MOZAMBIQUE", "Mozambique Metical",             "MZN",       "943",
  "MYANMAR",             "Kyat",                  "MMK",       "104",
  "NAMIBIA",             "Namibia Dollar",        "NAD",       "516",
  "NAMIBIA",             "Rand",                  "ZAR",       "710",
  "NAURU",               "Australian Dollar",     "AUD",       "036",
  "NEPAL",               "Nepalese Rupee",        "NPR",       "524",
  "NETHERLANDS (THE)",   "Euro",                  "EUR",       "978",
  "NEW CALEDONIA",       "CFP Franc",             "XPF",       "953",
  "NEW ZEALAND",         "New Zealand Dollar",    "NZD",       "554",
  "NICARAGUA",           "Cordoba Oro",           "NIO",       "558",
  "NIGER (THE)",         "CFA Franc BCEAO",       "XOF",       "952",
  "NIGERIA",             "Naira",                 "NGN",       "566",
  "NIUE",                "New Zealand Dollar",    "NZD",       "554",
  "NORFOLK ISLAND",      "Australian Dollar",     "AUD",       "036",
  "NORTHERN MARIANA ISLANDS (THE)", "US Dollar",  "USD",       "840",
  "NORWAY",              "Norwegian Krone",       "NOK",       "578",
  "OMAN",                "Rial Omani",            "OMR",       "512",
  "PAKISTAN",            "Pakistan Rupee",        "PKR",       "586",
  "PALAU",               "US Dollar",             "USD",       "840",
  "PANAMA",              "Balboa",                "PAB",       "590",
  "PANAMA",              "US Dollar",             "USD",       "840",
  "PAPUA NEW GUINEA",    "Kina",                  "PGK",       "598",
  "PARAGUAY",            "Guarani",               "PYG",       "600",
  "PERU",                "Nuevo Sol",             "PEN",       "604",
  "PHILIPPINES (THE)",   "Philippine Peso",       "PHP",       "608",
  "PITCAIRN",            "New Zealand Dollar",    "NZD",       "554",
  "POLAND",              "Zloty",                 "PLN",       "985",
  "PORTUGAL",            "Euro",                  "EUR",       "978",
  "PUERTO RICO",         "US Dollar",             "USD",       "840",
  "QATAR",               "Qatari Rial",           "QAR",       "634",
  "REPUBLIC OF NORTH MACEDONIA", "Denar",         "MKD",       "807",
  "ROMANIA",             "Romanian Leu",          "RON",       "946",
  "RUSSIAN FEDERATION (THE)", "Russian Ruble",    "RUB",       "643",
  "RWANDA",              "Rwanda Franc",          "RWF",       "646",
  "RÉUNION",             "Euro",                  "EUR",       "978",
  "SAINT BARTHÉLEMY",    "Euro",                  "EUR",       "978",
  "SAINT HELENA, ASCENSION AND TRISTAN DA CUNHA", "Saint Helena Pound", "SHP", "654",
  "SAINT KITTS AND NEVIS", "East Caribbean Dollar", "XCD",     "951",
  "SAINT LUCIA",         "East Caribbean Dollar", "XCD",       "951",
  "SAINT MARTIN (FRENCH PART)", "Euro",           "EUR",       "978",
  "SAINT PIERRE AND MIQUELON", "Euro",            "EUR",       "978",
  "SAINT VINCENT AND THE GRENADINES", "East Caribbean Dollar", "XCD", "951",
  "SAMOA",               "Tala",                  "WST",       "882",
  "SAN MARINO",          "Euro",                  "EUR",       "978",
  "SAO TOME AND PRINCIPE", "Dobra",               "STN",       "930",
  "SAUDI ARABIA",        "Saudi Riyal",           "SAR",       "682",
  "SENEGAL",             "CFA Franc BCEAO",       "XOF",       "952",
  "SERBIA",              "Serbian Dinar",         "RSD",       "941",
  "SEYCHELLES",          "Seychelles Rupee",      "SCR",       "690",
  "SIERRA LEONE",        "Leone",                 "SLE",       "925",
  "SINGAPORE",           "Singapore Dollar",      "SGD",       "702",
  "SINT MAARTEN (DUTCH PART)", "Caribbean guilder", "XCG",     "532",
  "SISTEMA UNITARIO DE COMPENSACION REGIONAL DE PAGOS \"SUCRE\"", "Sucre", "XSU", "994",
  "SLOVAKIA",            "Euro",                  "EUR",       "978",
  "SLOVENIA",            "Euro",                  "EUR",       "978",
  "SOLOMON ISLANDS",     "Solomon Islands Dollar", "SBD",      "090",
  "SOMALIA",             "Somali Shilling",       "SOS",       "706",
  "SOUTH AFRICA",        "Rand",                  "ZAR",       "710",
  "SOUTH SUDAN",         "South Sudanese Pound",  "SSP",       "728",
  "SPAIN",               "Euro",                  "EUR",       "978",
  "SRI LANKA",           "Sri Lanka Rupee",       "LKR",       "144",
  "SUDAN (THE)",         "Sudanese Pound",        "SDG",       "938",
  "SURINAME",            "Surinam Dollar",        "SRD",       "968",
  "SVALBARD AND JAN MAYEN", "Norwegian Krone",    "NOK",       "578",
  "SWAZILAND",           "Lilangeni",             "SZL",       "748",
  "SWEDEN",              "Swedish Krona",         "SEK",       "752",
  "SWITZERLAND",         "WIR Euro",              "CHE",       "947",
  "SWITZERLAND",         "Swiss Franc",           "CHF",       "756",
  "SWITZERLAND",         "WIR Franc",             "CHW",       "948",
  "SYRIAN ARAB REPUBLIC", "Syrian Pound",         "SYP",       "760",
  "TAIWAN (PROVINCE OF CHINA)", "New Taiwan Dollar", "TWD",    "901",
  "TAJIKISTAN",          "Somoni",                "TJS",       "972",
  "TANZANIA, UNITED REPUBLIC OF", "Tanzanian Shilling", "TZS", "834",
  "THAILAND",            "Baht",                  "THB",       "764",
  "TIMOR-LESTE",         "US Dollar",             "USD",       "840",
  "TOGO",                "CFA Franc BCEAO",       "XOF",       "952",
  "TOKELAU",             "New Zealand Dollar",    "NZD",       "554",
  "TONGA",               "Pa’anga",               "TOP",       "776",
  "TRINIDAD AND TOBAGO", "Trinidad and Tobago Dollar", "TTD",  "780",
  "TUNISIA",             "Tunisian Dinar",        "TND",       "788",
  "TURKEY",              "Turkish Lira",          "TRY",       "949",
  "TURKMENISTAN",        "Turkmenistan New Manat", "TMT",      "934",
  "TURKS AND CAICOS ISLANDS (THE)", "US Dollar",  "USD",       "840",
  "TUVALU",              "Australian Dollar",     "AUD",       "036",
  "UGANDA",              "Uganda Shilling",       "UGX",       "800",
  "UKRAINE",             "Hryvnia",               "UAH",       "980",
  "UNITED ARAB EMIRATES (THE)", "UAE Dirham",     "AED",       "784",
  "UNITED KINGDOM OF GREAT BRITAIN AND NORTHERN IRELAND (THE)", "Pound Sterling", "GBP", "826",
  "UNITED STATES MINOR OUTLYING ISLANDS (THE)", "US Dollar", "USD", "840",
  "UNITED STATES OF AMERICA (THE)", "US Dollar",  "USD",       "840",
  "UNITED STATES OF AMERICA (THE)", "US Dollar (Next day)", "USN", "997",
  "URUGUAY",             "Uruguay Peso en Unidades Indexadas (URUIURUI)", "UYI", "940",
  "URUGUAY",             "Peso Uruguayo",         "UYU",       "858",
  "UZBEKISTAN",          "Uzbekistan Sum",        "UZS",       "860",
  "VANUATU",             "Vatu",                  "VUV",       "548",
  "VENEZUELA (BOLIVARIAN REPUBLIC OF)", "Bolivar", "VEF",      "937",
  "VENEZUELA (BOLIVARIAN REPUBLIC OF)", "Bolivar", "VED",      "926",
  "VIET NAM",            "Dong",                  "VND",       "704",
  "VIRGIN ISLANDS (BRITISH)", "US Dollar",        "USD",       "840",
  "VIRGIN ISLANDS (U.S.)", "US Dollar",           "USD",       "840",
  "WALLIS AND FUTUNA",   "CFP Franc",             "XPF",       "953",
  "WESTERN SAHARA",      "Moroccan Dirham",       "MAD",       "504",
  "YEMEN",               "Yemeni Rial",           "YER",       "886",
  "ZAMBIA",              "Zambian Kwacha",        "ZMW",       "967",
  "ZIMBABWE",            "Zimbabwe Dollar",       "ZWL",       "932",
  "ÅLAND ISLANDS",       "Euro",                  "EUR",       "978"
};

/**
 * \brief returns human readable details for an ISSN
 * \param data_str String to be decoded
 * \return decoded ISSN string or NULL
 */
char * get_issn(char data_str[])
{
  int i, data_len = strlen(data_str);
  if (data_len < 10) return NULL;
  char * issn_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
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
  char * coupon_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
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
 * \brief returns the country for the given code number
 * \param data_str String to be decoded
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

  char * country_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
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

  char * currency_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
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

  if (application_identifier < 3900) return NULL;
  if (strlen(data_str) < 1) return NULL;
  int decimal_places = application_identifier % 10;
  if (decimal_places > 3) return NULL;
  /* check that the data is all numeric */
  int data_len = strlen(data_str);
  for (i = 0; i < data_len; i++) {
    if ((data_str[i] < '0') || (data_str[i] > '9')) return NULL;
  }

  char * decimal_str = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(unsigned char));
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
  char * app_id_str, * data_str, * date_str;
  char * curr_str, * decimal_str, * country_str, * coupon_str, * issn_str;
  char app_id_str2[10];
  unsigned char is_digital_link = 0;

  int data_length = (*application_data_end) - (*application_data_start);

  int curr_pos = (int)strlen(result);
  if (curr_pos != (*application_data_end)) {
    if (curr_pos <= 1) return;

    /* if the minimum data length has not yet arrived */
    if (*application_data_variable > 0) {
      if (curr_pos < (*application_data_end) - (*application_data_variable)) {
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
      *application_data_variable = 20;
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
      *application_data_variable = 20;
      break;
    }
    case 22: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 2;
      *application_data_variable = 20;
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
      *application_data_variable = 8;
      break;
    }
    case 41: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 3;
      break;
    }
    case 90: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 2;
      *application_data_variable = 30;
      break;
    }
    case 91: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 92: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 93: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 94: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 95: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 96: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 97: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 98: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 99: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 2;
      *application_data_variable = 90;
      break;
    }
    case 235: {
      *application_data_end = curr_pos + 28;
      *application_identifier_length = 3;
      *application_data_variable = 28;
      break;
    }
    case 240: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 241: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 242: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 3;
      *application_data_variable = 6;
      break;
    }
    case 243: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 250: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 251: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 253: {
      *application_data_end = curr_pos + 13 + 17;
      *application_identifier_length = 3;
      *application_data_variable = 17;
      break;
    }
    case 254: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 255: {
      *application_data_end = curr_pos + 13 + 12;
      *application_identifier_length = 3;
      *application_data_variable = 12;
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
    case 3370: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 3371: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 3372: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 3373: {
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
    case 3900: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3901: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3902: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3910: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3911: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3912: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3920: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3921: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3922: {
      *application_data_end = curr_pos + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3930: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3931: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 3932: {
      *application_data_end = curr_pos + 3 + 15;
      *application_identifier_length = 4;
      *application_data_variable = 15;
      break;
    }
    case 394: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      break;
    }
    case 3950: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 3951: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 3952: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 3953: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 400: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 401: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 402: {
      *application_data_end = curr_pos + 17;
      *application_identifier_length = 3;
      break;
    }
    case 403: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 3;
      *application_data_variable = 30;
      break;
    }
    case 410: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 411: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 412: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 413: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 414: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 415: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 3;
      break;
    }
    case 420: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 421: {
      *application_data_end = curr_pos + 3 + 9;
      *application_identifier_length = 3;
      *application_data_variable = 9;
      break;
    }
    case 422: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 3;
      break;
    }
    case 423: {
      *application_data_end = curr_pos + 3 + 12;
      *application_identifier_length = 3;
      *application_data_variable = 12;
      break;
    }
    case 424: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 3;
      break;
    }
    case 425: {
      *application_data_end = curr_pos + 3 + 12;
      *application_identifier_length = 3;
      *application_data_variable = 12;
      break;
    }
    case 426: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 3;
      break;
    }
    case 703: {
      *application_data_end = curr_pos + 3 + 27;
      *application_identifier_length = 4;
      *application_data_variable = 27;
      break;
    }
    case 710: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 711: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 712: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 713: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 714: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 715: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 716: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 3;
      *application_data_variable = 20;
      break;
    }
    case 4300: {
      *application_data_end = curr_pos + 35;
      *application_identifier_length = 4;
      *application_data_variable = 35;
      break;
    }
    case 4301: {
      *application_data_end = curr_pos + 35;
      *application_identifier_length = 4;
      *application_data_variable = 35;
      break;
    }
    case 4302: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4303: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4304: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4305: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4306: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4307: {
      *application_data_end = curr_pos + 2;
      *application_identifier_length = 4;
      break;
    }
    case 4308: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 4309: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 4310: {
      *application_data_end = curr_pos + 35;
      *application_identifier_length = 4;
      *application_data_variable = 35;
      break;
    }
    case 4311: {
      *application_data_end = curr_pos + 35;
      *application_identifier_length = 4;
      *application_data_variable = 35;
      break;
    }
    case 4312: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4313: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4314: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4315: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4316: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 4317: {
      *application_data_end = curr_pos + 2;
      *application_identifier_length = 4;
      break;
    }
    case 4318: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 4319: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 4320: {
      *application_data_end = curr_pos + 35;
      *application_identifier_length = 4;
      *application_data_variable = 35;
      break;
    }
    case 4321: {
      *application_data_end = curr_pos + 1;
      *application_identifier_length = 4;
      break;
    }
    case 4322: {
      *application_data_end = curr_pos + 1;
      *application_identifier_length = 4;
      break;
    }
    case 4323: {
      *application_data_end = curr_pos + 1;
      *application_identifier_length = 4;
      break;
    }
    case 4324: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      break;
    }
    case 4325: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      break;
    }
    case 4326: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 4330: {
      *application_data_end = curr_pos + 6 + 1;
      *application_identifier_length = 4;
      *application_data_variable = 1;
      break;
    }
    case 4331: {
      *application_data_end = curr_pos + 6 + 1;
      *application_identifier_length = 4;
      *application_data_variable = 1;
      break;
    }
    case 4332: {
      *application_data_end = curr_pos + 6 + 1;
      *application_identifier_length = 4;
      *application_data_variable = 1;
      break;
    }
    case 4333: {
      *application_data_end = curr_pos + 6 + 1;
      *application_identifier_length = 4;
      *application_data_variable = 1;
      break;
    }
    case 7001: {
      *application_data_end = curr_pos + 13;
      *application_identifier_length = 4;
      break;
    }
    case 7002: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 7003: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      break;
    }
    case 7004: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 7005: {
      *application_data_end = curr_pos + 12;
      *application_identifier_length = 4;
      *application_data_variable = 12;
      break;
    }
    case 7006: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 7007: {
      *application_data_end = curr_pos + 6 + 6;
      *application_identifier_length = 4;
      *application_data_variable = 6;
      break;
    }
    case 7008: {
      *application_data_end = curr_pos + 3;
      *application_identifier_length = 4;
      *application_data_variable = 3;
      break;
    }
    case 7009: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      *application_data_variable = 10;
      break;
    }
    case 7010: {
      *application_data_end = curr_pos + 2;
      *application_identifier_length = 4;
      *application_data_variable = 2;
      break;
    }
    case 7011: {
      *application_data_end = curr_pos + 6 + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 7020: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7021: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7022: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7023: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 7040: {
      *application_data_end = curr_pos + 1 + 3;
      *application_identifier_length = 4;
      break;
    }
    case 7041: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 7240: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 7241: {
      *application_data_end = curr_pos + 2;
      *application_identifier_length = 4;
      break;
    }
    case 7242: {
      *application_data_end = curr_pos + 25;
      *application_identifier_length = 4;
      *application_data_variable = 25;
      break;
    }
    case 7250: {
      *application_data_end = curr_pos + 8;
      *application_identifier_length = 4;
      break;
    }
    case 7251: {
      *application_data_end = curr_pos + 12;
      *application_identifier_length = 4;
      break;
    }
    case 7252: {
      *application_data_end = curr_pos + 1;
      *application_identifier_length = 4;
      break;
    }
    case 7253: {
      *application_data_end = curr_pos + 40;
      *application_identifier_length = 4;
      *application_data_variable = 40;
      break;
    }
    case 7254: {
      *application_data_end = curr_pos + 40;
      *application_identifier_length = 4;
      *application_data_variable = 40;
      break;
    }
    case 7255: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      *application_data_variable = 10;
      break;
    }
    case 7256: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 4;
      *application_data_variable = 90;
      break;
    }
    case 7257: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 7258: {
      *application_data_end = curr_pos + 1 + 1 + 1;
      *application_identifier_length = 4;
      break;
    }
    case 7259: {
      *application_data_end = curr_pos + 40;
      *application_identifier_length = 4;
      *application_data_variable = 40;
      break;
    }
    case 8001: {
      *application_data_end = curr_pos + 14;
      *application_identifier_length = 4;
      break;
    }
    case 8002: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 8003: {
      *application_data_end = curr_pos + 14 + 16;
      *application_identifier_length = 4;
      *application_data_variable = 16;
      break;
    }
    case 8004: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 8005: {
      *application_data_end = curr_pos + 6;
      *application_identifier_length = 4;
      break;
    }
    case 8006: {
      *application_data_end = curr_pos + 14 + 2 + 2;
      *application_identifier_length = 4;
      break;
    }
    case 8007: {
      *application_data_end = curr_pos + 34;
      *application_identifier_length = 4;
      *application_data_variable = 34;
      break;
    }
    case 8008: {
      *application_data_end = curr_pos + 8 + 4;
      *application_identifier_length = 4;
      *application_data_variable = 4;
      break;
    }
    case 8010: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 8011: {
      *application_data_end = curr_pos + 12;
      *application_identifier_length = 4;
      *application_data_variable = 12;
      break;
    }
    case 8012: {
      *application_data_end = curr_pos + 20;
      *application_identifier_length = 4;
      *application_data_variable = 20;
      break;
    }
    case 8013: {
      *application_data_end = curr_pos + 30;
      *application_identifier_length = 4;
      *application_data_variable = 30;
      break;
    }
    case 8017: {
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 4;
      break;
    }
    case 8018: {
      *application_data_end = curr_pos + 18;
      *application_identifier_length = 4;
      break;
    }
    case 8019: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 4;
      *application_data_variable = 10;
      break;
    }
    case 8020: {
      *application_data_end = curr_pos + 25;
      *application_identifier_length = 4;
      *application_data_variable = 25;
      break;
    }
    case 8026: {
      *application_data_end = curr_pos + 14 + 2 + 2;
      *application_identifier_length = 4;
      break;
    }
    case 8030: {
      *application_data_end = curr_pos + 90;
      *application_identifier_length = 4;
      *application_data_variable = 90;
      break;
    }
    case 8110: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 8111: {
      *application_data_end = curr_pos + 4;
      *application_identifier_length = 4;
      break;
    }
    case 8112: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 8200: {
      *application_data_end = curr_pos + 70;
      *application_identifier_length = 4;
      *application_data_variable = 70;
      break;
    }
    case 977: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 3;
      break;
    }
    case 978: {
      *application_data_end = curr_pos + 10;
      *application_identifier_length = 3;
      break;
    }
    case 979: {
      *application_data_end = curr_pos + 10;
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
    curr_str = NULL;
    decimal_str = NULL;
    country_str = NULL;
    coupon_str = NULL;
    issn_str = NULL;

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
      case 90: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 91: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 92: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 93: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 94: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 95: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 96: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 97: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 98: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
        }
        break;
      }
      case 99: {
        if (debug == 1) printf("INTERNAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "INTERNAL: ");
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
          coupon_str = get_coupon(data_str);
          if (coupon_str == NULL) {
            decode_strcat(gs1_result, "GCN: ");
          }
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
      case 3370: {
        if (debug == 1) printf("KG PER m2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "KG PER m2: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3371: {
        if (debug == 1) printf("KG PER m2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "KG PER m2: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3372: {
        if (debug == 1) printf("KG PER m2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "KG PER m2: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3373: {
        if (debug == 1) printf("KG PER m2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "KG PER m2: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
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
      case 3900: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3901: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3902: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3910: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
          curr_str = get_currency_value(*application_identifier, data_str);
        }
        break;
      }
      case 3911: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
          curr_str = get_currency_value(*application_identifier, data_str);
        }
        break;
      }
      case 3912: {
        if (debug == 1) printf("AMOUNT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AMOUNT: ");
          curr_str = get_currency_value(*application_identifier, data_str);
        }
        break;
      }
      case 3920: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3921: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3922: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3930: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
          curr_str = get_currency_value(*application_identifier, data_str);
        }
        break;
      }
      case 3931: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
          curr_str = get_currency_value(*application_identifier, data_str);
        }
        break;
      }
      case 3932: {
        if (debug == 1) printf("PRICE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE: ");
          curr_str = get_currency_value(*application_identifier, data_str);
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
      case 3950: {
        if (debug == 1) printf("PRICE/UoM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE/UoM: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3951: {
        if (debug == 1) printf("PRICE/UoM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE/UoM: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3952: {
        if (debug == 1) printf("PRICE/UoM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE/UoM: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
        }
        break;
      }
      case 3953: {
        if (debug == 1) printf("PRICE/UoM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE/UoM: ");
          decimal_str = get_decimal_value(*application_identifier, data_str);
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
      case 401: {
        if (debug == 1) printf("GINC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GINC: ");
        }
        break;
      }
      case 402: {
        if (debug == 1) printf("GSIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GSIN: ");
        }
        break;
      }
      case 403: {
        if (debug == 1) printf("ROUTE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ROUTE: ");
        }
        break;
      }
      case 410: {
        if (debug == 1) printf("SHIP TO LOC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO LOC: ");
        }
        break;
      }
      case 411: {
        if (debug == 1) printf("BILL TO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BILL TO: ");
        }
        break;
      }
      case 412: {
        if (debug == 1) printf("PURCHASE FROM ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PURCHASE FROM: ");
        }
        break;
      }
      case 413: {
        if (debug == 1) printf("SHIP FOR LOC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP FOR LOC: ");
        }
        break;
      }
      case 414: {
        if (debug == 1) printf("LOCN NO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "LOCN NO: ");
        }
        break;
      }
      case 415: {
        if (debug == 1) printf("PAY TO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PAY TO: ");
        }
        break;
      }
      case 420: {
        if (debug == 1) printf("SHIP TO POST ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO POST: ");
        }
        break;
      }
      case 421: {
        if (debug == 1) printf("SHIP TO POST ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO POST: ");
          country_str = get_country(data_str);
        }
        break;
      }
      case 422: {
        if (debug == 1) printf("ORIGIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ORIGIN: ");
        }
        break;
      }
      case 423: {
        if (debug == 1) printf("COUNTRY – INITIAL PROCESS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – INITIAL PROCESS: ");
        }
        break;
      }
      case 424: {
        if (debug == 1) printf("COUNTRY – PROCESS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – PROCESS: ");
        }
        break;
      }
      case 425: {
        if (debug == 1) printf("COUNTRY – DISASSEMBLY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – DISASSEMBLY: ");
        }
        break;
      }
      case 426: {
        if (debug == 1) printf("COUNTRY – FULL PROCESS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUNTRY – FULL PROCESS: ");
          country_str = get_country(data_str);
        }
        break;
      }
      case 703: {
        if (debug == 1) printf("PROCESSOR # s4 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROCESSOR # s4: ");
          country_str = get_country(data_str);
        }
        break;
      }
      case 710: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 711: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 712: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 713: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 714: {
        if (debug == 1) printf("NHRN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN: ");
        }
        break;
      }
      case 715: {
        if (debug == 1) printf("NHRN NDC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN NDC: ");
        }
        break;
      }
      case 716: {
        if (debug == 1) printf("NHRN AIC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NHRN AIC: ");
        }
        break;
      }
      case 4300: {
        if (debug == 1) printf("SHIP TO COMP ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO COMP: ");
        }
        break;
      }
      case 4301: {
        if (debug == 1) printf("SHIP TO NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO NAME: ");
        }
        break;
      }
      case 4302: {
        if (debug == 1) printf("SHIP TO ADD1 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO ADD1: ");
        }
        break;
      }
      case 4303: {
        if (debug == 1) printf("SHIP TO ADD2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO ADD2: ");
        }
        break;
      }
      case 4304: {
        if (debug == 1) printf("SHIP TO SUB ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO SUB: ");
        }
        break;
      }
      case 4305: {
        if (debug == 1) printf("SHIP TO LOC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO LOC: ");
        }
        break;
      }
      case 4306: {
        if (debug == 1) printf("SHIP TO REG ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO REG: ");
        }
        break;
      }
      case 4307: {
        if (debug == 1) printf("SHIP TO COUNTRY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO COUNTRY: ");
        }
        break;
      }
      case 4308: {
        if (debug == 1) printf("SHIP TO PHONE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO PHONE: ");
        }
        break;
      }
      case 4309: {
        if (debug == 1) printf("SHIP TO GEO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SHIP TO GEO: ");
        }
        break;
      }
      case 4310: {
        if (debug == 1) printf("RTN TO COMP ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO COMP: ");
        }
        break;
      }
      case 4311: {
        if (debug == 1) printf("RTN TO NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO NAME: ");
        }
        break;
      }
      case 4312: {
        if (debug == 1) printf("RTN TO ADD1 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO ADD1: ");
        }
        break;
      }
      case 4313: {
        if (debug == 1) printf("RTN TO ADD2 ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO ADD2: ");
        }
        break;
      }
      case 4314: {
        if (debug == 1) printf("RTN TO SUB ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO SUB: ");
        }
        break;
      }
      case 4315: {
        if (debug == 1) printf("RTN TO LOC ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO LOC: ");
        }
        break;
      }
      case 4316: {
        if (debug == 1) printf("RTN TO REG ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO REG: ");
        }
        break;
      }
      case 4317: {
        if (debug == 1) printf("RTN TO COUNTRY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO COUNTRY: ");
        }
        break;
      }
      case 4318: {
        if (debug == 1) printf("RTN TO POST ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO POST: ");
        }
        break;
      }
      case 4319: {
        if (debug == 1) printf("RTN TO PHONE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "RTN TO PHONE: ");
        }
        break;
      }
      case 4320: {
        if (debug == 1) printf("SRV DESCRIPTION ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SRV DESCRIPTION: ");
        }
        break;
      }
      case 4321: {
        if (debug == 1) printf("DANGEROUS GOODS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DANGEROUS GOODS: ");
        }
        break;
      }
      case 4322: {
        if (debug == 1) printf("AUTH LEAVE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AUTH LEAVE: ");
        }
        break;
      }
      case 4323: {
        if (debug == 1) printf("SIG REQUIRED ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SIG REQUIRED: ");
        }
        break;
      }
      case 4324: {
        if (debug == 1) printf("NBEF DEL DT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NBEF DEL DT: ");
        }
        break;
      }
      case 4325: {
        if (debug == 1) printf("NAFT DEL DT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NAFT DEL DT: ");
        }
        break;
      }
      case 4326: {
        if (debug == 1) printf("REL DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REL DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 4330: {
        if (debug == 1) printf("MAX TEMP F ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MAX TEMP F: ");
        }
        break;
      }
      case 4331: {
        if (debug == 1) printf("MAX TEMP C ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MAX TEMP C: ");
        }
        break;
      }
      case 4332: {
        if (debug == 1) printf("MIN TEMP F ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MIN TEMP F: ");
        }
        break;
      }
      case 4333: {
        if (debug == 1) printf("MIN TEMP C ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MIN TEMP C: ");
        }
        break;
      }
      case 7001: {
        if (debug == 1) printf("NSN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "NSN: ");
        }
        break;
      }
      case 7002: {
        if (debug == 1) printf("MEAT CUT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "MEAT CUT: ");
        }
        break;
      }
      case 7003: {
        if (debug == 1) printf("EXPIRY TIME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "EXPIRY TIME: ");
          date_str = data_id_convert_date("YYMMDDHHMM", data_str);
        }
        break;
      }
      case 7004: {
        if (debug == 1) printf("ACTIVE POTENCY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ACTIVE POTENCY: ");
        }
        break;
      }
      case 7005: {
        if (debug == 1) printf("CATCH AREA ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CATCH AREA: ");
        }
        break;
      }
      case 7006: {
        if (debug == 1) printf("FIRST FREEZE DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FIRST FREEZE DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 7007: {
        if (debug == 1) printf("HARVEST DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "HARVEST DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 7008: {
        if (debug == 1) printf("AQUATIC SPECIES ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AQUATIC SPECIES: ");
        }
        break;
      }
      case 7009: {
        if (debug == 1) printf("FISHING GEAR TYPE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FISHING GEAR TYPE: ");
        }
        break;
      }
      case 7010: {
        if (debug == 1) printf("PROD METHOD ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROD METHOD: ");
        }
        break;
      }
      case 7011: {
        if (debug == 1) printf("TEST BY DATE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "TEST BY DATE: ");
          date_str = data_id_convert_date("YYMMDD", data_str);
        }
        break;
      }
      case 7020: {
        if (debug == 1) printf("REFURB LOT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REFURB LOT: ");
        }
        break;
      }
      case 7021: {
        if (debug == 1) printf("FUNC STAT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FUNC STAT: ");
        }
        break;
      }
      case 7022: {
        if (debug == 1) printf("REV STAT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REV STAT: ");
        }
        break;
      }
      case 7023: {
        if (debug == 1) printf("GIAI – ASSEMBLY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GIAI – ASSEMBLY: ");
        }
        break;
      }
      case 7040: {
        if (debug == 1) printf("UIC+EXT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "UIC+EXT: ");
        }
        break;
      }
      case 7041: {
        if (debug == 1) printf("UFRGT UNIT TYPE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "UFRGT UNIT TYPE: ");
        }
        break;
      }
      case 7240: {
        if (debug == 1) printf("PROTOCOL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROTOCOL: ");
        }
        break;
      }
      case 7241: {
        if (debug == 1) printf("AIDC MEDIA TYPE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "AIDC MEDIA TYPE: ");
        }
        break;
      }
      case 7242: {
        if (debug == 1) printf("VCN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VCN: ");
        }
        break;
      }
      case 7250: {
        if (debug == 1) printf("DOB ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DOB: ");
          date_str = data_id_convert_date("YYYYMMDD", data_str);
        }
        break;
      }
      case 7251: {
        if (debug == 1) printf("DOB TIME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DOB TIME: ");
          date_str = data_id_convert_date("YYYYMMDDHHMM", data_str);
        }
        break;
      }
      case 7252: {
        if (debug == 1) printf("BIO SEX ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BIO SEX: ");
        }
        break;
      }
      case 7253: {
        if (debug == 1) printf("FAMILY NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FAMILY NAME: ");
        }
        break;
      }
      case 7254: {
        if (debug == 1) printf("GIVEN NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GIVEN NAME: ");
        }
        break;
      }
      case 7255: {
        if (debug == 1) printf("SUFFIX ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SUFFIX: ");
        }
        break;
      }
      case 7256: {
        if (debug == 1) printf("FULL NAME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "FULL NAME: ");
        }
        break;
      }
      case 7257: {
        if (debug == 1) printf("PERSON ADDR ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PERSON ADDR: ");
        }
        break;
      }
      case 7258: {
        if (debug == 1) printf("BIRTH SEQUENCE ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BIRTH SEQUENCE: ");
        }
        break;
      }
      case 7259: {
        if (debug == 1) printf("BABY ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "BABY: ");
        }
        break;
      }
      case 8001: {
        if (debug == 1) printf("DIMENSIONS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DIMENSIONS: ");
        }
        break;
      }
      case 8002: {
        if (debug == 1) printf("CMT NO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CMT NO: ");
        }
        break;
      }
      case 8003: {
        if (debug == 1) printf("GRAI ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GRAI: ");
        }
        break;
      }
      case 8004: {
        if (debug == 1) printf("GIAI ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GRAI: ");
        }
        break;
      }
      case 8005: {
        if (debug == 1) printf("PRICE PER UNIT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRICE PER UNIT: ");
        }
        break;
      }
      case 8006: {
        if (debug == 1) printf("ITIP ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ITIP: ");
        }
        break;
      }
      case 8007: {
        if (debug == 1) printf("IBAN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "IBAN: ");
        }
        break;
      }
      case 8008: {
        if (debug == 1) printf("PROD TIME ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PROD TIME: ");
          date_str = data_id_convert_date("YYMMDDHH", data_str);
        }
        break;
      }
      case 8010: {
        if (debug == 1) printf("CPID ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CPID: ");
        }
        break;
      }
      case 8011: {
        if (debug == 1) printf("CPID SERIAL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "CPID SERIAL: ");
        }
        break;
      }
      case 8012: {
        if (debug == 1) printf("VERSION ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "VERSION: ");
        }
        break;
      }
      case 8013: {
        if (debug == 1) printf("GMN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GMN: ");
        }
        break;
      }
      case 8017: {
        if (debug == 1) printf("GSRN - PROVIDER ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GSRN - PROVIDER: ");
        }
        break;
      }
      case 8018: {
        if (debug == 1) printf("GSRN - RECIPIENT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "GSRN - RECIPIENT: ");
        }
        break;
      }
      case 8019: {
        if (debug == 1) printf("SRIN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "SRIN: ");
        }
        break;
      }
      case 8020: {
        if (debug == 1) printf("REF NO ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "REF NO: ");
        }
        break;
      }
      case 8026: {
        if (debug == 1) printf("ITIP CONTENT ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ITIP CONTENT: ");
        }
        break;
      }
      case 8030: {
        if (debug == 1) printf("DIGSIG ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "DIGSIG: ");
        }
        break;
      }
      case 8110: {
        if (debug == 1) printf("COUPON ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUPON: ");
        }
        break;
      }
      case 8111: {
        if (debug == 1) printf("POINTS ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "POINTS: ");
        }
        break;
      }
      case 8112: {
        if (debug == 1) printf("COUPON ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "COUPON: ");
        }
        break;
      }
      case 8200: {
        if (debug == 1) printf("PRODUCT URL ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "PRODUCT URL: ");
        }
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
        if (debug == 1) printf("ISBN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ISBN: ");
        }
        break;
      }
      case 979: {
        if (debug == 1) printf("ISBN ");
        if (is_digital_link == 0) {
          decode_strcat(gs1_result, "ISBN: ");
        }
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
        else if (coupon_str != NULL) {
          decode_strcat(gs1_result, coupon_str);
          free(coupon_str);
        }
        else if (issn_str != NULL) {
          decode_strcat(gs1_result, issn_str);
          free(issn_str);
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
