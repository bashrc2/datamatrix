
all:
	gcc -Wall -std=c99 -pedantic -O3 -o datamatrix -Isrc src/*.c -lm

debug:
	gcc -Wall -std=c99 -pedantic -g -o datamatrix -Isrc src/*.c -lm

clean:
	rm -f src/*.plist datamatrix *.png
