# snmp.tcl --
# Sample SNMP application.
# This uses the Scotty package for snmp access.
# This implements ".snmp" HTML template files that have
# in-line Tcl that calls out to the routines here to do SNMP
# and display the results.
#
# Stephen Uhler  (c) 1997 Sun Microsystems
# Brent Welch (c) 1998-2000 Ajuba Solutions
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# RCS: @(#) $Id$

package provide httpd::snmp 1.0

# Handle .snmp templates.
# First process the incoming form data in an SNMP specific way,
# then do a normal Subst into a safe interpreter
# Use the session management stuff.
#   path:	The path name of the document
#   suffix:     The file suffix
#   sock:	The name of the socket, and a handle for the socket state

proc Doc_application/x-tcl-snmp {path suffix sock} {
    upvar #0 Httpd$sock data
    global Snmp

    append data(query) ""
    set queryList [Url_DecodeQuery $data(query)]

    # Find the current session (or start a new one if session=new).

    set session [Session_Match $queryList Snmp error]
    if {$session == {}} {
	# Branch to a new session page.
	if [catch {Httpd_RedirectSelf $Snmp(page,session) $sock} err] {
	    Httpd_ReturnData $sock text/html $error
	}
	return
    }

    # Process the query data from the previous page.

    if [catch {SnmpProcess $session $queryList} result] {
	Httpd_ReturnData $sock text/html $result
	return
    } 

    # Expand the page in the correct session interpreter, or treat
    # the page as ordinary html if the session has ended.

    if {$result} {
	Subst_ReturnFile $sock $path interp$session
    } else {
	Httpd_ReturnFile $sock text/html $path
    }
}

# Process the result of the previous form in an snmp specific way.
# Variables which are valid writable mib's are used to set the device.
# Other variables are made available for use later in the form.
# The following form names are treated specially:
#   group		- Sets the group within this session
#   host		- Sets the snmp host
#   exit=1		- Terminates the session
#   listvar		- Defines a list variable
# Parameters
#   session:  the session id
#   query:    a list of names and values produced by Url_DecodeQuery

proc SnmpProcess {session query} {

    upvar #0 Session:$session state
    set group FormState
    set interp $state(interp)

    # One-time snmp initialization.

    if {![info exists state(snmp)]} {
	set state(snmp) [SnmpInit]
    }
    # Process each query item.
    # Some items, such as "session", "group", and "host" are treated
    # specially.
    # Upon completion, zero or more of the following may occur:
    #   values are set in the appropriate "group" arrays
    #   the snmp host is modified
    #   a list of writable mibs is produced

    foreach {name value} $query {
	if {[string match "group" $name]} {
	    if {$value == {}} {
		set group FormState
	    } else {
		set group $value
	    }
	} elseif {[string match "exit" $name] && $value} {
	    Session_Destroy $session
	    return 0
	} elseif {[string match "host" $name]} {
	    Snmp_host $session $value
	} elseif {[catch {mib access $name} access]} {
	    # Not a MIB name
	    interp eval $interp [list set ${group}($name) $value]
	} elseif {[regexp write $access]} {
	    # A writable MIB name that we are setting
	    lappend list [list $name $value]
	} else {
	    # A read-only MIB name
	    interp eval $interp [list set ${group}($name) $value]
	}
    }

    # Set the device with these mibs.

    if {[info exists list]} {
	if {[catch {$state(snmp) set $list} result]} {
	    interp eval $interp [list set snmp-failure $result]
	}
    }
    return 1
}

# Create an snmp connection for this session.  This is done
# once.

proc SnmpInit {} {
    package require Tnm
    set snmp [snmp session]
    # If you don't load any mibs, then all available mibs are
    # loaded at initialization
    #mib load rfc1213.mib
    return $snmp
}

# Set the session start page.
proc Snmp_SessionPage {path} {
    global Snmp
    set Snmp(page,session) $path
}

