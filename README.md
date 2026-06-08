<img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/logo_square3.png" width="25%" alt="Datamatrix under a magnifying glass"/>

*datamatrix* is a commandline utility for generating, reading, and verifying Datamatrix direct part markings. Some image processing functions are based on [visionutils](https://codeberg.org/bashrc/visionutils).

## Key features

 * Generates datamatrix codes in text and image formats
 * Reads PNG files (recommended at least 800 pixels across) using [lodepng](https://lodev.org/lodepng)
 * Outputs the plain text encoded by the datamatrix pattern
 * Implemented in pure [C99](https://en.wikipedia.org/wiki/C99)
 * Supports [GS1 semantics](https://www.gs1.org/standards/barcodes/2d) and digital links
 * Supports [ANSI/HIBC](https://www.hibcc.org/wp-content/uploads/SLS-2.6-Final.pdf) health industry supplier labeling standard
 * Can generate quality metrics for verification reports
 * Parallelized with [OpenMP](https://www.openmp.org)
 * No dependencies or software supply chains!

## Build

Install the build system:

``` bash
sudo apt install build-essential
```

Then compile.

``` bash
make
```

And finally install.

``` bash
sudo make install
```

## Command options

``` text
-f --filename [PNG file]       Filename of a PNG file containing the datamatrix image
--squaredim                    Matrix aspect ratio is expected to be square
--rectangle                    Matrix aspect ratio is expected to be rectangular
--quality                      Show quality metrics for verification reports
-o --output [PNG file]         Save an image showing the detected datamatrix
-g --grid [PNG file]           Save an image showing the detected grid
--hist [PNG file]              Save reflectance histogram
--shape [PNG file]             Save cell shape variance image
-t --template [TEX file]       Path for verification report template
-r --report [TEX file]         Filename to save a verification report as
-l --logo [image file]         Logo image to be shown at top of verification report
--histcentres                  Only sample grid cell centres for reflectance histogram
--mingrid [8..144]             Minimum matrix dimension
--maxgrid [8..144]             Maximum matrix dimension
--debug                        Show debugging information
--csv                          Show quality metrics in CSV format
--json                         Show quality metrics in JSON format
--yaml                         Show quality metrics in yaml format
--freq [8..144]                Set the matrix dimension
--erode [0..20]                Set erosion itterations
--dilate [0..20]               Set dilation itterations
--meanlight [0..20]            Set mean light threshold
--darklight [1..10]            Sampling step size when calculating peak dark and light
--maxhigh [6..90]              Maximum percent of high pixels after thresholding
--resolver [url]               GS1 resolver (eg. https://id.gs1.org)
--sampleradius [pixels]        Radius for sampling each grid cell
--minsegmentlength [pixels]    Minimum edge segment length for segment joining
--resizewidth [pixels]         Width of image after resizing from original
--resizeheight [pixels]        Height of image after resizing from original
--binwidth [pixels]            Width of the binary image used for perimeter detection
--raw                          Show the raw decoded text
--aperture [ref]               Aperture reference number from ISO 15416
--light [nm]                   Peak light wavelength used in nanometres
--angle [90|45]                Angle of illumination in degrees
--address1 [text]              First line of address on verification report
--address2 [text]              Second line of address on verification report
--address3 [text]              Third line of address on verification report
--phone [number]               Telephone number on verification report
--email [address]              Email address on verification report
--web [url]                    Website url on verification report
--footer [text]                Footer shown on verification report
-e --encode [text]             Generate a datamatrix from the given text
--sscc [code]                  Generate a GS1 datamatrix with the given SSCC
--gtin [code]                  Generate a GS1 datamatrix with the given GTIN
--content [code]               Generate a GS1 datamatrix with the given CONTENT
--mtogtin [code]               Generate a GS1 datamatrix with the given MTO GTIN
--batch [code]                 Generate a GS1 datamatrix with the given BATCH/LOT
--proddate [YYMMDD]            Generate a GS1 datamatrix with the given PROD DATE
--duedate [YYMMDD]             Generate a GS1 datamatrix with the given DUE DATE
--packdate [YYMMDD]            Generate a GS1 datamatrix with the given PACK DATE
--bestbefore [YYMMDD]          Generate a GS1 datamatrix with the given BEST BEFORE
--sellby [YYMMDD]              Generate a GS1 datamatrix with the given SELL BY
--useby [YYMMDD]               Generate a GS1 datamatrix with the given USE BY
--variant [two digits]         Generate a GS1 datamatrix with the given VARIANT
--serial [number]              Generate a GS1/HIBC datamatrix with the given SERIAL
--custpartno [number]          Generate a GS1 datamatrix with the given CUST PART NO
--mtovariant [code]            Generate a GS1 datamatrix with the given MTO VARIANT
--gdti [code]                  Generate a GS1 datamatrix with the given GDTI
--gln [number]                 Generate a GS1 datamatrix with the given GLN
--gcn [number]                 Generate a GS1 datamatrix with the given GCN
--orderno [number]             Generate a GS1 datamatrix with the given ORDER NUMBER
--gsin [code]                  Generate a GS1 datamatrix with the given GSIN
--shipto [code]                Generate a GS1 datamatrix with the given SHIP TO LOC
--billto [code]                Generate a GS1 datamatrix with the given BILL TO
--origin [code]                Generate a GS1 datamatrix with the given ORIGIN
--labeler [4 characters]       Generate a HIBC datamatrix with the given LABELER ID
--prodid [4 characters]        Generate a HIBC datamatrix with the given PRODUCT ID
--unit [1 character]           Generate a HIBC datamatrix with the given UNIT OF MEASURE
--expiryjulian [YYJJJ]         Generate a HIBC datamatrix with the given EXPIRY julian day
--expiry [YYMMDD]              Generate a HIBC datamatrix with the given EXPIRY
--expiryjour [YYMMDDHH]        Generate a HIBC datamatrix with the given EXPIRY hour
--expiration [YYYYMMDD]        Generate a HIBC datamatrix with the given EXPIRATION DATE
--mandate [YYYYMMDD]           Generate a HIBC datamatrix with the given MANUFACTURE DATE
--lot [4 characters]           Generate a HIBC datamatrix with the given LOT NUMBER
--qty [1-5 digits]             Generate a HIBC datamatrix with the given QUANTITY
--txt [text]                   Decode a text string containing a datamatrix
--squaremodules                Generated datamatrix has square modules
--dot [character]              Custom dot character when generating a datamatrix
--space [character]            Custom space character when generating a datamatrix
--scale [text]                 Scale of generated datamatrix in range 1-10
-w --width [pixels]            Width for generated datamatrix image
--coords                       Show dot centre coordinates for generated datamatrix
--offsetx [value]              x offset added to dot centre coordinates
--offsety [value]              y offset added to dot centre coordinates
--seglink [value]              Pixel radius used to link edge segments
--pos [above|below|right|left] Position of text within encoded datamatrix image
--charwidth [pixels]           Width of characters shown on encoded datamatrix image
--linespacing [pixels]         Line spacing for text on encoded datamatrix image
--tests                        Run unit tests
```

## Example Encoding

To generate a datamatrix pattern from some text.

``` bash
datamatrix -e "ABCDEF"
```

You can also GS1 encode a datamatrix using the typical fields.

``` bash
datamatrix --gtin 12345678901234 --batch 3725a --serial 738235495
```

Or use HIBC encoding:

``` bash
datamatrix --labeler A999 --prodid 1234 --unit 5 --expiry 26034 --mandate 20260116
```

You can specify how dots and spaces appear.

``` bash
datamatrix -e "ABCDEF" --dot "●" --space " "
```

And show square modules rather than round.

``` bash
datamatrix -e "ABCDEF" --squaremodules
```

The datamatrix can be forced to square dimensions.

``` bash
datamatrix -e "user@email.domain" --squaredim
```

You can output the dot centre coordinates in the range 0-1000, optionally with offsets.

``` bash
datamatrix -e "ABCDEF" --coords -w 1000 --offsetx 50 --offsety 30
```

Optionally you can output the generated datamatrix pattern in csv format.

``` bash
datamatrix -e "ABCDEF" --csv
```

You can also export the generated datamatrix as an image in PNG or SVG format.

``` bash
datamatrix --gtin 12345678901234 --pos right --charwidth 20 -o test.png -w 512
datamatrix --labeler A999 --prodid 1234 --unit 5 --expiry 26034 --mandate 20260116 -o test.png --charwidth 12
```

## Example Decoding

It's possible to decode a datamatrix contained within a text string, as generated with the *-e* option above.

``` bash
datamatrix --txt "datamatrix text goes here"
```

When decoding, you typically only need to supply a PNG formatted input image.

``` bash
datamatrix -f examples/01.png
```

When possible, decoded text will appear in a more verbose human readable form. However if you want to know what text the datamatrix directly encodes then you can use the *--raw* option.

``` bash
datamatrix -f examples/01.png --raw
```

When decoding GS1 datamatrix symbols you can also specify a [GS1 resolver](https://www.gs1.org/standards/resolver) if you want a digital link to be returned.

``` bash
datamatrix -f examples/01.png --resolver https://id.gs1.org
```

If you set the resolver as a '.' character then the GS1 code will be shown with brackets, such as *(01)00068780000108(13)301231(10)ABC123*.

``` bash
datamatrix -f examples/01.png --resolver .
```

If you know the range of grid dimensions which are expected then you can narrow down the search using the *--mingrid* and *--maxgrid* options.

``` bash
datamatrix -f examples/01.png --mingrid 10 --maxgrid 20
```

If needed, an image can be saved showing the detected datamatrix region and also the grid.

``` bash
datamatrix -f examples/01.png -o detection.png -g grid.png
```

If you want to calculate quality metrics for symbol verification purposes:

``` bash
datamatrix -f examples/01.png -o detection.png -g grid.png --quality
```

When showing quality metrics you can also generate a reflectance histogram, for use in a verification document. The option *--modulecentres* may also be used to only sample the module centres when creating the histogram.

``` bash
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --histogram reflectance.png
```

The *--shape* option also can be used to save an image showing variations in grid cell shape. This is only really useful for round or dot peen type markings.

``` bash
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --histogram reflectance.png --shape cell_shape.png
```

Metrics can also be output in *CSV*, *JSON* or *yaml* formats, for easy import into other systems.

``` bash
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --csv
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --json
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --yaml
```

Optionally, if you want debugging information and images for each decoding stage:

``` bash
datamatrix -f examples/01.png --debug
```

## Producing Verification Reports

A verification report in accordance with *GS1 2D Barcode Verification Process Implementation Guideline* may be produced with:

``` bash
datamatrix -f examples/01.png --report myreport.tex
```

These reports are in [LaTeX](https://en.wikipedia.org/wiki/LaTeX) format, and you can also set contact details, the report image filenames and a custom header logo as follows:

``` bash
datamatrix -f examples/01.png --report myreport.tex --address1 "Address line 1" --address2 "Address line 2" --address3 "Address line 3" --phone "123456789" --email "myemail@address" --web "https://mywebsite" --logo mylogo.png -o detection.png --hist histogram.png
```

<img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/report.png" width="50%" alt="Datamatrix verification report"/>

## Testing

Unit tests may be run with:

``` bash
datamatrix --tests
```

If you want to test how well the decoding works with a specific settings:

``` bash
datamatrix -f examples/01.png --debug \
    --erode [value] --dilate [value] \
    --meanlight [value]
```

*erode* and *dilate* values are in pixels. *meanlight* is typically in the range 0-20.

A set of images will be produced which show the detection process at each stage.

## References

[AIDC Healthcare Implementation Guideline](https://www.gs1.org/docs/healthcare/GS1_Healthcare_Implementation_Guideline.pdf)

[An Introduction to the Global Shipment Identification Number (GSIN)](https://edi.gs1si.org/cashedi/doc/gsin_intro.pdf)

[An Introduction to the Serial Shipping Container Code (SSCC)](https://documents.gs1us.org/adobe/assets/deliver/urn:aaid:aem:494e625b-e1d8-4bbd-a1be-5918879cfc3d/An-Introduction-to-the-Serial-Shipping-Container-Code-SSCC.pdf)

[Data Matrix](https://en.wikipedia.org/wiki/Data_Matrix)

[Edge linking](https://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MARSHALL/node30.html)

[European Medicines Verification System (EMVS): European Pack Coding Guidelines](https://www.medicinesforeurope.com/docs/European%20Pack%20Coding%20Guideline%20V4_0.pdf)

[GRAI Corrections & Clarifications](https://ref.gs1.org/standards/genspecs/gscn/2021/GSCN_21-118_GRAI.pdf)

[GS1 General Specifications](https://www.gs1.org/standards/barcodes-epcrfid-id-keys/gs1-general-specifications)

[GS1 2D Barcode Verification Process Implementation Guideline](https://www.gs1.org/docs/barcodes/2D_Barcode_Verification_Process_Implementation_Guideline.pdf)

[Line–line intersection](https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection)

[North American Coupon Application Guideline Using GS1 DataBar Expanded Symbols](https://www.gs1us.org/content/dam/gs1us/documents/industries-insights/standards/North-American-Coupon-Application-Guideline-Using-GS1-DataBar.pdf)

[Popular ISO/IEC 15434 Barcode Formats](https://www.barcodefaq.com/2d/data-matrix/iso-iec-15434)

[Reed–Solomon error correction](https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction)

[The Health Industry Supplier Labeling Standard for Patient Safety & Unique Device Identification (UDI)](https://www.hibcc.org/wp-content/uploads/SLS-2.6-Final.pdf)

[UNECE Standard Bovine Meat Carcases and Cuts](https://unece.org/sites/default/files/2024-03/Bovine_2007_e_0.pdf)

<img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/noai.gif" width="10%" alt="No AI Allowed"/> <img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/hand_coded.gif" width="10%" alt="Hand Coded"/> <img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/dontfeedai.gif" width="10%" alt="Don't feed the AI"/> <img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/writtenbyhuman.png" width="10%" alt="Written by a human not by an AI"/>
