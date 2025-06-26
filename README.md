*datamatrix* is a commandline utility for reading square Datamatrix direct part markings.
Some image processing functions are based on *visionutils*.

## Key features

 * Reads PNG files (recommended at least 800 pixels across) using *lodepng*
 * Outputs the plain text encoded by the datamatrix pattern
 * Implemented in pure C99
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
./datamatrix -f myimage.png
```

Optionally, if you want debugging information and images for each decoding stage:

``` bash
./datamatrix -f myimage.png --debug
```
## References

Canny edge detection https://en.wikipedia.org/wiki/Canny_edge_detector

Data Matrix https://en.wikipedia.org/wiki/Data_Matrix

Edge linking https://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MARSHALL/node30.html

Line–line intersection https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection

Reed–Solomon error correction https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction
