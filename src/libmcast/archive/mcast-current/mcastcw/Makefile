TARGET =	recv
SOURCES =	recv.c
OBJECTS =	recv.o mcast.o 
HEADERS =	mcast.h

CFLAGS +=	-c -Wall

all: ${TARGET}

recv: ${OBJECTS}
	cc -o $@ ${OBJECTS}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} $<

clean:
	rm -f *~ ${OBJECTS} *.exe

