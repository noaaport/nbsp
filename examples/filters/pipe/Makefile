SOURCES =	pipe.in
M4SOURCES =	pipe.defs.m4 pipe.m4 
M4OBJECTS =	pipe.m4.out
SEDSOURCES =	pipe.sed
ALLSOURCES =	${SOURCES} ${M4SOURCES} ${SEDSOURCES}
ALLOBJECTS =	${M4OBJECTS}

TARGET =	pipe.pl

all: ${TARGET}

pipe.pl: ${SOURCES} ${SEDSOURCES} ${M4OBJECTS}
	sed -f pipe.sed pipe.in > ${.TARGET}

pipe.m4.out: ${M4SOURCES}
	m4 pipe.m4 > ${.TARGET}

clean:
	rm -f *~ ${TARGET} ${ALLOBJECTS}
