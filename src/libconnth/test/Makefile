HEADERS = common.h dbenv.h
LIBS	= -lconn -lqdb -ldb -lpthread
LIBDIR	= -L .. -L ../../libqdb -L /usr/local/lib/db47
LDOPTS	= ${LIBDIR} ${LIBS}
#CFLAGS	+= -Wall -c -I /usr/local/include/db47
CFLAGS	+= -Wall -c -I /usr/local/include/db47

all: server nclient

.c.o: ${HEADERS}
	${CC} ${CFLAGS} ${.IMPSRC}

nclient: nclient.o 
	${CC} -o nclient nclient.o ${LDOPTS}

server: serverm.o serverc.o dbenv.o
	${CC} -o server serverm.o serverc.o dbenv.o ${LDOPTS}

clean:
	rm -f *~ *.o server client nclient __db* *.core
