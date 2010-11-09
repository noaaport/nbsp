#
# $Id$
#

#
# What will be copied to "enable" but avoiding duplicates (rad_conus
# includes all conus sites, and "us" includes conus plus ak, hi and pr).
#
#RADENABLE = rad_conus.conf rad_ak.conf rad_gu.conf rad_hi.conf rad_pr.conf
RADENABLE = rad_us.conf
#SATENABLE = sat_all.conf
SATENABLE =

ENABLE = ${RADENABLE} ${SATENABLE}

all: clean
	cd ..; mkdir dist enable
	cp sat_all.conf ../dist
	for file in *.in;\
	do \
	name=$${file%.in};\
	cat $${file} rad_body.conf > ../dist/rad_$${name}.conf; \
	done
	#
	# Copy to enable but avoid duplicates
	#
	cd ../dist; cp ${ENABLE} ../enable

clean:
	cd ..; rm -rf *~ dist enable
	rm -f *~
