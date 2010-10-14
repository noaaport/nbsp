#
# $Id$
#

RADENABLE = rad_conus.conf rad_ak.conf rad_pr.conf rad_hi.conf \
	rad_east.conf rad_south.conf rad_central.conf rad_west.conf
SATENABLE =
#SATENABLE = sat_conus.conf

ENABLE = ${RADENABLE} ${SATENABLE}

all: clean
	cd ..; mkdir dist enable
	cp sat_conus.conf ../dist
	for file in *.in; \
	do \
	name=$${file%.in}; \
	cat $${file} rad_body.conf > ../dist/rad_$${name}.conf; \
	done
	cd ../dist; cp ${ENABLE} ../enable
	## ./mkres.sh

clean:
	cd ..; rm -rf *~ dist enable
	rm -f *~
