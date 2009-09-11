#
# Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id

SUBDIRS =	tcl

all:
	for dir in ${SUBDIRS}; do \
	(cd $$dir; ${MAKE}) \
	done

install:
	for dir in ${SUBDIRS}; do \
	(cd $$dir; ${MAKE} install) \
	done

clean:
	rm -f *~
	for dir in ${SUBDIRS}; do \
	(cd $$dir; ${MAKE} clean) \
	done

