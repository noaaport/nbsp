NAME = xy2lonlat

OBJECTS = ${NAME}.o
TARGET = ${NAME}
LIBS = -lm

all: ${TARGET}

xy2lonlat.o: xy2lonlat.c
	cc -c xy2lonlat.c

${TARGET}: xy2lonlat.o
	cc -o ${TARGET} xy2lonlat.o ${LIBS}

clean:
	rm -f ${OBJECTS}
	rm -f *~
	rm -f ${TARGET}
