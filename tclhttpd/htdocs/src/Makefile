BUILDDIR =	pub

CSSDIR	=	css
IMGDIR =	images
SUBDIRS =	${CSSDIR} ${IMGDIR}

all:
	./mk-tml.sh ${BUILDDIR}
	cp -r ${SUBDIRS} ${BUILDDIR}

clean:
	rm -f *~
	rm -rf ${BUILDDIR}
