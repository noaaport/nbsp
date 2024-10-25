TARGET =	recv send rate
SOURCES =	recv.c send.c rate.c mcast.c
OBJECTS =	mcast.o
HEADERS =	mcast.h

CFLAGS +=	-c -Wall

all: ${TARGET}

recv: recv.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

send: send.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

rate: rate.o ${OBJECTS}
	cc -o ${.TARGET} ${.ALLSRC}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} ${.IMPSRC}

clean:
	rm -f *~ *.o
