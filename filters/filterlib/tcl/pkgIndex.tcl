#
# $Id$
#
package ifneeded nbsp::errx 1.0 \
    [list source [file join $dir errx.tcl]]

package ifneeded nbsp::hscheduler 1.0 \
    [list source [file join $dir hscheduler.tcl]]

package ifneeded nbsp::mscheduler 1.0 \
    [list source [file join $dir mscheduler.tcl]]

package ifneeded nbsp::util 1.0 \
    [list source [file join $dir util.tcl]]

package ifneeded nbsp::periodic 1.0 \
    [list source [file join $dir periodic.tcl]]

package ifneeded nbsp::syslog 1.0 \
    [list source [file join $dir syslog.tcl]]

#
# Not yet converted to use namespaces
#
package ifneeded nbsp::filterslib 1.0 \
    [list source [file join $dir filters.lib]]

package ifneeded nbsp::filterserrlib 1.0 \
    [list source [file join $dir filters-err.lib]]

package ifneeded nbsp::filtersgribidlib 1.0 \
    [list source [file join $dir filters-gribid.lib]]

# filters/gisfilter/gismap
package ifneeded nbsp::gis 1.0 \
    [list source [file join $dir gis.tcl]]

package ifneeded nbsp::radstations 1.0 \
    [list source [file join $dir radstations.tcl]]
