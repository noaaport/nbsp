#
# $Id$
#

all: clean
	cd ..; mkdir dist enable
	cp sat_conus.conf ../dist
	for file in *.in; \
	do \
	name=$${file%.in}; \
	cat $${file} rad_body.conf > ../dist/rad_$${name}.conf; \
	done
	cd ../dist; cp *.conf ../enable
	## ./mkres.sh

clean:
	cd ..; rm -rf *~ dist enable
	rm -f *~