#############################################################################
# Snmp building blocks.  These are available as aliases in the session
# interpreter, and may be embedded in snmp web pages.

# Set the session host. Return {} if successful, an error message
# otherwise. (This needs a better error handling strategy). An empty
# host name is a no-op.

proc Snmp_host {session host} {
    upvar #0 Session:$session state
    if {$host != {}} {
	if {[catch {$state(snmp) configure -address $host} msg]} {
	    return $msg
	} else {
	    set state(host) $host
	}
    }
    return ""
}

# Generate a form or table of MIB data
# All of the mibs under a list of nodes is displayed in an html table.
# The various options control which mibs and attributes will be displayed.

#  Options for mibTable
#  method:	http submission method (POST/GET)
#  caption: 	The table caption
#  writeOnly	Only display writable mibs
#  readonly	Don't generate a form - just a table
#  matchName:	regexp to match (or ! match) symbolic mib names
#  matchValue:  regexp to match (or ! match) mib values
#  group:	a list of "group" to import options from
#  submit:	The text of the submit button
#  mib:		A list of leaves or nodes in the mib tree.  Every item below these
#		nodes are displayed in the table.  If the list contains more
#		than one node, then the leaf values for each node are
#		displayed on the same row.
#  border:	Draw a border around the table (1/0)
#  order:	The order of columns in the table.  Contains 1 or more of:
#		   ident		Text name of the mib
#		   description,		Verbose description of the mib
#		   id,			Numerical value (e.g. 1.3.1.6.}
#		   type,		Mib type
#		   port,		Trailing digit(s) of the id
#		   value		Current value
#		   full			fully qualified name
#		A new column is created for each item specified, subject
#		to the setting of its corrosponding "OK" value.  Example: for
#		"type" to be displayed, "type" must be present in "order"
#		*and* "typeOK" must be true.
#  host		The host to use for this query
#  colors	A list of color/tcl-expression pairs.  Each expression
#		is evaluated for every mib.  If the expression evaluates
#		to "true", the table entry is set to "color".  Expressions
#		may use the variables "$value" and "$writable" as part of
#		the expression. "$value" is the current value of the mib,
#		and "$writable" is a boolean, true if this mib may be written.
#  specify	The default columns, as determined by "order" are overridden.
#		I'll have to figure out how to explain how.  It is useful
#		when multiple mibs are displayed in the same row, and different
#		attributes are displayed for each mib.
#  heading	A list of column headings for the table.

