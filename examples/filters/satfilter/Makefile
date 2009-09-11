TARGET = nbspsat
SOURCES = nbspsat.c err.c
OBJECTS = nbspsat.o err.o
HEADERS = err.h

CFLAGS += -c -Wall -I . -I ../../../src -I- -I /usr/local/include 
LIBS += -L /usr/local/lib -lpng -lz

.c.o: ${HEADERS}
	${CC} ${CFLAGS} $<

${TARGET}: ${OBJECTS}
	${CC} -o ${TARGET} ${OBJECTS} ${LIBS}

clean:
	rm -r -f *~ ${TARGET} ${OBJECTS}
