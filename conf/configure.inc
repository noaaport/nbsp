#
# $Id$
#
. ../configure.inc

NBSPDDEFAULTS=nbspd.conf-freebsd

case $os in
    *BSD) 
    	;;
    SunOS)
	NBSPDDEFAULTS=nbspd.conf-generic
    	;;
    Linux)
	NBSPDDEFAULTS=nbspd.conf-generic
    	;;
    Darwin)
	NBSPDDEFAULTS=nbspd.conf-macosx
	;;
esac
