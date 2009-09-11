SOURCES =	d.c e.c r.c nbspdb.c
HEADERS =	nbspdb.h
TARGET  =	e d r

CFLAGS +=	-c -Wall -I /usr/local/include/db42
LIBS =		-L /usr/local/lib -ldb-4.2

all: ${TARGET}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} ${.IMPSRC}

d: d.o nbspdb.o
	cc -o d d.o nbspdb.o ${LIBS}

e: e.o nbspdb.o
	cc -o e e.o nbspdb.o ${LIBS}

r: r.o nbspdb.o
	cc -o r r.o nbspdb.o ${LIBS}

clean:
	rm -rf *~ *.o ${TARGET}

