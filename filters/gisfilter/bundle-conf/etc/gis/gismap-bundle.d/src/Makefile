#
# $Id$
#

RADENABLE = rad_conus.conf rad_ak.conf rad_gu.conf rad_hi.conf rad_pr.conf \
	rad_east.conf rad_south.conf rad_central.conf rad_west.conf \
	rad_sites.conf
SATENABLE = sat.conf
EXAMPLES = rad_n0r_tx.conf-ex sat_conus.conf-ex

ENABLE = ${RADENABLE} ${SATENABLE}

all: clean
	cd ..; mkdir dist enable
	cp sat.conf ../dist
	cp rad_sites.conf ../dist
	cp ${EXAMPLES} ../dist
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
