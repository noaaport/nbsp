#
# $Id: Makefile,v 3de6367c6e6d 2013/11/12 22:45:56 nieves $
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
OBJECTS =	${PROGNAME}.o strsplit.o err.o

#LIBS =		-L ${LDMLIBDIR} -Xlinker -R ${LDMLIBDIR} \
#			-lldm -lm -lcrypto -lxml2
LIBS =		${LDMLIBDIR}/libldm.a -lpthread -lm -lcrypto -lxml2
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
