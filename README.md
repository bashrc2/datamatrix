*datamatrix* is a commandline utility for reading square Datamatrix direct part markings.
Some image processing functions are based on *visionutils*.

## Key features

 * Reads PNG files (recommended at least 800 pixels across) using *lodepng*
 * Outputs the plain text encoded by the datamatrix pattern
 * Implemented in pure C99
 * Supports GS1 semantics https://www.gs1.org/standards/barcodes/2d
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

## Example usage

``` bash
./datamatrix -f examples/01.png
```

If needed, an image can be saved showing the detected datamatrix region.

``` bash
./datamatrix -f examples/01.png -o detection.png
```

Optionally, if you want debugging information and images for each decoding stage:

``` bash
./datamatrix -f examples/01.png --debug
```

## Testing with specific settings

It you want to test how well the decoding works with a specific settings:

``` bash
./datamatrix -f examples/01.png --debug \
    --erode [value] --dilate [value] \
    --meanlight [value] --edgethresh [value]
```

*erode* and *dilate* values are in pixels. *meanlight* is typically in the range 0-20 and *edgethresh* is typically in the range 0-10.

A set of images will be produced which show the detection process at each stage.

## References

Canny edge detection https://en.wikipedia.org/wiki/Canny_edge_detector

Data Matrix https://en.wikipedia.org/wiki/Data_Matrix

Edge linking https://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MARSHALL/node30.html

Line–line intersection https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

Reed–Solomon error correction https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction
