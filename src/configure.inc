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
	LIBDIRS=$COMMON_LIBDIRS
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread -lrt"

	# For CentOS use the local db if we have it
	[ $flavor = centos ] && \
		LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
	;;
    Darwin)
	COMMON_LIBDIRS="-L./libconnth -L./libqdb -L./libspoolbdb"
	LIBDIRS="$COMMON_LIBDIRS $LOCAL_LIBDIR"
	LIBS="$COMMON_LIBS -l${TCL_LIB} -l${DB_LIB} -lpthread"

	DEFINES=
	;;
esac
