#
# $Id$
#
DIRS = exporter weatherscope npstats catalog inbsp

all:
	for dir in ${DIRS}; do \
	(cd $$dir; ${MAKE}) \
	done

install:
	for dir in ${DIRS}; do \
	(cd $$dir; ${MAKE} install) \
	done

clean:
	for dir in ${DIRS}; do \
	(cd $$dir; ${MAKE} clean) \
	done
	rm -f *~
