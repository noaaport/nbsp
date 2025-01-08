NAME = goesrcsv

TARGET = ${NAME}
SOURCES = main.c xy2lonlat.c
HEADERS = xy2lonlat.h
OBJECTS = main.o xy2lonlat.o

all: ${TARGET}

${TARGET}: ${OBJECTS}
	cc -o ${TARGET} ${OBJECTS} -L /usr/local/lib -lnetcdf -lm

main.o: main.c ${HEADERS}
	cc -c -I /usr/local/include main.c

xy2lonlat.o: xy2lonlat.c ${HEADERS}
	cc -c xy2lonlat.c

clean:
	rm -f *~
	rm -f ${OBJECTS}
	rm -f ${TARGET}
