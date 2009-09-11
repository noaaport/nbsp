TARGET =	interp
SOURCES =	interp.c
OBJECTS =	interp.o

INCDIRS =	-I /usr/local/include/tcl8.4
LIBDIRS =	-L \/usr\/local\/lib

TCL_LIB =	tcl84
CFLAGS =	-c -Wall -g ${INCDIRS}
LIBS =		${LIBDIRS} -l${TCL_LIB}

.c.o:
	${CC} ${CFLAGS} ${.IMPSRC}

${TARGET}: ${OBJECTS}
	${CC} -o ${TARGET} ${OBJECTS} ${LIBS}

clean:
	rm -f *~ ${TARGET} ${OBJECTS}
