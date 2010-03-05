#
# $Id$
#
. ../../../configure.inc

# defaults (FreeBSD)
INCDIRS="-iquote . -iquote ../../../src $LOCAL_INCDIR"
LIBDIRS=$LOCAL_LIBDIR
LIBS="-lpng -lz"

case $os in
    *BSD) 
        # use the defaults
	;;
    SunOS)
	INCDIRS="$INCDIRS -I /opt/sfw/include -I /usr/sfw/include"
	LIBDIRS="-L /opt/sfw/lib -L /usr/sfw/lib"
	;;
    Linux)       
	LIBDIRS=
	;;
    Darwin)
	# use the defaults
	;;
esac
