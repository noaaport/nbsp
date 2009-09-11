#
# $Id$
#
. ../../../configure.inc

SCHEDCONF=scheduler.conf-freebsd

case $os in
    FreeBSD) 
        # use the defaults
	;;
    OpenBSD) 
        # use the defaults
	;;
    SunOS)
	SCHEDCONF=scheduler.conf-generic
	;;
    Linux)       
	SCHEDCONF=scheduler.conf-generic
	;;
    Darwin)
	# use the defaults
	;;
esac
