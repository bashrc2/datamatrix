<img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/logo_square3.png" width="25%" alt="Picture of a datamatrix"/>

*datamatrix* is a commandline utility for reading and verifying Datamatrix direct part markings. Some image processing functions are based on [visionutils](https://codeberg.org/bashrc/visionutils).

## Key features

 * Reads PNG files (recommended at least 800 pixels across) using *lodepng*
 * Outputs the plain text encoded by the datamatrix pattern
 * Implemented in pure C99
 * Supports [GS1 semantics](https://www.gs1.org/standards/barcodes/2d) and digital links
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

``` bash
-f --filename [PNG file]      Filename of a PNG file containing the datamatrix image
--square                      Matrix is expected to be square
--rectangle                   Matrix is expected to be rectangular
--quality                     Show quality metrics for verification reports
-o --output [PNG file]        Save an image showing the detected datamatrix
-g --grid [PNG file]          Save an image showing the detected grid
--hist [PNG file]             Save reflectance histogram
--shape [PNG file]            Save cell shape variance image
-t --template [TEX file]      Path for verification report template
-r --report [TEX file]        Filename to save a verification report as
-l --logo [image file]        Logo image to be shown at top of verification report
--histcentres                 Only sample grid cell centres for reflectance histogram
--mingrid [8..144]            Minimum matrix dimension
--maxgrid [8..144]            Maximum matrix dimension
--debug                       Show debugging information
--csv                         Show quality metrics in CSV format
--json                        Show quality metrics in JSON format
--yaml                        Show quality metrics in yaml format
--freq [8..144]               Set the matrix dimension
--erode [0..20]               Set erosion itterations
--dilate [0..20]              Set dilation itterations
--meanlight [0..20]           Set mean light threshold
--resolver [url]              GS1 resolver (eg. https://id.gs1.org)
--sampleradius [pixels]       Radius for sampling each grid cell
--minsegmentlength [pixels]   Minimum edge segment length for segment joining
--resizewidth [pixels]        Width of image after resizing from original
--resizeheight [pixels]       Height of image after resizing from original
--binwidth [pixels]           Width of the binary image used for perimeter detection
--raw                         Show the raw decoded text
--aperture [ref]              Aperture reference number from ISO 15416
--light [nm]                  Peak light wavelength used in nanometres
--angle [90|45]               Angle of illumination in degrees
--address1 [text]             First line of address on verification report
--address2 [text]             Second line of address on verification report
--address3 [text]             Third line of address on verification report
--phone [number]              Telephone number on verification report
--email [address]             Email address on verification report
--web [url]                   Website url on verification report
--footer [text]               Footer shown on verification report
--tests                       Run unit tests
```

## Example usage

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

[Data Matrix](https://en.wikipedia.org/wiki/Data_Matrix)

[Edge linking](https://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MARSHALL/node30.html)

[GS1 General Specifications](https://www.gs1.org/standards/barcodes-epcrfid-id-keys/gs1-general-specifications)

[GS1 2D Barcode Verification Process Implementation Guideline](https://www.gs1.org/docs/barcodes/2D_Barcode_Verification_Process_Implementation_Guideline.pdf)

[Line–line intersection](https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection)

[Popular ISO/IEC 15434 Barcode Formats](https://www.barcodefaq.com/2d/data-matrix/iso-iec-15434)

[Reed–Solomon error correction](https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction)

[The Health Industry Supplier Labeling Standard for Patient Safety & Unique Device Identification (UDI)](https://www.hibcc.org/wp-content/uploads/SLS-2.6-Final.pdf)
