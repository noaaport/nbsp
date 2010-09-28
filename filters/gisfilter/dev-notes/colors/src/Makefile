#
# $Id$
#

TARGET =	colors.html
SOURCES =	colors.html.in
OBJECTS =	body.html
AUX =		cvt.output

BINTOOLS =	cvt
SCRIPTS =	mkhtml.awk mkhtml.sed
TOOLS =		${BINTOOLS} ${SCRIPTS}

all: ${TARGET}

cvt: cvt.c
	cc -o cvt cvt.c

${TARGET}: ${SOURCES} ${OBJECTS}
	sed -f mkhtml.sed ${SOURCES} > ${TARGET}

${OBJECTS}: ${AUX}
	awk -f mkhtml.awk cvt.output > body.html

cvt.output: cvt
	./cvt > cvt.output

clean:
	rm -f *~ ${OBJECTS} ${AUX}

cleanall: clean
	rm -f ${TARGET} ${BINTOOLS}
