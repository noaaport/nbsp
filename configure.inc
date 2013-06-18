#
# $Id$
#

# defaults (FreeBSD)
INCLUDE=".include"
Q='"'
PREFIX=usr/local

CC=cc
CCWFLAGS="-Wall -Wextra -Wformat-security -Werror"
INSTALL=install
SUFFIXRULES=".c.o:"

# exclude options to tar
EXCLUDE="--exclude "

TCL_INCDIR="-I /usr/local/include/tcl8.6"
TCL_LIB="tcl86"
TCLSH=/usr/local/bin/tclsh8.6

WISH=/usr/local/bin/wish8.6
EXPECT=/usr/local/bin/expect

DB_INCDIR="-I /usr/local/include/db5"
DB_LIB="db-5"
DB_BINDIR=/usr/local/bin/db5

LOCAL_LIBDIR="-L /usr/local/lib"
LOCAL_INCDIR="-I /usr/local/include"

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
   ubuntu=0
   if [ -f /etc/os-release ]
   then
	. /etc/os-release
	ubuntu=$VERSION_ID
   fi
fi

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD)
	CCWFLAGS="-Wall -Wno-char-subscripts -fno-strict-aliasing"
	PREFIX=usr/pkg
        TCL_INCDIR="-I /usr/pkg/include"
        TCL_LIB="tcl84"
        TCLSH=/usr/pkg/bin/tclsh
	WISH=/usr/pkg/bin/wish
        DB_INCDIR="-I /usr/pkg/include/db4"
        DB_LIB="db4-4.4"
	DB_BINDIR=/usr/pkg/bin/db44
        LOCAL_LIBDIR="-L /usr/pkg/lib"
	LOCAL_INCDIR="-I /usr/pkg/include"
        ;;
    OpenBSD) 
        # use the defaults
	;;
    Linux)
	INCLUDE=include
	Q=
	SUFFIXRULES="%.o: %.c"
	EXCLUDE="--exclude "

	TCL_INCDIR=
	TCL_LIB="tcl8.5"
	TCLSH=/usr/bin/tclsh8.5
	WISH=/usr/bin/wish
	EXPECT=/usr/bin/expect
	#
	DB_INCDIR=
	DB_BINDIR=
	DB_LIB="db-5.1"

	LOCAL_INCDIR=
	LOCAL_LIBDIR=

	if [ $flavor = centos ]
	then
	   TCL_INCDIR=
	   TCL_LIB="tcl8.5"
	   TCLSH=/usr/bin/tclsh8.5
	   DB_INCDIR=
	   DB_BINDIR=
	   DB_LIB="db-4.7"
	elif [ $flavor = debian ]
	then
	   TCL_INCDIR="-I /usr/include/tcl8.5"
	   TCL_LIB="tcl8.5"
	   TCLSH=/usr/bin/tclsh8.5
	   DB_INCDIR=
	   DB_LIB="db-5.1"
	   [ $ubuntu != "0" ] && DB_LIB="db-5.1"
    	fi
	;;
    Darwin)
	INCLUDE="include"
	Q=
	SUFFIXRULES="%.o: %.c"

	TCL_INCDIR=
	TCL_LIB="tcl8.4"
	TCLSH=/usr/bin/tclsh
	WISH=/sw/bin/wish
	EXPECT=/sw/bin/expect
	#
	DB_INCDIR="-I /usr/local/include/db46"
	DB_LIB="db-4.6"

	LOCAL_INCDIR="-I /usr/include -I /usr/local/include -I /sw/include"
	LOCAL_LIBDIR="-L/usr/lib -L/usr/local/lib -L/sw/lib"
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
      -e "/@CC@/ s||$CC|" \
      -e "/@CCWFLAGS@/ s||$CCWFLAGS|" \
      -e "/@SUFFIXRULES@/ s||$SUFFIXRULES|" \
      -e "/@EXCLUDE@/ s||$EXCLUDE|" \
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
      $makefile_in > $makefile_out
}
