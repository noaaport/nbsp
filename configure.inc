#
# $Id$
#

# defaults (FreeBSD)
INCLUDE=".include"
Q='"'
PREFIX=usr/local

TCL_INCDIR="-I /usr/local/include/tcl8.5"
TCL_LIB="tcl85"
TCLSH=/usr/local/bin/tclsh8.5

WISH=/usr/local/bin/wish8.5
EXPECT=/usr/local/bin/expect

DB_INCDIR="-I /usr/local/include/db46"
DB_LIB="db-4.6"
DB_BINDIR=/usr/local/bin/db46

LOCAL_LIBDIR="-L /usr/local/lib"
LOCAL_INCDIR="-I /usr/local/include"

CC=cc
CCWFLAGS="-Wall -Wextra"
INSTALL=install
SUFFIXRULES=".c.o:"

# excude options to tar
EXCLUDE="-W exclude"

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
        TCL_LIB="tcl84"
        TCL_INCDIR="-I /usr/pkg/include"
        TCLSH=/usr/pkg/bin/tclsh
	WISH=/usr/pkg/bin/wish
        DB_LIB="db4-4.4"
        DB_INCDIR="-I /usr/pkg/include/db4"
	DB_BINDIR=/usr/pkg/bin/db44
        LOCAL_LIBDIR="-L /usr/pkg/lib"
	LOCAL_INCDIR="-I /usr/pkg/include"
	CCWFLAGS="-Wall -Wno-char-subscripts -fno-strict-aliasing"
        ;;
    OpenBSD) 
        # use the defaults
	;;
    SunOS)
	INCLUDE=include
	Q=
	PREFIX=opt/local
	INSTALL=ginstall
	CC=gcc
	TCL_LIB="tcl8.3"
	;;
    Linux)
	INCLUDE=include
	Q=
	TCL_LIB="tcl8.5"
	TCL_INCDIR=
	TCLSH=/usr/bin/tclsh
	WISH=/usr/bin/wish
	EXPECT=/usr/bin/expect

	DB_LIB="db-4.6"
	DB_INCDIR=
	DB_BINDIR=

	LOCAL_INCDIR=
	LOCAL_LIBDIR=
	SUFFIXRULES="%.o: %.c"

	EXCLUDE="--exclude"

	# Override the defaults for flavor != fedoracore
    	if [ $flavor = opensuse ]
	then
	   DB_LIB="db-4.5"
	elif [ $flavor = centos ]
	then
	   TCL_LIB="tcl8.4"
	   DB_INCDIR="-I /usr/local/include/db46"
	   DB_LIB="db-4.6"
	elif [ $flavor = debian ]
	then
	   TCLSH=/usr/bin/tclsh8.5
	   TCL_INCDIR="-I /usr/include/tcl8.5"
    	fi
	;;
    Darwin)
	INCLUDE="include"
	Q=

	LOCAL_INCDIR="-I /usr/include -I /usr/local/include -I /sw/include"
	LOCAL_LIBDIR="-L/usr/lib -L/usr/local/lib -L/sw/lib"
	TCL_LIB="tcl8.4"
	TCL_INCDIR=
	DB_LIB="db-4.6"
	DB_INCDIR="-I /usr/local/include/db46"
	TCLSH=/usr/bin/tclsh

	WISH=/sw/bin/wish
	EXPECT=/sw/bin/expect

	SUFFIXRULES="%.o: %.c"
     	;;
esac

configure_default () {

  makefile_out=Makefile
  [ $# -eq 1 ] && makefile_out=$1
  makefile_in=${makefile_out}.in

  sed \
      -e "/@include@/ s||$INCLUDE|" \
      -e "/@q@/ s||$Q|g" \
      -e "/@PREFIX@/ s||$PREFIX|" \
      -e "/@INSTALL@/ s||$INSTALL|" \
      -e "/@TCL_INCDIR@/ s||$TCL_INCDIR|" \
      -e "/@TCL_LIB@/ s||$TCL_LIB|" \
      -e "/@TCLSH@/ s||$TCLSH|" \
      -e "/@WISH@/ s||$WISH|" \
      -e "/@EXPECT@/ s||$EXPECT|" \
      -e "/@DB_INCDIR@/ s||$DB_INCDIR|" \
      -e "/@DB_LIB@/ s||$DB_LIB|" \
      -e "/@DB_BINDIR@/ s||$DB_BINDIR|" \
      -e "/@LOCAL_INCDIR@/ s||$LOCAL_INCDIR|" \
      -e "/@LOCAL_LIBDIR@/ s||$LOCAL_LIBDIR|" \
      -e "/@CC@/ s||$CC|" \
      -e "/@CCWFLAGS@/ s||$CCWFLAGS|" \
      -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
      -e "/@EXCLUDE@/ s||$EXCLUDE|" \
      $makefile_in > $makefile_out
}
