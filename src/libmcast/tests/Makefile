TARGET =	recv send rate
SOURCES =	recv.c send.c rate.c mcast.c
OBJECTS =	mcast.o
HEADERS =	mcast.h

CFLAGS +=	-c -Wall -g

all: ${TARGET}

recv: recv.o ${OBJECTS}
	${CC} -o $@ recv.o ${OBJECTS}

send: send.o ${OBJECTS}
	${CC} -o $@ send.o ${OBJECTS}

rate: rate.o ${OBJECTS}
	${CC} -o $@ rate.o ${OBJECTS}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} $<

mcast.c:
	cp ../src/mcast.c .

mcast.h:
	cp ../src/mcast.h .


clean:
	rm -f *~ *.o ${TARGET}
	rm -f mcast.c mcast.h



