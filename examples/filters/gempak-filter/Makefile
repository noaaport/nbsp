SOURCES =	gempak.in inngempak.in
M4SOURCES =	gp.defs.m4 gp.m4 gp_pipe.m4 gp_file.m4 
M4OBJECTS =	gp.pl
SEDSOURCES =	gp.sed
ALLSOURCES =	${SOURCES} ${M4SOURCES} ${SEDSOURCES}
ALLOBJECTS =	${M4OBJECTS}

TARGET =	gempakfilter.pl inngempakfilter.pl

all: ${TARGET}

gempakfilter.pl: gempak.in ${SEDSOURCES} ${M4OBJECTS}
	sed -f gp.sed gempak.in > ${.TARGET}

inngempakfilter.pl: inngempak.in ${SEDSOURCES} ${M4OBJECTS}
	sed -f gp.sed inngempak.in > ${.TARGET}

gp.pl: ${M4SOURCES}
	m4 gp.m4 > ${.TARGET}

install: ${TARGET}
	cp ${TARGET} ..

clean:
	rm -f *~ ${TARGET} ${ALLOBJECTS}
