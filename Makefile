APP=datamatrix
PREFIX?=/usr/local

all:
	gcc -Wall -std=c99 -pedantic -O3 -o ${APP} -Isrc src/*.c -lm -fopenmp

debug:
	gcc -Wall -std=c99 -pedantic -g -o ${APP} -Isrc src/*.c -lm -fopenmp

clean:
	rm -f src/*.plist ${APP} *.png

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp ${APP} ${DESTDIR}${PREFIX}/bin
	mkdir -m 755 -p ${DESTDIR}${PREFIX}/share/man/man1
	cp man/${APP}.1.gz ${DESTDIR}${PREFIX}/share/man/man1

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/${APP}
	rm ${DESTDIR}${PREFIX}/share/man/man1/man/${APP}.1.gz
