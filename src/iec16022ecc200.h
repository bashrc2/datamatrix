// IEC16022 encoder, see IEC16022
// This software is provided under the terms of the GPL v2 or later.
// This software is provided free of charge with a full "Money back" guarantee.
// Use entirely at your own risk. We accept no liability. If you don't like that - don't use it.

// $Log: iec16022ecc200.h,v $
// Revision 1.4  2006/01/25 11:10:43  cvs
// New sqlhtml
//
// Revision 1.3  2004/09/09 07:45:09  cvs
// Added change history to source files
// Added "info" type to IEC16022
// Added exact size checking shortcodes on encoding generation for iec16022
//

#define MAXBARCODE 3116
typedef struct {
   unsigned int *Wptr;          // Width (pre fill 0 for auto, sets actual)
   unsigned int *Hptr;          // Height (pre fill 0 for auto, sets actual)
   char **encodingptr;          // Fills in encoding string if set
   unsigned int barcodelen;     // Lengths of code
   unsigned char *barcode;      // Code string
   unsigned int *lenp;          // Stores data length before any final unlatch or pad
   unsigned int *maxp;          // Stores max storage of this size code
   unsigned int *eccp;          // Stores the number of ecc bytes used in this size
   unsigned char square:1;      // Force square (also setting Wptr==Hptr does this)
   unsigned char noquiet:1;     // No quiet area (normally 1 unit all around is included)
} iec16022ecc200_t;
#define iec16022ecc200(...) iec16022ecc200_opts((iec16022ecc200_t){__VA_ARGS__})
unsigned char *iec16022ecc200_opts(iec16022ecc200_t);
