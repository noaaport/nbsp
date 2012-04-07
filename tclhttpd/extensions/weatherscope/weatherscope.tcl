#
# $Id$
#

# The various handlers (in the weatherscope subdirectory) are loaded
# after this file by the extensions module.
#
Direct_Url /_weatherscope _weatherscope;

package require httpd::mtype;

# This loads the array weatherscope() in the global context, but it is used
# only by the functions in the weatherscope file.

set _weatherscope(conf) "weatherscope.conf";
set _weatherscope(localconfdirs) $Config(localconfdirs);

#
# Some settings inherit the default values from the dafilter and wsfilter.
#

# nids 
set _weatherscope(rad_basedir) "/var/noaaport/data/digatmos/nexrad/nids";
set _weatherscope(rad_fext) ".nids";
set _weatherscope(rad_mimetype) "image/nids";
set _weatherscope(rad_unzskip) 54;

# metar/ua/grib
set _weatherscope(mdf_fext) ".mdf";
set _weatherscope(mts_fext) ".mts";
set _weatherscope(mdf_mimetype) "application/mdf";
set _weatherscope(mts_mimetype) "application/mts";

# The local overrides
set _weatherscopeconf [file join $Config(confdir) $_weatherscope(conf)];
if {[file exists ${_weatherscopeconf}]} {
    source ${_weatherscopeconf};
}
unset _weatherscopeconf;

## Not needed because the file is not returned but the content
## Mtype_Add $_weatherscope(rad_fext) $_weatherscope(rad_mimetype);
#
Mtype_Add $_weatherscope(mdf_fext) $_weatherscope(mdf_mimetype);
Mtype_Add $_weatherscope(mts_fext) $_weatherscope(mts_mimetype);
