#
# $Id$
#
. ../configure.inc

NBSPDDEFAULTS=nbspd.conf-freebsd
SYSCTLDEFAULTS=sysctl.conf-freebsd
SYSCTLCONFLOCAL=sysctl.conf.local
UDPRECVSIZEDEFAULTS=udprecvsize.conf-freebsd

case $os in
    *BSD) 
    	;;
    Linux)
	NBSPDDEFAULTS=nbspd.conf-linux
	UDPRECVSIZEDEFAULTS=udprecvsize.conf-linux
	SYSCTLCONFLOCAL=sysctl.conf
	[ $flavor = centos ] && SYSCTLDEFAULTS=sysctl.conf-linux
	[ $flavor = centos ] && SYSCTLCONFLOCAL=sysctl.conf
	[ $flavor = debian ] && SYSCTLDEFAULTS=sysctl.conf-debian
	[ $flavor = debian ] && SYSCTLCONFLOCAL=sysctl.d/local-noaaport.conf
    	;;
    Darwin)
	NBSPDDEFAULTS=nbspd.conf-macosx
	;;
esac
