SOURCES = main.c
OBJECTS = main.o
HEADERS = ../sbnpack.h

LIBS = -L .. -lsbnpack
INCLUDES = -I ..

TARGET = test

CFLAGS += -c -Wall

all: ${TARGET}

main.o: main.c ${HEADERS}
	${CC} ${CFLAGS} ${INCLUDES} main.c

test: ${OBJECTS}
	${CC} -o ${TARGET} ${OBJECTS} ${LIBS}

clean:
	rm -f *~
	rm -f ${OBJECTS} ${TARGET}
	rm -f output output0 output1 outputlast