proc Snmp_mibTable {session args} {

    array set options {
       -method POST
       -caption {}
       -writeOnly 0
       -readOnly 0
       -matchName {}
       -matchValue {}
       -group {}
       -submit Update
       -mib system
       -border 1
       -order {ident description id type port value}
       -host {}
       -colors {}
       -specify {}
       -heading {}
       -port {}

       -identOK 1
       -descriptionOK 0
       -idOK 0
       -typeOK 0
       -portOK 0
       -valueOK 1
       -fullOK 0
    }

    # Patterns that cause match failures:
    # x->no pattern  0->match-ok   1->not-ok

    array set kill {x1 1   1x 1  11 1}

    # Retrieve all the options, either from the command line (query)
    # or from the proper group arrays.

    upvar #0 Session:$session state
    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
	Session_Import options $table interp$session
    }
    
    # Process the various table options.

    Snmp_host $session $host
    if {$caption == {}} {
	catch {set caption "$state(host) $mib configuration"}
    }
    append result "<table[expr {$border ?" border" : ""}]>\n"
    append result "<caption>$caption</caption>\n"

    if {$heading != {}} {
    	append result "<tr>"
    	foreach i $heading {
	    append result "<th>$i</th>"
	}
    	append result "</tr>\n"
    }

    if {!$readOnly} {
	append result "<form method=$method>\n"
	append result "<input type=hidden name=session value=$session>"
    }

    # "match" regeexps may be inverted by preceeding them with a "!".
    # Keep track of the "!" setting

    set notValue [regexp -- "^!(.*)" $matchValue {} matchValue]
    set notName  [regexp -- "^!(.*)" $matchName {} matchName]

    # Remove all "turned off" items from the order list.

    set fields ""
    foreach item $order {
    	if {![info exists ${item}OK]} {
	    continue
	}
	if {[set ${item}OK]} {
	    lappend fields $item
	}
    }

    # Enumerate all of the mibs under this list of nodes
    # of the device.  This can fail with a no-response error.
    # We need to decide what to do about that.

    if [catch {
	$state(snmp) walk vars $mib {
	    # puts "mib is: <$vars>"
	    set specifyIndex 0
	    set row ""
	    set skip 0
    
	    # If $mib is a list, then $vars will be too.
    
	    foreach var $vars {
		if {$skip} continue
		foreach {name type value} [mib name $var] {}
		# puts "  name,type,value=$name,$type,$value"
    
		# Skip all mibs that don't match the desired critereon.
    
		set novalue x
		if {$matchValue != ""} {
		    set novalue 0
		    set match [regexp  -- $matchValue $value]
		    if {!($notValue ^ $match)} {
			set novalue 1
		    }
		}
    
		set noname x
		if {$matchName != ""} {
		    set noname 0
		    set match [regexp  -- $matchName $name]
		    if {!($notName ^ $match)} {
			set noname 1
		    }
		}
    
		# puts "Match ($noname$novalue $name,$value)"
		if {[info exists kill($noname$novalue)]} {
		    incr skip
		    continue
		}
    
		if {$port != {}} {
		    if {![regexp -- ".$port\$" $name]} {
			incr skip
			continue
		    }
		}
    
		set writeable [expr {[regexp write [mib access $name]]}]
		if {$writeOnly && !$writeable} {
		    incr skip
		    continue
		}
    
		# Compute the data for this column element
    
		# Experimental color stuff
		# for color/expr pair, if <expr> is true, then set the color
		# The values "value" and "writeable" are made available in
		# the session interpreter.
    
		set color ""
		foreach  {col expr} $colors {
		    set true 0
		    interp eval interp$session "
			set value [list $value] 
			set writeable $writeable
			"
		    catch {set true [interp eval interp$session [list expr $expr]]} foo
		    if {$true} {
			set color " bgcolor=\"$col\""
			break
		    }
		}
		set td "<td$color>"
    
		# Override the mib attributes for each column, as needed.
    
		if {$specify != ""} {
		    set fields [lindex $specify $specifyIndex]
		    incr specifyIndex
		}
    
		# Select the proper attribute(s) for this mib.
    
		foreach item $fields {
		    switch $item {
			ident {
			    append row "$td<b>$name</b>"
			}
			id {
			    append row "$td[lindex $var 0]"
			}
			type {
			    append row "$td$type"
			}
			full {
			    set desc [Snmp_DisplayMib $session $name -type full]
			    append row "$td$desc"
			}
			port {
			    set desc [Snmp_DisplayMib $session $name -type port]
			    append row "$td$desc"
			}
			description {
			    set desc [Snmp_DisplayMib $session $name -type description]
			    append row "$td$desc"
			}
			value {
			    if {$writeable && !$readOnly} {
				append row "$td[Snmp_setMib $session $name]"
			    } else {
				append row "$td$value"
			    }
			}
		    }
		}
	    }
    
	    # Collect the data for this row, if any.
    
	    if {!$skip && $row != ""} {
		append result <tr>$row</tr>\n
	    } else {
		append result "<!-- skipping row -->\n"
	    }
	}
    } err] {
	append result $err
    }

    if {!$readOnly} {
	append result "<tr><td colspan=5><input type=submit value=\"$submit\">"
	append result "</tr>\n"
	append result </form>\n
    }
    append result </table>\n
}

