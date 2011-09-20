#
# $Id$
#
. ../configure.inc

RCINIT=nbspd.sh-freebsd
RCFPATH=/usr/local/etc/rc.d/nbspd
RCCONF=
HOURLYCONF=hourly-cleanup.conf-freebsd
CLEANUP=nbspcleanup.tcl
STARTCLEANCONF=start-cleanup.conf-freebsd
STARTSTOPRC=startstop.rc-freebsd
POSTINSTALL_SRC=../build/bsd/pkg-postinstall
POSTINSTALL_TARGET=nbsppostinstall

case $os in
    FreeBSD) 
    	;;
    NetBSD) 
	RCINIT=nbspd.sh-netbsd
    	;;
    OpenBSD) 
	RCINIT=nbspd.sh-generic
    	;;
    SunOS)
	RCINIT=nbspd.sh-generic
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
    	;;
    Linux)
	RCINIT=nbspd.sh-${flavor}
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
	RCFPATH=/etc/init.d/nbspd

	[ $flavor = debian ] && STARTSTOPRC=startstop.rc-debian
	[ $flavor = debian ] && RCCONF=nbspd.sh.conf-debian
	[ $flavor = debian ] && POSTINSTALL_SRC=../build/debian/postinst

	[ $flavor = centos ] && POSTINSTALL_SRC=../build/rpm/rpm-postinstall
    	;;
    Darwin)
	RCINIT=nbspd.sh-macosx
	STARTSTOPRC=startstop.rc-macosx
	;;
esac
