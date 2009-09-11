#
# $Id$
#
-Installation instructions for gempak-compatible filtering

1) Create directory

	/var/noaaport/data/gempak

2) Install filter
	- Mandatory: install nbsp.conf and rc files (nbspsat.rc, gpfilter.rc)
	- Optionally: install gpfilter.conf if defaults are not suitable
	- Recommended: logfile rotation (newsyslog.conf) and hourly cron script
	  to purge old data files from gempak data directory.