# Get an item as a form element

proc Snmp_setMib {session mib} {
    upvar #0 Session:$session state

    foreach {num type value}  [lindex [$state(snmp) get $mib] 0] {}
    set names [lindex [mib tc $mib] 3]
    if {[llength $names] >1} {
    	append result "<select name=\"[mib name $mib]\">\n"
    	foreach name $names {
	    lassign-brent {choice index} $name
	    set s [expr {("$value" == "$choice") ? "SELECTED" : ""}]
	    append result "  <option value=$index$s>$choice\n"
	}
	append result "</select>"
    } else {
    	append result "<input name=\"[mib name $mib]\" value=\"$value\">"
    }
    return $result
}

# Set the session name from a page, and an optional session group.

proc Snmp_formSession {session args} {
    upvar #0 Session:$session state

    array set options {
    	-new 0
    	-group {}
    }
    optionSet options $args

    if {$new} {
    	set session new
    }
    append result "<input type=hidden name=session value=\"$session\">"
    if {$group != {}} {
	append result [Snmp_formGroup $session $group]
    }
    return $result
}

# set the group

proc Snmp_formGroup {session name} {
    upvar #0 Session:$session state
    set state(group) $name
    append result "<input type=hidden name=group value=\"$name\">"
    return $result
}

# Generate a selection item.
# The current value in the group array is "selected".
# The options for select:
#   name:	the name of the variable
#   group:	the group name 
#   choices:	a list of choices 
#   selected	the name of the selected choice, which is overriden by the
#		current value of "name" in the "group" array.
#   Show:	How many items to display at once
#   noempty:	Don't output anything if selection would be empty
#   submit:	Add a submit button

proc Snmp_select {session name args} {
    upvar #0 Session:$session state

    array set options {
       -group {}
       -choices {}
       -selected {}
       -show 1
       -noempty 0
       -submit {}
    }

    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
	Session_Import options $table interp$session
    }
    append result "<!--  select group $group -->\n"
    append result "<select name=$name size=$show>\n"
    catch {set selected [interp eval interp$session [list set ${group}($name)]]} foo
    foreach choice $choices {
	if {$choice == $selected} {
	    append result "  <option SELECTED>$choice\n"
	} else {
	    append result "  <option>$choice\n"
    	}
    }	
    append result "</select>\n"
    if {$submit != {}} {
    	append result "<input type=submit value=\"$submit\">\n"
    } 

    if {$noempty && [llength $choices] == 0} {
    	return ""
    }
    return $result
}

# Create a radio button to indicate on/off (in a form)
#  group:  Which global array to use
#  name:    The name of the variable
#  table:   1/0 output table directives
#  selected 1/0 which item is selected
#  labels   2 element list of radio button labels
#  values   2 element list of the off and on values
#  default  0/1 Which element is the default, if no value is already set.

proc Snmp_radio {session args} {
    upvar #0 Session:$session state

    array set options {
       -group FormState
       -name {}
       -istable 1
       -selected 0
       -labels {off on}
       -values {0 1}
       -default 0
    }

    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
	Session_Import options $table interp$session
    }
    set default [expr {$default ? 1 : 0}]
    set current [lindex $values $default]
    catch {set current [interp eval interp$session [list set ${group}($name)]]} foo

    append result "<!--  radio group $group  name $name -->\n"
    if {$istable} {
    	set td <td>
    }
    append td ""
    append result "$td<input type=radio name=$name value=[lindex $values 0]"
    append result "[expr {[string compare $current [lindex $values 0]] ? {} : { checked}}]>"
    append result [lindex $labels 0]
    append result "$td<input type=radio name=$name value=[lindex $values 1]"
    append result "[expr {[string compare $current [lindex $values 1]] ? {} : { checked}}]>"
    append result [lindex $labels 1]
    return $result
}

