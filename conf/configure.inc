#
# $Id: configure.inc 957 2011-05-17 19:38:12Z jfnieves $
#
. ../configure.inc

NBSPDDEFAULTS=nbspd.conf-freebsd
SYSCTLDEFAULTS=sysctl.conf-freebsd

case $os in
    *BSD) 
    	;;
    Linux)
	NBSPDDEFAULTS=nbspd.conf-linux
	[ $flavor = centos ] && SYSCTLDEFAULTS=sysctl.conf-linux
	[ $flavor = debian ] && SYSCTLDEFAULTS=sysctl.conf-debian
    	;;
    Darwin)
	NBSPDDEFAULTS=nbspd.conf-macosx
	;;
esac
