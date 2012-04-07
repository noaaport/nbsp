#! /bin/sh
#
#	handle the tclhttpd web server
#

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/bin/httpd.tcl
NAME=tclhttpd
DESC=tclhttpd
TCLSH=/usr/bin/tclsh
STARTER="default"
export TCL_HTTPD_LIBRARY=/usr/lib/tclhttpd

test -x $DAEMON || exit 0

# Include tclhttpd defaults if available
if [ -f /etc/default/tclhttpd ] ; then
	. /etc/default/tclhttpd
fi

set -e

case "$1" in
  start)
	echo -n "Starting $DESC with $STARTER: "
	if [ $STARTER = "daemon" ]; then
	    /usr/bin/daemon --name=tclhttpd \
		--chdir /var/www \
		--user=www-data.www-data --umask=005 \
		--respawn --inherit \
		--errlog=daemon.err --output=daemon.debug --dbglog=daemon.debug \
		-- $TCLSH $DAEMON $DAEMON_OPTS
	else
	    start-stop-daemon --start --quiet --background \
		--chuid www-data:www-data \
		--pidfile /var/run/$NAME.pid --make-pidfile \
		--exec $TCLSH -- $DAEMON $DAEMON_OPTS
	fi
	echo "$NAME."
	;;

  stop)
	echo -n "Stopping $DESC: "
	if [ $STARTER = "daemon" ]; then
	    /usr/bin/daemon --user www-data.www-data --name=tclhttpd --stop
	else
	    start-stop-daemon --stop --quiet --pidfile /var/run/$NAME.pid \
		--exec $TCLSH
	fi
	echo "$NAME."
	;;

  restart|force-reload)
	#
	#	If the "reload" option is implemented, move the "force-reload"
	#	option to the "reload" entry above. If not, "force-reload" is
	#	just the same as "restart".
	#
	echo -n "Restarting $DESC: "
	if [ $STARTER = "daemon" ]; then
	    /usr/bin/daemon --user www-data.www-data --name=tclhttpd --restart
	else
	    start-stop-daemon --stop --quiet --pidfile \
		/var/run/$NAME.pid --exec $DAEMON
	    sleep 1
	    start-stop-daemon --start --quiet --pidfile \
		/var/run/$NAME.pid --exec $DAEMON -- $DAEMON_OPTS
	fi

	echo "$NAME."
	;;

  *)
	N=/etc/init.d/$NAME
	# echo "Usage: $N {start|stop|restart|reload|force-reload}" >&2
	echo "Usage: $N {start|stop|restart|force-reload}" >&2
	exit 1
	;;
esac

exit 0
