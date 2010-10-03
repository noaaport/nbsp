#
# $Id$
#

PLIST =	pkg-plist
DISTFILE = gis-bundle-etc-templates.tgz

all:
	cd ..; make all

pack: all
	cd ..; tar -czf build/${DISTFILE} -T build/${PLIST}

clean:
	cd ..; make clean
	rm -f *~ ${DISTFILE}