# display a mib parameter
#   host: specify the snmp host.
#   group: set the session group
#   type: one of:
#       name, id value description type access choices full name
#   mibleaf:  al leaf mib item to extract the parameter from.

proc Snmp_DisplayMib {session mibleaf args} {
    upvar #0 Session:$session state

    array set options {
       -host {}
       -group {}
       -type value
       -mibleaf {}
    }
    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
	Session_Import options $table interp$session
    }
    Snmp_host $session $host

    switch -glob -- $type {
    	i* {				# numerical id
	    return [mib oid $mibleaf]
	}
    	p* {				# port number (e.g. leaf #)
	    return [lindex [split [mib oid $mibleaf] .] end]
	}
    	n* {				# text name
	    return [mib name $mibleaf]
    	}
    	f* {				# fully qualified name
	    foreach i [split [mib oid $mibleaf] .] {
	    	lappend j $i
	    	lappend result [mib name [join $j .]]
	    }
	    return [join $result .]
    	}
	v* {				# current value
	    if {[catch {$state(snmp) get $mibleaf} x]} {
		return {}
	    }
	    return [lindex [lindex $x 0] end]
    	}
    	d* {				# description
	    regsub -all "\[ \t\n\]+" [mib description $mibleaf] { } result
	    return $result
	}
	t* {				# variable type
	    return [lindex [mib tc $mibleaf] 1]
	}
	a* {				# access
	    return [mib access $mibleaf]
	}
	c* {				# value choices
	    set names ""
	    foreach name [lindex [mib tc $mibleaf] 3] {
	    	lappend names [lindex $name 0]
	    }
	return [join $names ,]
	}
    }
}

# Get a variable as input

proc Snmp_input {session varname args} {
    upvar #0 Session:$session state

    array set options {
       -varname {}
       -group {}
       -size {}
       -host {}
       -value {}
    }
    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
	Session_Import options $table interp$session
    }
    Snmp_host $session $host
    catch {set value [interp eval interp$session [list set ${group}($varname)]]} foo
    append result "<input name=$varname value=\"$value\""
    if {$size != ""} {
    	append result " size=$size"
    }
    append result ">\n"
    return $result
}

# Generate a mib heirarchy for interactive browser traversal.
#  descriptionTarget	Target frame for leaf item description (BROKEN)
#  group		Where to find the override parameters
#  host			Which host to get data from
#  mib 			Current point in the mib heirarchy
#  name2		Cause a submit button to be generated
#  page			The target page for "up" or "down" links
#  page2		The target page for the "Submit" item
#  part			up|down|all: Which part of the heirarchy to draw
#  target		The target frame for heirarchy items
#  target2		The target frame for the Submit button

