SOURCES =	qdb.c d.c e.c
HEADERS =	qdb.h
TARGET  =	d e

CFLAGS +=	-g -c -Wall -I /usr/local/include/db42
LIBS =		-L /usr/local/lib -ldb-4.2

all: ${TARGET}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} ${.IMPSRC}

d: d.o qdb.o
	cc -o d d.o qdb.o ${LIBS}

e: e.o qdb.o
	cc -o e e.o qdb.o ${LIBS}

r: r.o qdb.o
	cc -o r r.o qdb.o ${LIBS}

clean:
	rm -rf *~ *.o ${TARGET}

