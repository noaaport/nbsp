#
# Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
#

SOURCES = file.c fill.c util.c sbnpack.c sbn.c
OBJECTS = file.o fill.o util.o sbnpack.o sbn.o
HEADERS = file.h sbnpack.h sbnpack_private.h sbn.h
AUXFILES = util.c util.h

TARGET = libsbnpack

CFLAGS += -c -Wall

all: ${TARGET}.a

file.o: file.c ${HEADERS}
	${CC} ${CFLAGS} file.c

fill.o: fill.c ${HEADERS}
	${CC} ${CFLAGS} fill.c

util.o: util.c ${HEADERS}
	${CC} ${CFLAGS} util.c

sbnpack.o: sbnpack.c ${HEADERS}
	${CC} ${CFLAGS} sbnpack.c

${TARGET}.a: ${OBJECTS}
	${AR} -rc $@ ${OBJECTS}

clean:
	rm -f *~
	rm -f ${OBJECTS} ${TARGET}.a
	rm -f ${AUXFILES}
