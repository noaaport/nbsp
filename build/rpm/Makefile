#
# $Id$
#

MAINDIR = ../..
PKGMAKEFILE = Makefile.pkg
MAKEDEPS = ${MAINDIR}/Makefile.inc ${MAINDIR}/Makefile

default: build

${MAINDIR}/Makefile: ${MAINDIR}/Makefile.in
	cd ${MAINDIR}; ./configure.sh

${MAINDIR}/Makefile.inc: ${MAINDIR}/Makefile.inc.in
	cd ${MAINDIR}; ./configure.sh

build: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} build

install: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} install

package: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} package

clean: ${MAKEDEPS}
	${MAKE} -f ${PKGMAKEFILE} clean
