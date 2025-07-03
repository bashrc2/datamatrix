<img src="https://gitlab.com/bashrc2/datamatrix/-/raw/main/img/datamatrix.png" width="25%" alt="Picture of a datamatrix"/>

*datamatrix* is a commandline utility for reading and verifying Datamatrix direct part markings. Some image processing functions are based on [visionutils](https://codeberg.org/bashrc/visionutils).

## Key features

 * Reads PNG files (recommended at least 800 pixels across) using *lodepng*
 * Outputs the plain text encoded by the datamatrix pattern
 * Implemented in pure C99
 * Supports [GS1 semantics](https://www.gs1.org/standards/barcodes/2d) and digital links
 * Can generate quality metrics for verification reports
 * Parallelized with [OpenMP](https://www.openmp.org)
 * No dependencies or supply chains!

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

## Example usage

``` bash
datamatrix -f examples/01.png
```

When possible decoded text will appear in a more verbose human readable form. However if you want to know what text the datamatrix directly encodes then you can use the *--raw* option.

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

Metrics can also be output in *CSV* or *JSON* formats, for easy import into other systems.

``` bash
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --csv
datamatrix -f examples/01.png -o detection.png -g grid.png --quality --json
```

Optionally, if you want debugging information and images for each decoding stage:

``` bash
datamatrix -f examples/01.png --debug
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
    --meanlight [value] --edgethresh [value]
```

*erode* and *dilate* values are in pixels. *meanlight* is typically in the range 0-20 and *edgethresh* is typically in the range 0-10.

A set of images will be produced which show the detection process at each stage.

## References

[Canny edge detection](https://en.wikipedia.org/wiki/Canny_edge_detector)

[Data Matrix](https://en.wikipedia.org/wiki/Data_Matrix)

[Edge linking](https://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MARSHALL/node30.html)

[GS1 General Specifications](https://www.gs1.org/standards/barcodes-epcrfid-id-keys/gs1-general-specifications)

[GS1 2D Barcode Verification Process Implementation Guideline](https://www.gs1.org/docs/barcodes/2D_Barcode_Verification_Process_Implementation_Guideline.pdf)

[Line–line intersection](https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection)

[Popular ISO/IEC 15434 Barcode Formats](https://www.barcodefaq.com/2d/data-matrix/iso-iec-15434)

[Reed–Solomon error correction](https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction)
