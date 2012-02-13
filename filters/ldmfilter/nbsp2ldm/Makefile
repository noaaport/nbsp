#
# $Id$
#
# Usage:
#	make LDMHOME=/home/ldm
#	make LDMHOME=/home/ldm install
#
LDMINCLUDEDIR = ${LDMHOME}/include
LDMLIBDIR =	${LDMHOME}/lib

PKGBUILDDIR =
PREFIX =	usr/local
DESTDIR =	${PKGBUILDDIR}/${PREFIX}
BINDIR =	${DESTDIR}/bin

PROGNAME =	nbsp2ldm
TARGET =	${PROGNAME}
SOURCES =	${PROGNAME}.c
OBJECTS =	${PROGNAME}.o

LIBS =		-L ${LDMLIBDIR} -lldm -lm -lssl -lxml2
CFLAGS +=	-c -Wall -I ${LDMINCLUDEDIR}

all: ${TARGET}

.c.o:
	${CC} ${CFLAGS} $<

${TARGET}: ${OBJECTS}
	${CC} -o ${TARGET} ${OBJECTS} ${LIBS}

install: all
	install -m 755 ${TARGET} ${BINDIR}

clean:
	rm -f *~ ${TARGET} ${OBJECTS}
