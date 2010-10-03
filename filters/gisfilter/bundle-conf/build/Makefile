#
# $Id$
#

all:
	cd ../etc/build; make pack
	mv ../etc/build/gis-bundle-etc-templates.tgz .
	cd ../www/build; make pack
	mv ../www/build/gis-bundle-www-templates.tgz .

clean:
	cd ../etc/build; make clean
	cd ../www/build; make clean
	rm -f gis-bundle-etc-templates.tgz gis-bundle-www-templates.tgz
	rm -f *~
