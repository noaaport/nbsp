#
# Copyright (c) 2005 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#

SUBDIRS = nbspfilter gpfilter rstfilter dafilter nntpfilter rstnntpfilter \
	inventory rssfilter capfilter filterlib ldmfilter metarfilter \
	uafilter wsfilter gisfilter panfilter gribfilter netfilter \
	spoolfilter msgfilter trackfilter craftfilter masterfilter \
	scheduler recover clusterfilter arcfilter

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
