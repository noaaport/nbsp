#
# Copyright (c) 2005-2023 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# See LICENSE
#
# $Id$
#

SUBDIRS = nbspinsert nbspmcast nbspcraftinsert

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
