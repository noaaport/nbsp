# bugdb.tcl.tcl --
#
#    Procedures to support the bug database sample application.

package require Mk4tcl
package provide bugdb 1.0

namespace eval bugdb {
    namespace export *
}

# bugdb::insert --
#
#    Insert a new bug report into a Metakit database.
#
# Arguments:
#    Application	Application name
#    OS			OS where bug occurs
#    Priority		Level of impact of the bug
#    Assigned		Email address of the assignee
#    Summary		A short summary of the problem
#    Description	More in-depth information regarding the problem
#
# Results:
#    0 if database insert was successful
#    1 if database insert failed
#

proc bugdb::insert {Application OS Priority Assigned Summary Description} {
    # set date
    set date [clock format [clock seconds] ]

    # Open the db
    mk::file open bugdb ../sampleapp/bugdb/bugdb.mk

    set result [catch {mk::row append bugdb.bugs Application "$Application" OS "$OS" \
    Priority "$Priority" Assigned "$Assigned" Summary "[special-chars $Summary]" \
    Description "[special-chars $Description]" date "$date" Status "New"} msg]

    mk::file commit bugdb

    # Close the db
    mk::file close bugdb

    return $result
}

# bugdb::bug-list --
#
#    Return a list of bugs from the Metakit database.
#
# Arguments:
#    None
#
# Results:
#    Results variable which is a list of bug information
#

proc bugdb::bug-list {} {

    # Open the db
    mk::file open bugdb ../sampleapp/bugdb/bugdb.mk

    set results {}
    set rows [mk::select bugdb.bugs]

    foreach row $rows {
        
	# Get the contents for that row
	lappend results $row \
	                [mk::get bugdb.bugs!$row date] \
	                [mk::get bugdb.bugs!$row Application] \
	                [mk::get bugdb.bugs!$row OS] \
	                [mk::get bugdb.bugs!$row Priority] \
	                [mk::get bugdb.bugs!$row Summary] \
	                [mk::get bugdb.bugs!$row Assigned]
    }

    # Close the db
    mk::file close bugdb

    return $results
}

# bugdb::bug-details --
#
#    Returns the details of a single bug database entry
#
# Arguments:
#    bug		The bug ID in the Metakit database
#    Application	Result variable returned by upvar
#    OS			Result variable returned by upvar
#    Priority		Result variable returned by upvar
#    Assigned		Result variable returned by upvar
#    Summary		Result variable returned by upvar
#    Description	Result variable returned by upvar
#    Date		Result variable returned by upvar
#    Status		Result variable returned by upvar
#    Updated		Result variable returned by upvar
#
# Results:
#    See results returned above.
#

proc bugdb::bug-details {bug Application OS Priority Assigned \
Summary Description Date Status Updated} {

    upvar 1 $Application my_application
    upvar 1 $OS my_os
    upvar 1 $Priority my_priority
    upvar 1 $Assigned my_assigned
    upvar 1 $Summary my_summary
    upvar 1 $Description my_description
    upvar 1 $Date my_date
    upvar 1 $Status my_status
    upvar 1 $Updated my_updated

    # Open the db
    mk::file open bugdb ../sampleapp/bugdb/bugdb.mk

    set row [mk::get bugdb.bugs!$bug]

    # Close the db
    mk::file close bugdb

    set my_application [lindex $row 1]
    set my_os [lindex $row 3]
    set my_priority [lindex $row 5]
    set my_assigned [lindex $row 7]
    set my_summary [lindex $row 9]
    set my_description [lindex $row 11]
    set my_date [lindex $row 13]
    set my_status [lindex $row 15]
    set my_updated [lindex $row 17]

    return
}

# bugdb::update --
#
#    Update an existing bug in the Metakit database.
#
# Arguments:
#    Bug		Bug ID in the Metakit database
#    Status		The bugs updated status
#    Application	Application name
#    OS			OS where bug occurs
#    Priority		Level of impact of the bug
#    Assigned		Email address of the assignee
#    Summary		A short summary of the problem
#    Description	More in-depth information regarding the problem
#
# Results:
#    None
#

proc bugdb::update {Bug Status Application OS Priority Assigned Summary Description} {
    # set date for last updated field
    set date_updated [clock format [clock seconds] ]
    
    # Open the db
    mk::file open bugdb ../sampleapp/bugdb/bugdb.mk

    # Update the db
    mk::set bugdb.bugs!$Bug Status "$Status" Application "$Application" \
    OS "$OS" Priority "$Priority" Assigned "$Assigned" Summary "[special-chars $Summary]" \
    Description "[special-chars $Description]" Updated "$date_updated"

    # Close the db
    mk::file close bugdb

    return
}

# bugdb::special-chars --
#
#    Substitute characters that can break HTML with appropriate
#    character entities.
#
# Arguments:
#    value		String potentially containing characters to replace
#
# Results:
#    String with bad characters replaced by character entities
#

proc bugdb::special-chars {value} {
    # Substitute for characters that break the HTML
    regsub -all {\"} $value {\&quot;} safe_value
    regsub -all {<} $safe_value {\&lt;} safe_value
    regsub -all {>} $safe_value {\&gt;} safe_value
    return $safe_value
}
