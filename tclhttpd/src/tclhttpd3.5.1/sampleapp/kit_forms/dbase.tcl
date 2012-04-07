# DESCRIPTION
#
# Database description and creation using the Metakit for TCL interface.
# Contains the tables (views in Metakit terminology) for:
# - Registered users
# - Usage right (who is using the observatory and since when)
# Also contains generci database procedures
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


package require Mk4tcl
package provide dbase 1.0

namespace eval db {

	# pretty-print display names to show in forms and tables
	
	variable displayNames
	
	array set displayNames [list \
		v_users.login "Identificador de usuario" \
		v_users.password Contraseña \
		v_users.nombre Nombre \
		v_users.apellidos Apellidos \
		v_users.telefono Teléfono \
		v_users.email "Correo Electrónico" \
		v_users.adminFlag "¿ Es Administrador?"  \
		v_rights.login "Identificador de usuario" \
		v_rights.fechaComienzo "Fecha/Hora de comienzo de derecho de uso" \
		v_rights.duracion "Duración del derecho de uso (minutos)" \
		v_rights.fechaFinal "Fecha/Hora de finalización" \
		v_rights.afterId "Ticket de plazo para expirar" \
	]
	
	
}
proc db::RightsCleanup {} {
    set db     corweb
    set view   v_rights
    
    puts "db::RightsCleanup"
    if {[mk::view size $db.$view]} {
      eval mk::row delete $db.$view!0
      mk::file commit $db
    }
    return 
}

proc db::Init {} {

	set db	 corweb
	set file corweb.dat
	set rightsView v_rights
	set usersView  v_users
	
    mk::file open $db [file join [file dirname $starkit::topdir] $file]
    InitUsers  $db  $usersView
    InitRights $db  $rightsView
    
    # Expire pending access rights even after the server has been down for a while
    # or renew it for the remainder
    
    set count [mk::view size $db.$rightsView]
    if { $count > 0 } {
      set fechaFinal  [eval mk::get $db.$rightsView!0 fechaFinal]
      set fechaActual [clock seconds]
      if { $fechaActual > $fechaFinal } {
        eval mk::row delete $db. $rightsView!0
      } else {
        set timediff [expr {1000*($fechaFinal-$fechaActual)}]
        set afterId  [after $timediff {db::RightsCleanup}]
        eval mk::set $db.$rightsView!0 afterId $afterId
      }
    }  
    mk::file commit $db 
    return   
}

#
# Initializes the users view
#

proc db::InitUsers {db view} {

    global Httpd
    
    mk::view layout $db.$view {login:S password:S nombre:S apellidos:S email:S telefono:S adminFlag:I}

    # See if admin user exists, if not create it so we have access to database.
 
    set adminCreate [mk::select $db.$view -exact login admin]
    
    if {[string match "" $adminCreate]} {
        set adminpass [tclcrypt admin [config::cget CryptSeed]]
        lappend admin_values login admin \
        		     password $adminpass \
                             nombre {Cuenta de Administrador} \
                             apellidos {}  \
                             adminFlag 1
                             
        eval mk::row append $db.$view $admin_values
        append Httpd(Frm_adminCreate) "\nDefault Administrator Account created, please change password!"
        append Httpd(Frm_adminCreate) "\nUsername: admin"
        append Httpd(Frm_adminCreate) "\nPassword: admin\n"
        Stderr $Httpd(Frm_adminCreate)
     
        unset adminCreate
    }
    return
}

#-------------------------------------------------------------
#
# Initializes the rights-of-use view
#

proc db::InitRights {db view} {
   mk::view layout $db.$view {login:S fechaComienzo:L duracion:I fechaFinal:L afterId:S}
}

#-------------------------------------------------------------
# Check that the row is not being edited
# by another session. If not, locks the row in the current
# session and defines the variable in the local interpreter.
# They are not locked in the database, only in the sesion array


proc db::RowLock {cursession db view rowId} {
    
    upvar #0 Session:$cursession state
    
    foreach id [info globals Session:*] {
      upvar #0 $id othersession
      if {[string match $id Session:$cursession]} {
        continue
      }
      if {[info exist othersession($db.$view!$rowId)]} {          
        return 1 
      }
    }
    set state($db.$view!$rowId) 1
    return 0
}

#---------------------------------------------------------------
# Locks the whole Metakit view if possible
#

proc db::ViewLock {cursession db view} {
   upvar #0 Session:$cursession state
    
    foreach id [info globals Session:*] {
      upvar #0 $id othersession
      if {[string match $id Session:$cursession]} {
        continue
      }
      if {[info exist othersession($db.$view)]} {          
        return 1 
      }
    }
    set state($db.$view) 1
    return 0
}

#---------------------------------------------------------------
proc db::LockRowByKey {session db view key value} {

    upvar #0 Session:$session state
    set interp $state(interp)
   
    set position [mk::select $db.$view -exact $key $value]
    if {[string match "" $position]} {
        return 0
    }
   
    if { ! [db::RowLock $session $db $view $position] } {
      interp eval $interp [list set state($key) $value]
      set locked 0
    } else {
      set locked 1
    }    
    return $locked
}


#-------------------------------------------------------------
# Fill a session with variables read from a table of
# the metakit database. Returns al the fileds read except
# the key field
#

proc db::SearchBy {db view key value fields} {
    
    set rowId   [mk::select $db.$view $key $value]
    if {![llength $rowId]} {
       return {}
    }
    
    set values [eval mk::get $db.$view!$rowId $fields]
    set i 0
    foreach name $fields {
       set value [lindex $values $i]
       lappend result $name $value
       incr i
    } 
    return $result
}

#-------------------------------------------------------------
# Selects a Metaki database tag for further processing
#
proc db::SetTag {session dbase} {
    if {[string length $session] } {
      upvar #0 Session:$session state
      set state(dbase) $dbase
    } else {
      global Form
      	  set Form(dbase) $dbase
    }
    return
}

#-------------------------------------------------------------
# Selects a view for further processing
#
proc db::SetView {session view} {
    if {[string length $session] } {
      upvar #0 Session:$session state
      set state(view) $view
    } else {
      global Form
 	  set Form(view)  $view
    }
    return
}

#-------------------------------------------------------------
# Selects the natural key within the view
#
proc db::SetKeyName {session key} {
    if {[string length $session]} {
      upvar #0 Session:$session state
      set state(key) $key
    } else {
      global Form
 	  set Form(key) $key
    }
    return
}


#-------------------------------------------------------------
# Get a dbase tag
proc db::GetTag {session} {
    if {[string length $session]} {
      upvar #0 Session:$session state
      return $state(dbase)
    } else {
	global Form
	return $Form(dbase)
    }
}

#-------------------------------------------------------------
# Get a view
proc db::GetView {session} {
    if {[string length $session]} {
      upvar #0 Session:$session state
      return $state(view)
    } else {
	global Form
	return $Form(view)
    }
}

#-------------------------------------------------------------
# Get the view's natural key name
#
proc db::GetKeyName {session} {
    if {[string length $session] } {
      upvar #0 Session:$session state
      return $state(key)
    } else {
      global Form
 	  return $Form(key)
    }
}



#-------------------------------------------------------------
# Start the ball rolling

db::Init