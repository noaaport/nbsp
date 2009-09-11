#
# $Id$
#
. ../../../configure.inc

# defaults (FreeBSD)
GPOSNAME=freebsd
NETPBMBINDIR=/usr/local/bin
INNBINDIR=/usr/local/news/bin
PATH="/bin:/sbin:/usr/bin:/usr/sbin";

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD)
	GPOSNAME=netbsd
	NETPBMBINDIR=/usr/pkg/bin
	INNBINDIR=/usr/pkg/news/bin
	;;
    OpenBSD) 
        GPOSNAME=openbsd
	;;
    SunOS)
	GPOSNAME=sunos
	;;
    Linux)       
	GPOSNAME=linux
	[ `uname -m` = "x86_64" ] && GPOSNAME=linux64
	NETPBMBINDIR=/usr/bin
	INNBINDIR=/usr/lib/news/bin
	;;
    Darwin)       
	GPOSNAME=darwmin
	NETPBMBINDIR=/sw/bin
	INNBINDIR=/sw/lib/news/bin
	PATH="/bin:/sbin:/usr/bin:/usr/sbin:/sw/bin:/sw/sbin";
	;;
esac