proc Snmp_Walk {session args} {
    array set options {
    	-mib iso.org.dod.internet
    	-page {}
    	-part all
	-group {}
    	-target {}
    	-target2 {}
	-name2 {}
	-page2 {}
    	-host {}
    	-descriptionTarget {}
    }

    upvar #0 Session:$session state
    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
        Session_Import options $table interp$session
    }

    Snmp_host $session $host

    append item ""

    array set map_color {
      read-only		red
      read-write	orange
      not-accessible	black
    }

    # Compute the part of the tree above the current node.

    if {$target != ""} {
    	set go " target=\"$target\""
    }
    append go ""
    set up ""
    foreach i [split [mib oid $mib] .]  {
	lappend j $i
	set oid [join $j .]
	append up "<a href=$page?session=$session&group=$group&mib=$oid$go>[mib name $oid]</a><br>" \n
    }

    # Create a "submit" button if requested.

    if {$name2 != {}} {
    	if {$page2 == {}} {
	    set page2 $page
	}
    	if {$target2 == {}} {
	    set target2 $target
	}
	if {$target2 != ""} {
	    set go2 " target=\"$target2\""
	}
	set submit "<b><a href=$page2?session=$session&group=$group&$name2=$oid$go2>[mib name $oid]</a>"
	# Use this for submit at the top!
	# set up "$submit\n<hr>\n$up"
	append up "\n<hr>\n$submit"
    }

    # Compute "down" list - items below current node.

    if {$descriptionTarget != ""} {
    	set go2 " target=\"$descriptionTarget\""
    } else {
    	set go2 $go
    }
    set oid [mib oid $mib]
    set children [mib successor $oid]
    set down ""
    foreach i $children {
    	set access [mib access $i]

    	# Distinguish between interior and leaf nodes.

    	if {$access != "not-accessible" && \
    		[regexp {(.*)\.(.*)} $i {} root leaf]} {
	    set href $page?session=$session&group=$group&mib=$root&item=$leaf
	    set targ $go2
    	} else {
	    set href $page?session=$session&group=$group&mib=$i
	    set targ $go
    	}

    	# Add the link for this item.

    	set color $map_color($access)
    	append down "<font color=$color>*&nbsp;"
    	append down "<a href=$href$targ>"
    	append down "[mib name $i]"
    	append down "</font>"
    	append down "</a><br>" \n
    }

    if {$item != {}} {
	set mib [mib oid $mib.$item]
	set descr "<center><h2>[mib name $mib]</h2></center>"
	append descr "[mib description $mib]"
    }
    append descr ""

    # Assemble the final output.

    switch -glob -- $part {
	u* {
	    set result "$up<a name=end></a>"
	}
	do* {
	    set result $down
	}
	de* {
	    set result  $descr
	}
	default {
	    set result "<table><tr><td>$up<td>$down</tr>"
	    append result "<tr><td colspan=2>$descr</tr></table>"
	}
    }
    return $result
}

# Discovery stuff.  Redo the discovery every time (for now).
#  group		where to store/get the options
#  net			which network to do discovery on
#  cache 		use previously cached results, if any.
#  format		names, numbers, or descriptions
#  sort			unused
#  incache		no output if info is not already cached

proc Snmp_discover {session args} {
    array set options {
	-group {}
    	-net {}
	-cache 1
    	-format names
	-sort 0
	-incache 0

	-delay 25
	-window 255
	-retries 3
	-timeout 2
    }

    upvar #0 Session:$session state
    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
        Session_Import options $table interp$session
    }
    # Do the discovery

    if {$net == {}} {
	set net [MyNet]
    }
    upvar #0 Discover:$net discover
    if {$incache && ![info exists discover]} {
	return ""
    }

    set renew 0
    if {!$cache || ![info exists discover]} {
	SnmpDiscover $net SnmpDebug $delay $window $retries $timeout
	set discover(timestamp) [clock seconds]
	set renew 1
    }

    # Display the output

    set result ""
    switch -glob -- $format {
	form  -
	name* {			# List the host names
	    foreach {name value} [array get discover *.*] {
		set host [lindex $value end]
		if {$host == "(unknown)"} {
		    set host $name
		}
		lappend result $host
	    }
	}
	numbers {	# List the host numbers
	    foreach {name value} [array get discover *.*] {
		lappend result $name
	    }
	}
	desc* {		# List the host descriptions
	    foreach {name value} [array get discover *.*] {
		lappend result [lindex $value 0]
	    }
	}
    }
    if {$sort} {
	set result [lsort $result]
    }

    if {$format == "form"} {
	foreach item $result {
	}
    }
    return $result
}

proc SnmpDebug {args} {
    #Stderr [join $args]
}

# A simple combo box.  Output a form with a list
# I don't know what this does yet

