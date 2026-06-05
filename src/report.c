/*********************************************************************
 * Software License Agreement (GPLv3)
 *
 *  Datamatrix verification report
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

/**
 * \brief saves a verification report
 * \param grid grid object
 * \param address_line1 First line of address to appear on verification report
 * \param address_line2 Second line of address to appear on verification report
 * \param address_line3 Third line of address to appear on verification report
 * \param phone Phone number to appear on verification report
 * \param email Email address to appear on verification report
 * \param website Web address to appear on verification report
 * \param grid_filename Image to be shown in the report
 * \param histogram_filename Reflectance histogram image to be shown in the report
 * \param report_template Filename for a TEX template
 * \param report_filename Filename to save the report as
 * \param logo_filename Filename for logo to display at top of report
 * \param decode_result decoded text
 * \param aperture Aperture reference number from ISO 15416
 * \param light_nm Peak light wavelength used in nanometres
 * \param light_angle_degrees Angle of illumination in degrees
 * \param footer Footer text on verification report
 */
void save_verification_report(struct grid_2d * grid,
                              char address_line1[],
                              char address_line2[],
                              char address_line3[],
                              char phone[],
                              char email[],
                              char website[],
                              char grid_filename[],
                              char histogram_filename[],
                              char report_template[],
                              char report_filename[],
                              char logo_filename[],
                              char decode_result[],
                              float aperture,
                              int light_nm,
                              int light_angle_degrees,
                              char footer[])
{
    FILE * fp_template, * fp_report;
    char * line = NULL;
    unsigned char grade = overall_quality_grade(grid);
    char grade_letter[] = {'F', 'D', 'C', 'B', 'A'};
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fp_template = fopen(report_template, "r");
    if (fp_template == NULL) return;

    fp_report = fopen(report_filename, "w");
    if (fp_report == NULL) return;

    line = (char*)safemalloc(MAX_DECODE_LENGTH*sizeof(char));

    while (getline2(line, fp_template) != -1) {
        if (strstr(line, "\\newcommand{") != NULL) {
            /* address line 1 */
            if (strstr(line, "{\\addressa}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\addressa}{%s}\n", address_line1);
                continue;
            }
            /* address line 2 */
            if (strstr(line, "{\\addressb}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\addressb}{%s}\n", address_line2);
                continue;
            }
            /* address line 3 */
            if (strstr(line, "{\\addressc}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\addressc}{%s}\n", address_line3);
                continue;
            }
            /* phone */
            if (strstr(line, "{\\phone}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\phone}{%s}\n", phone);
                continue;
            }
            /* email */
            if (strstr(line, "{\\email}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\email}{%s}\n", email);
                continue;
            }
            /* website */
            if (strstr(line, "{\\website}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\website}{%s}\n", website);
                continue;
            }
            /* issue date */
            if (strstr(line, "{\\issuedate}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\issuedate}{%d-%02d-%02d}\n",
                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                continue;
            }
            /* symbol type */
            if (strstr(line, "{\\symboltype}") != NULL) {
                if (grid->gs1_datamatrix == 1) {
                    fprintf(fp_report, "\\newcommand{\\symboltype}{GS1 datamatrix}\n");
                }
                else {
                    fprintf(fp_report, "\\newcommand{\\symboltype}{Datamatrix}\n");
                }
                continue;
            }
            /* matrix size */
            if (strstr(line, "{\\matrixsize}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\matrixsize}{%dx%d}\n",
                        grid->dimension_x, grid->dimension_y);
                continue;
            }
            /* overall grade A-F */
            if (strstr(line, "{\\overallgrade}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\overallgrade}{%c}\n",
                        grade_letter[grade]);
                continue;
            }
            /* decode result */
            if (strstr(line, "{\\decoderesult}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\decoderesult}{");
                for (int i = 0; i < strlen(decode_result); i++) {
                    if (decode_result[i] == 29) {
                        fprintf(fp_report, "<GS>");
                    }
                    else if (decode_result[i] == 30) {
                        fprintf(fp_report, "<RS>");
                    }
                    else if (decode_result[i] == 31) {
                        fprintf(fp_report, "<US>");
                    }
                    else if (decode_result[i] == 28) {
                        fprintf(fp_report, "<FS>");
                    }
                    else if (decode_result[i] == 4) {
                        fprintf(fp_report, "<EOT>");
                    }
                    else if (decode_result[i] == '_') {
                        fprintf(fp_report, "\\_");
                    }
                    else if (decode_result[i] == '\\') {
                        fprintf(fp_report, "\\textbackslash");
                    }
                    else if (decode_result[i] == '%') {
                        fprintf(fp_report, "\\%%");
                    }
                    else {
                        fprintf(fp_report, "%c", decode_result[i]);
                    }
                }
                fprintf(fp_report, "}\n");
                continue;
            }
            /* ISO Symbol Grade 0.0 - 4.0 */
            if (strstr(line, "{\\isosymbolgrade}") != NULL) {
                if (grade > 0) {
                    fprintf(fp_report, "\\newcommand{\\isosymbolgrade}{%d.0 PASS}\n",
                            (int)grade);
                }
                else {
                    fprintf(fp_report, "\\newcommand{\\isosymbolgrade}{%d.0 FAIL}\n",
                            (int)grade);
                }
                continue;
            }
            /* symbol contrast */
            if (strstr(line, "{\\symbolcontrast}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\symbolcontrast}{%c (%d\\%%)}\n",
                        grade_letter[(int)grid->symbol_contrast_grade],
                        (int)grid->symbol_contrast);
                continue;
            }
            /* modulation */
            if (strstr(line, "{\\modulation}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\modulation}{%c (%d\\%%)}\n",
                        grade_letter[(int)grid->modulation_grade],
                        (int)grid->modulation);
                continue;
            }
            /* axial non-uniformity */
            if (strstr(line, "{\\axialnonuniformity}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\axialnonuniformity}{%c (%.1f\\%%)}\n",
                        grade_letter[(int)grid->axial_non_uniformity_grade],
                        grid->axial_non_uniformity);
                continue;
            }
            /* grid non-uniformity */
            if (strstr(line, "{\\gridnonuniformity}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\gridnonuniformity}{%c (%.1f\\%%)}\n",
                        grade_letter[(int)grid->grid_non_uniformity_grade],
                        grid->grid_non_uniformity);
                continue;
            }
            /* unused error correction */
            if (strstr(line, "{\\unusederrorcorrection}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\unusederrorcorrection}{%c (%d\\%%)}\n",
                        grade_letter[(int)grid->unused_error_correction_grade],
                        (int)grid->unused_error_correction);
                continue;
            }
            /* fixed pattern damage */
            if (strstr(line, "{\\fixedpatterndamage}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\fixedpatterndamage}{%c (%d\\%%)}\n",
                        grade_letter[(int)grid->fixed_pattern_damage_grade],
                        (int)grid->fixed_pattern_damage);
                continue;
            }
            /* clock track regularity */
            if (strstr(line, "{\\clocktrackregularity}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\clocktrackregularity}{%c (%d\\%%)}\n",
                        grade_letter[(int)grid->clock_track_regularity_grade],
                        (int)grid->clock_track_regularity);
                continue;
            }
            /* minimum reflectance */
            if (strstr(line, "{\\minreflectance}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\minreflectance}{%c (%d\\%%)}\n",
                        grade_letter[(int)grid->minimum_reflectance_grade],
                        (int)grid->minimum_reflectance);
                continue;
            }
            /* aperture */
            if (aperture > 0) {
                if (strstr(line, "{\\lightaperture}") != NULL) {
                    fprintf(fp_report, "\\newcommand{\\lightaperture}{%.1f}\n",
                            aperture);
                    continue;
                }
            }
            /* light wavelength nm */
            if (strstr(line, "{\\lightnm}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\lightnm}{%d}\n",
                        light_nm);
                continue;
            }
            /* light angle degrees */
            if (strstr(line, "{\\lightangle}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\lightangle}{%d}\n",
                        light_angle_degrees);
                continue;
            }
            /* angle of distortion */
            if (strstr(line, "{\\angleofdistortion}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\angleofdistortion}{%.1f}\n",
                        grid->angle_of_distortion);
                continue;
            }
            /* contrast uniformity */
            if (strstr(line, "{\\contrastuniformity}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\contrastuniformity}{%d}\n",
                        (int)grid->contrast_uniformity);
                continue;
            }
            /* dots per element */
            if (strstr(line, "{\\dotsperelement}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\dotsperelement}{%d}\n",
                        grid->dots_per_element);
                continue;
            }
            /* elongation */
            if (strstr(line, "{\\elongation}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\elongation}{%.1f}\n",
                        grid->elongation);
                continue;
            }
            /* quiet zone */
            if (strstr(line, "{\\quietzone}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\quietzone}{%d}\n",
                        (int)grid->quiet_zone);
                continue;
            }
            /* distributed damage */
            if (strstr(line, "{\\distributeddamage}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\distributeddamage}{%d}\n",
                        (int)grid->distributed_damage);
                continue;
            }
            /* cell fill */
            if (strstr(line, "{\\cellfill}") != NULL) {
                fprintf(fp_report, "\\newcommand{\\cellfill}{%d}\n",
                        (int)grid->cell_fill);
                continue;
            }
        }
        /* logo image at top of report */
        if (strlen(logo_filename) > 0) {
            if (strstr(line, "{img/logo_square.png}") != NULL) {
                fprintf(fp_report, "\\includegraphics[height=2.5cm,clip]{%s}\n",
                        logo_filename);
                continue;
            }
        }
        /* detected symbol image */
        if (strstr(line, "{img/datamatrix.png}") != NULL) {
            fprintf(fp_report, "    \\includegraphics[height=6cm]{%s}\n",
                    grid_filename);
            continue;
        }
        /* reflectance histogram image */
        if (strstr(line, "{histogram.png}") != NULL) {
            fprintf(fp_report, "    \\includegraphics[height=6cm]{%s}\n",
                    histogram_filename);
            continue;
        }
        /* footer text */
        if (strlen(footer) > 0) {
            if (strstr(line, "\\fancyfoot") != NULL) {
                fprintf(fp_report, "\\fancyfoot[C]{%s}\n", footer);
                continue;
            }
        }
        fprintf(fp_report, "%s", line);
    }

    free(line);
    fclose(fp_template);
    fclose(fp_report);
}
