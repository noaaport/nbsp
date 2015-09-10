#
# $Id$
#
. ../configure.inc

NBSPDDEFAULTS=nbspd.conf-freebsd
SYSCTLDEFAULTS=sysctl.conf-freebsd
SYSCTLCONFLOCAL=sysctl.conf.local
UDPRECVSIZEDEFAULTS=udprecvsize.conf-freebsd
NBSPCONFIGURE_UPDATEDB=nbspconfigure-updatedb.tcl-freebsd

case $os in
    *BSD) 
    	;;
    Linux)
	NBSPDDEFAULTS=nbspd.conf-linux
	SYSCTLDEFAULTS=sysctl.conf-linux
	SYSCTLCONFLOCAL=sysctl.d/local-noaaport.conf
	UDPRECVSIZEDEFAULTS=udprecvsize.conf-linux
	NBSPCONFIGURE_UPDATEDB=nbspconfigure-updatedb.tcl-linux
    	;;
    Darwin)
	NBSPDDEFAULTS=nbspd.conf-macosx
	;;
esac
