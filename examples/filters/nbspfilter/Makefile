SOURCES =	nbspfilter.tcl
M4SOURCES =	nbspfilter.defs.m4 nbspfilter.rc.m4 
ALLSOURCES =	${SOURCES} ${M4SOURCES} ${SEDSOURCES}

TARGET =	nbspfilter.rc

all: ${TARGET}

${TARGET}: ${M4SOURCES}
	m4 nbspfilter.defs.m4 nbspfilter.rc.m4 > ${TARGET}

clean:
	rm -f *~ ${TARGET}
