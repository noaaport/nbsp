SOURCES = main.c file.c fill.c util.c sbnpack.c
OBJECTS = main.o file.o fill.o util.o sbnpack.o
HEADERS = file.h sbnpack.h

TARGET = test

all: ${TARGET}

file.o: file.c ${HEADERS}
	cc -c file.c

fill.o: fill.c ${HEADERS}
	cc -c fill.c

util.o: util.c ${HEADERS}
	cc -c util.c

sbnpack.o: sbnpack.c ${HEADERS}
	cc -c sbnpack.c

main.o: main.c ${HEADERS}
	cc -c main.c

test: ${OBJECTS}
	cc -o test ${OBJECTS}

clean:
	rm -f *~ ${OBJECTS} ${TARGET}