proc Snmp_combo {session args} {
    array set options {
	-group {}
	-varname list
	-reset {}
	-action {}
	-show 1

	-select {}
	-add {}
    }

    upvar #0 Session:$session state
    catch {set group $state(group)}
    optionSet options $args
    foreach table $group {
        Session_Import options $table interp$session
    }

    # If add or select is specified, just modify the current
    # list then return

    if {$add == {} && $select != {}} {
    	set add $select
    }
    #puts "add <$add> select <$select>"

    if {$add != {}} {
	#puts "adding $add"
	set list ""
	catch {set list [interp eval interp$session \
		[list set ${group}(${varname}s)]]} foo

	# Add all items onto the list, making sure no duplicates are added.

	foreach i $list {
	    set tmp($i) 1
	}

	foreach i $add {
	    if {![info exists tmp($i)]} {
		interp eval interp$session \
			[list lappend ${group}(${varname}s) $i]
	    set tmp($i) 1
	    }
	}

	if {$select != {}} {
	    interp eval interp$session \
		    [list set ${group}($varname) $select]
	}
    	# return ""
    }

    # Build the combo box.

    if {$action != {}} {
    	set action " action=\"$action\""
    }
    append result "<form method=GET$action>\n"
    append result "<input type=hidden name=session value=\"$session\">\n"
    append result "<input type=hidden name=group value=\"$group\">\n"

    # output selection box

    catch {set select [interp eval interp$session \
	    [list set ${group}($varname)]]} foo
    catch {set list [interp eval interp$session \
	    [list set ${group}(${varname}s)]]} foo

    if {[info exists list]} {
	append result "<select name=$varname size=$show>\n"
	foreach choice $list {
	    if {$choice == $select} {
		append result "  <option SELECTED>$choice\n"
	    } else {
		append result "  <option>$choice\n"
	    }
	}	
	append result "</select>\n"
	append result "<input type=submit value=\"select\">"
    }
    append result "</form>\n"

    append result "<form method=GET$action>\n"
    append result "<input type=hidden name=session value=\"$session\">\n"
    append result "<input type=hidden name=group value=\"$group\">\n"
    append result "<input name=add>"
    append result "<input type=submit value=\"add to list\">"
    append result "</form>\n"
    return $result
}

# return the network number or host number

proc Snmp_netId {args} {
    MyNet
}    

proc Snmp_hostId {args} {
    MyIpaddr
}    

# Get my ip address.
 
proc MyIpaddr {} {
    set addr ""
    if {[catch {dns address [info hostname]} addr]} {
        set server [socket -server # 0]
        set port [lindex [fconfigure $server -sockname] 2]
        set host [lindex [fconfigure $server -sockname] 1]
        set client [socket $host $port]
        set addr [lindex [fconfigure $client -sockname] 0]
        close $client
        close $server
    }
    return $addr
}

# Return the network part of the address (assume class C).

proc MyNet {} {
    set net ""
    regexp {(.*)\..*} [MyIpaddr] {} net
    return $net
}

# Look for all snmp hosts on a  (class C) network.
# Return an array containing the discovery info.

proc SnmpDiscover {{net {}}  {callback #} {delay 25} {window 255}
		    {retries 3} {timeout 7}} {
    if {$net == {}} {
	set net [MyNet]
    }
    for {set i 1} {$i < 255} {incr i} {
        set s [snmp session -address $net.$i -delay $delay -window $window \
               -retries $retries -timeout $timeout]
        $s get sysDescr.0 [list DiscoverCallback $net $i $callback %S %E %V] 
        update
    }
    snmp wait
}

# Callback point for SnmpDiscover
#   net:  which network
#   session: the token for the snmp session
#   error:   the error string
#   descr:   the result of the query

proc DiscoverCallback {net host callback session error {desc {}}} {
    upvar #0 Discover:$net discover
    #Stderr "Discover $net $host $error"
    if {$error == "noError"} {
	catch {$callback $net $host found}
        regsub -all "\[\n\r\]" [lindex $desc 2] "" d
        if [catch {$session get sysName.0} name] {
            set name (unknown)
        }
        set name [lindex [lindex $name 0] end]
        set discover([$session cget -address]) [list $d $name]
    } else {
	catch {$callback $net $host "not found"}
    }
    $session destroy
}
