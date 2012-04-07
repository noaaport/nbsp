#
# Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#
SOURCES =	wr.c wrm.c wrc.c wc1.c rc1.c
OBJECTS =	wr.o wrm.o wrc.o wc1.o rc1.o
HEADERS =	../spoolbdb.h ../spoolbdbv.h

TARGET =	wr wrm wrc wc1 rc1
CFLAGS =	-c -Wall -g -I /usr/local/include/db46 -I ..
LIBS =		-L /usr/local/lib/db46 -ldb -L .. -lspoolbdb

all: ${TARGET}

.c.o: ${HEADERS}
	${CC} ${CFLAGS} ${.IMPSRC}

wr: wr.o
	${CC} -g -o $@ wr.o ${LIBS}

wrm: wrm.o
	${CC} -g -o $@ wrm.o ${LIBS}

wrc: wrc.o
	${CC} -g -o $@ wrc.o ${LIBS}

wc1: wc1.o
	${CC} -g -o $@ wc1.o ${LIBS}

rc1: rc1.o
	${CC} -g -o $@ rc1.o ${LIBS}

clean:
	rm -f *~ ${OBJECTS} ${TARGET}
