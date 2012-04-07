#
# $Id$
#

# defaults (FreeBSD)
INCLUDE=".include"
Q='"'
PREFIX=usr/local

INSTALL=install
TCLSH=/usr/local/bin/tclsh8.6

os=`uname`
[ $os = Linux ] && flavor=unknown
if [ -f /etc/fedora-release ]
then
    flavor=fedoracore
elif [ -f /etc/SuSE-release ] 
then
   flavor=opensuse
elif [ -f /etc/redhat-release ] 
then
   flavor=centos
elif [ -f /etc/debian_version ] 
then
   flavor=debian
fi

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD)
	PREFIX=usr/pkg
        TCLSH=/usr/pkg/bin/tclsh
        ;;
    OpenBSD) 
        # use the defaults
	;;
    SunOS)
	INCLUDE=include
	Q=
	INSTALL=ginstall
	;;
    Linux)
	INCLUDE=include
	Q=
	PREFIX=usr
	TCLSH=/usr/bin/tclsh
	if [ $flavor = debian ]
	then
	   TCLSH=/usr/bin/tclsh8.5
	fi
	;;
esac

configure_default () {

  makefile_out=Makefile
  [ $# -eq 1 ] && makefile_out=$1
  makefile_in=${makefile_out}.in

  sed \
      -e "/@include@/ s||$INCLUDE|" \
      -e "/@q@/ s||$Q|g" \
      -e "/@PREFIX@/ s||$PREFIX|g" \
      -e "/@INSTALL@/ s||$INSTALL|" \
      -e "/@TCLSH@/ s||$TCLSH|" \
      $makefile_in > $makefile_out
}
