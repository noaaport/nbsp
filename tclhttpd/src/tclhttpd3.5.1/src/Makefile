# This is based on the Makefile generated in the Tcl dltest directory

CC = gcc

# Linux - probably needs -lcrypt
CRYPT_LIB =
#CRYPT_LIB = -lcrypt
LIBS =			$(CRYPT_LIB) -ltcl8.0 -ldl  -lm -lc

SHLIB_CFLAGS =		-fPIC
SHLIB_LD =		ld -G -z text
SHLIB_SUFFIX =		.so
SHLIB_VERSION =		
SRC_DIR =		.
TCL_VERSION=		8.0

CFLAGS = -g
#MEM_DEBUG = -DTCL_MEM_DEBUG
CC_SWITCHES = $(CFLAGS) $(MEM_DEBUG) ${SHLIB_CFLAGS}

all: crypt${SHLIB_SUFFIX} limit${SHLIB_SUFFIX} setuid${SHLIB_SUFFIX} utime${SHLIB_SUFFIX}

install: ${all}
	cp crypt${SHLIB_SUFFIX} limit${SHLIB_SUFFIX} setuid${SHLIB_SUFFIX} ../lib

crypt${SHLIB_SUFFIX}: $(SRC_DIR)/crypt.c
	$(CC) -c $(CC_SWITCHES) $(SRC_DIR)/crypt.c
	${SHLIB_LD} crypt.o -o crypt${SHLIB_SUFFIX} 

limit${SHLIB_SUFFIX}: $(SRC_DIR)/limit.c
	$(CC) -c $(CC_SWITCHES) $(SRC_DIR)/limit.c
	${SHLIB_LD} limit.o -o limit${SHLIB_SUFFIX} 

setuid${SHLIB_SUFFIX}: $(SRC_DIR)/setuid.c
	$(CC) -c $(CC_SWITCHES) $(SRC_DIR)/setuid.c
	${SHLIB_LD} setuid.o -o setuid${SHLIB_SUFFIX} 

utime${SHLIB_SUFFIX}: $(SRC_DIR)/utime.c
	$(CC) -c $(CC_SWITCHES) $(SRC_DIR)/utime.c
	${SHLIB_LD} utime.o -o utime${SHLIB_SUFFIX} 

clean:
	rm -f *.o *${SHLIB_SUFFIX} config.cache config.log config.status lib.exp
