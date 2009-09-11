#
# Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#

SUBDIRS =	filter satmap radmap

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

