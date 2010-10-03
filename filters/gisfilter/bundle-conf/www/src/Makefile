#
# $Id$
#

# These directories are relative to the www pub directory (www/htdocs/pub)
GISDIR = gis
DEFAULTSDIR = ${GISDIR}/defaults
RADDIR = ${GISDIR}/defaults/rad
SATDIR = ${GISDIR}/defaults/sat
SITEDIR = ${GISDIR}/site

all:
	cd ..; install -d ${RADDIR} ${SATDIR} ${SITEDIR}
	cp index.tml-top ../${GISDIR}/index.tml
	sed -e '/^#/d' index.tml-defaults > ../${DEFAULTSDIR}/index.tml
	sed -e '/^#/d' tig01_conus.tml.in > ../${SATDIR}/tig01_conus.tml
	./mkrad.sh ../${RADDIR}

clean:
	cd ..; rm -rf ${GISDIR}
	rm -f *~ *.tml
