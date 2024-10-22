SOURCES = main.c
OBJECTS = main.o
HEADERS = ../sbnpack/sbnpack.h

LIBS = -L ../sbnpack -lsbnpack
INCLUDES = -I ../sbnpack

TARGET = test

CFLAGS += -c -Wall

all: ${TARGET}

lib:
	cd ../sbnpack; make

main.o: main.c ${HEADERS}
	${CC} ${CFLAGS} ${INCLUDES} main.c

test: ${OBJECTS} lib
	${CC} -o ${TARGET} ${OBJECTS} ${LIBS}

clean:
	rm -f *~
	rm -f ${OBJECTS} ${TARGET}
	rm -f output output0 output1
	cd ../sbnpack; ${MAKE} clean
