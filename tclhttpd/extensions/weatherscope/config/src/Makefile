#
# $Id$
#

PACK_CLIENT_NAME = ws-client
PACK_SERVER_NAME = ws-server

DEFS = PACK_CLIENT_NAME=${PACK_CLIENT_NAME} \
	PACK_SERVER_NAME=${PACK_SERVER_NAME}

all: client server

client:
	cd m4 && ${MAKE} ${DEFS} client

server:
	cd m4 && ${MAKE} ${DEFS} server

clean:
	rm -f *~
	cd m4 && ${MAKE} ${DEFS} clean

distclean: clean
	rm -f ${PACK_CLIENT_NAME}* ${PACK_SERVER_NAME}*
	cd m4 && ${MAKE} ${DEFS} distclean
