TARGET =	recv send rate faz
SOURCES =	recv.c send.c rate.c faz.c mcast.c
OBJECTS =	mcast.o
HEADERS =	mcast.h

CFLAGS +=	-c -Wall -g

all: ${TARGET}

faz: faz.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

recv: recv.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

send: send.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

rate: rate.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} ${.IMPSRC}

clean:
	rm -f *~ *.o ${TARGET}

cleanall: clean
	rm -f Makefile mcast.c mcast.h
