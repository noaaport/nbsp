#
# $Id$
#
include ../../VERSION

# The defaults in spec.in are for SL, and the overrides are other
# flavors are below.
#
rpmrelease = $(shell grep -v "^\#" rpm-release)
rpmroot = ${HOME}/rpmbuild
requires = $(shell grep -v "^\#" rpm-requires)

flavor = $(shell ./flavor.sh)
## arch =  $(shell uname -m)
arch = noarch
dist = $(shell ./dist.sh)

ifeq (${flavor}, opensuse)
  ifeq (${arch}, i386)
    arch = i586
  endif
endif

tgzfile = ${name}-${version}.tgz
rpmfile = ${name}-${version}-${rpmrelease}${dist}.${arch}.rpm

default: package

spec: rpm-spec
tgz: ${tgzfile}

${tgzfile}:
	cd ../../..;\
	cp -r ${name} ${name}-${version};\
	tar -czf ${name}-${version}.tgz ${name}-${version};\
	rm -rf ${name}-${version}
	mv ../../../${name}-${version}.tgz .

rpm-spec: rpm-spec.in
	sed -e "/@name@/s//${name}/" \
	-e "/@version@/s//${version}/" \
	-e "/@rpmrelease@/s//${rpmrelease}/" \
	-e "/@postinstall@/r rpm-postinstall" \
	-e "/@requires@/s//${requires}/" \
	-e "/@postinstall@/d" \
	-e "/@postuninstall@/r rpm-postuninstall" \
	-e "/@postuninstall@/d" \
	-e "/@plist@/r rpm-plist" \
	-e "/@plist@/d" \
	rpm-spec.in > rpm-spec

package: rpm-spec ${tgzfile}
	mv ${tgzfile} ${rpmroot}/SOURCES
	rpmbuild -bb rpm-spec
	cp ${rpmroot}/RPMS/${arch}/${rpmfile} .

clean:
	rm -f *~ rpm-spec ${tgzfile} ${rpmfile} ${rpmfile_client}
