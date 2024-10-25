#
# Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
#

SOURCES = mcast.c
OBJECTS = mcast.o 
HEADERS = mcast.h

TARGET = libmcast

CFLAGS += -c -Wall

all: ${TARGET}.a

mcast.o: mcast.c ${HEADERS}
	${CC} ${CFLAGS} mcast.c

${TARGET}.a: ${OBJECTS}
	${AR} -rc $@ ${OBJECTS}

clean:
	rm -f *~
	rm -f ${OBJECTS} ${TARGET}.a
