SOURCES =	gempak.in
M4SOURCES =	gp.defs.m4 gp.m4 
M4OBJECTS =	gp.pl
SEDSOURCES =	gp.sed
ALLSOURCES =	${SOURCES} ${M4SOURCES} ${SEDSOURCES}
ALLOBJECTS =	${M4OBJECTS}

TARGET =	gempak-pipe.pl

all: ${TARGET}

gempak-pipe.pl: ${SOURCES} ${SEDSOURCES} ${M4OBJECTS}
	sed -f gp.sed gempak.in > ${.TARGET}

gp.pl: ${M4SOURCES}
	m4 gp.m4 > ${.TARGET}

install: ${TARGET}
	cp ${TARGET} ..

clean:
	rm -f *~ ${TARGET} ${ALLOBJECTS}
