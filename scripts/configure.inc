#
# $Id$
#
. ../configure.inc

NBSPDSH=nbspd.sh-freebsd
NBSPDSHCONF=
HOURLYCONF=hourly-cleanup.conf-freebsd
NBSPCLEANUP=nbspcleanup.tcl
STARTCLEANCONF=start-cleanup.conf-freebsd
STARTSTOPRC=startstop.rc-freebsd
NBSPD_RC_FPATH=/usr/local/etc/rc.d/nbspd

case $os in
    FreeBSD) 
    	;;
    NetBSD) 
	NBSPDSH=nbspd.sh-netbsd
    	;;
    OpenBSD) 
	NBSPDSH=nbspd.sh-generic
    	;;
    SunOS)
	NBSPDSH=nbspd.sh-generic
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
    	;;
    Linux)
	NBSPDSH=nbspd.sh-${flavor}
	HOURLYCONF=hourly-cleanup.conf-generic
	STARTCLEANCONF=start-cleanup.conf-generic
	NBSPD_RC_FPATH=/etc/init.d/nbspd

	[ $flavor = debian ] && STARTSTOPRC=startstop.rc-debian
	[ $flavor = debian ] && NBSPDSHCONF=nbspd.sh.conf-debian

	# Centos does not have tcl-8.5 and cannot the dicts version of
	# the nbspcleanup script
	if [ $flavor = centos ]
	then
	    NBSPCLEANUP=nbspcleanup.tcl-8.4
	    HOURLYCONF=hourly-cleanup.conf-generic-8.4
	fi
    	;;
    Darwin)
	NBSPDSH=nbspd.sh-macosx
	STARTSTOPRC=startstop.rc-macosx
	;;
esac
