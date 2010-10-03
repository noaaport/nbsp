#
# $Id$
#

# These directories are relative to the www pub directory (www/htdocs/pub)
GISDIR = gis
DEFAULTSDIR = ${GISDIR}/defaults
RADDIR = ${GISDIR}/defaults/rad
SATDIR = ${GISDIR}/defaults/sat
SITEDIR = ${GISDIR}/site

all: in
	cd ..; install -d ${RADDIR} ${SATDIR} ${SITEDIR}
	cp index.tml-top ../${GISDIR}/index.tml
	sed -e '/^#/d' index.tml-defaults > ../${DEFAULTSDIR}/index.tml
	sed -e '/^#/d' tig01_conus.tml > ../${SATDIR}/tig01_conus.tml
	for f in *.in; \
	do \
	name=$${f%.in}; cat $${f} rad_body.tml | \
		sed -e '/^#/d' > ../${RADDIR}/$${name}.tml; \
	done

in: mkradin.tcl
	./mkradin.tcl

clean:
	cd ..; rm -rf ${GISDIR}
	rm -f *~ *.in
