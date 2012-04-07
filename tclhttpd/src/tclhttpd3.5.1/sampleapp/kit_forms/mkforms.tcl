# DESCRIPTION
#
# Extension of the forms module when using a Metakit database
#
# AUTHOR
#
# Rafael González Fuentetaja
#
# LEGAL STATEMENTS
#
# This file is protected by the BSD License. 

#
#
# $Id$


package require html
package require ncgi
package require mime
package require msgcat

#-------------------------------------------------------------
#
# Form_DBSearchFields
#
#	Searches in a view for a given key
#	The key's value is already in the
#	state($key) array in the slave interpreter
#
# Arguments:
#	
#	sock - the open socket for the response
#	error - the error message 
#	environ - additional environment array 
#
# Returns:
#	Number of rows selected (>1 if key was not a real key).

proc Form_DBSearchFields {session fields} {
   upvar #0 Session:$session state
   
   set db    [db::GetTag $session]
   set view  [db::GetView $session]
   set key   [db::GetKeyName $session]
   set value [interp eval $state(interp) [list set state($key)]]
   
   set result [db::SearchBy $db $view $key $value $fields]
   return [llength $result]
}
   
   
#-------------------------------------------------------------
#
# Form_ViewEditForm
#
#	A simple form with no checking attempts for required fields
#	used to display and edit metakit database fields
#
# Arguments:
#	
#	session - the session number
#	title - the form title
#	fields - list of {type name label} values to generate
#	buttons - list of {label, url} buttons to go after processing.
#	dbdesc - list of {dbase view key} values needed to interface Metakit
#   	size - number of characters of input texts
#
# Results:
#	HTML for the form.


proc Form_ViewEditForm {session title fields buttons {size 64}} {

	upvar #0 Session:$session state
    	set interp $state(interp)
    
	
   	set db    [db::GetTag $session]
   	set view  [db::GetView $session]
   	set key   [db::GetKeyName $session]
    
    	# construct a list of database fields to search for
    	# and the true field structire to be passed to Form_Form
    	# where all fields are optional
    	 
    	foreach {type field label} $fields {
    		lappend dbfields $field
    		lappend truefields 0 $type $field $label
    	}
    	
   	interp eval $interp [list set modify $dbfields]
	set value [interp eval $interp [list set state($key)]]
	
	# merge query data with data from database
	 
	array set query $state(query)
	array set query [db::SearchBy $db $view $key $value $dbfields]
	set state(query) [array get query]
		
 	Form_Form  $session $title $truefields $buttons $size FormViewEditFormHandler		
}

proc FormViewEditFormHandler {formid session action} {
	upvar #0 Session:$session state 
	global Form
		
	set result FORM_SUBST
	if {[string match $action [mc Cancel]]} {  
				
		set result [KillSessionAndRedirect $session $action]
	
	} elseif {[string match $action [mc Save]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

##################################################
##################################################
##################################################

#-------------------------------------------------------------
# The number of users in the database.
#

proc Form_ViewCount {session} {
    upvar #0 Session:$session state
    
   
   set db    [db::GetTag $session]
   set view  [db::GetView $session]
   
   return   [mk::view size $db.$view]
}


#-------------------------------------------------------------
# Display a View in an HTML table
#
proc Form_ViewDisplay {session} {

    set db   [db::GetTag $session]
    set view [db::GetView $session]
    set key  [db::GetKeyName $session]
     
    set dbfields [mk::view info $db.$view]
    
    append html "<TABLE class=dbase><TR>"
    foreach name $dbfields {
      regsub (:\[SILBDF\]) $name "" name
      append html "<TH class=dbase>$db::displayNames($view.$name)</TH>"
    }
    append html "</TR>"
    
 
    foreach row [mk::select $db.$view -sort $key] {
       append html "<TR>"
       foreach {name value} [mk::get $db.$view!$row] {
          append html "<TD class=dbase>"
          if {[string match $value ""]} {
            append html "&nbsp\;"
          } else {
            append html $value
          }
          append html "</TD>"
       }   
    }
    append html "</TABLE>\n"
    return $html
}


#-------------------------------------------------------------
# Display a View in an HTML table
#
proc Form_RowDisplay {session} {

    upvar #0 Session:$session state
    set interp $state(interp)
    
    set db   corweb
    set view [db::GetView $session]
    set key  [db::GetKeyName $session]
    set value [interp eval $interp [list set state($key)]]
    
    set dbfields [mk::view info $db.$view]
    
    append html "<TABLE class=dbase><TR>"
    foreach name $dbfields {
      regsub (:\[SIBLDF\]) $name "" name
      append html "<TH class=dbase>$db::displayNames($view.$name)</TH>"
    }
    append html "</TR>"
    
   
  
     foreach row [mk::select $db.$view -exact $key $value] {
       append html "<TR>"
       foreach {name value} [mk::get $db.$view!$row] {
          append html "<TD class=dbase>"
          if {[string match $value ""]} {
            append html "&nbsp\;"
          } else {
            append html $value
          }
          append html "</TD>"
       }   
    }
    append html "<DIV>\n"
    return $html
}



#-------------------------------------------------------------
#
# Selects a Metakit DBase tag to operate with 
# Valid also for non-session based templates (.tml files) passing {}
# as session id
#
proc Form_DBSetTag {session dbtag} {
	upvar #0 Session:$session state
   
    db::SetTag $session $dbtag
    return ""
}


#-------------------------------------------------------------
#
# Selects a view to operate with (append, delete or modify fields)
# Valid also for non-session based templates (.tml files) passing {}
# as session id
#
proc Form_DBSetView {session view} {
	upvar #0 Session:$session state
   
    db::SetView $session $view
    return ""
}


#-------------------------------------------------------------
#
# Selects the key in the view to operate with (append, delete or modify fields)
# Valid also for non-session based templates (.tml files) passing {}
# as session id
#
proc Form_DBSetKeyName {session key} {
    upvar #0 Session:$session state
	
	db::SetKeyName $session $key
	return ""
}


