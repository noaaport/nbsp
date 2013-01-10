#
# $Id: Makefile,v cf4fd7927017 2009/09/11 16:26:32 nieves $
#

PKGMAKEFILE = Makefile.pkg
MAKEDEPS = ../../Makefile.inc ../../Makefile

default: build

../../Makefile: ../../Makefile.in
	cd ../..; ./configure.sh

../../Makefile.inc: ../../Makefile.inc.in
	cd ../..; ./configure.sh

build: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} build

install: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} install

package: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} package

clean: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} clean
