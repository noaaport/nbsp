#
# $Id$
#

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
	cd ../dist; \
	cp sat_all.conf rad_conus.conf rad_ak.conf rad_hi.conf rad_pr.conf \
	../enable

clean:
	cd ..; rm -rf *~ dist enable
	rm -f *~
