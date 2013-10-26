#
# $Id$
#
. ../configure.inc

COMMON_LIBDIRS="-L libconnth -L libqdb -L libspoolbdb"
COMMON_LIBS="-lconn -lqdb -lspoolbdb -lwrap -lz"

# defaults (FreeBSD)
INCDIRS="$TCL_INCDIR $DB_INCDIR"
LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"

DEFINES="-DHAS_CLOCK_GETTIME"

case $os in
    FreeBSD) 
        # use the defaults
	;;
    NetBSD) 
        # use the defaults
	;;
    OpenBSD) 
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lm -lutil -lpthread"
	;;
    SunOS)
	INCDIRS="-I /opt/sfw/include -I /usr/sfw/include"
	LIBDIRS="$COMMON_LIBDIRS -L /opt/sfw/lib -L /usr/sfw/lib"
	LIBS="$COMMON_LIBS -lpthread -lsocket -lnsl -lresolv"
	;;
    Linux)
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread -lrt"

	# In Ubuntu-12.04-i386 the default db-5.1 library apparently
	# cannot be used in a threaded application. We will use the
	# locally (static) compiled version, for both amd and i386.
	#
	## if [ $ubuntu = "12.04" ]
	## then
	##     _BDBDIR="/usr/local/BerkeleyDB.5.1"
	##     INCDIRS="$TCL_INCDIR -I ${_BDBDIR}/include"
	##     LIBDIRS="$COMMON_LIBDIRS -L ${_BDBDIR}/lib"
	## fi
	;;
    Darwin)
	COMMON_LIBDIRS="-L./libconnth -L./libqdb -L./libspoolbdb"
	LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"

	DEFINES=
	;;
esac
