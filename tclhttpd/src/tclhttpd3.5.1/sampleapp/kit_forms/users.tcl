# DESCRIPTION
#
# User management functionality
# A "vertical" module that mixes
# form handlers, and other related procedures
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

#-------------------------------------------------------------
proc UserId {} { 
  global Httpd
   
  set sock $Httpd(currentSocket)
  upvar #0 Httpd$sock data 
 
  if { [info exists data(remote_user)] } {
    set result $data(remote_user)
  } else {
  	set result ""
  }  
  return $result
}

#-------------------------------------------------------------
#
# Processing of alta.frm
#
#-------------------------------------------------------------

proc NewUserFormHandler {formid session action} {

	upvar #0 Session:$session state 
	global Form
	
	set result FORM_SUBST
	if {[string match $action [mc Accept]]} { 		
		
		set login [interp eval $state(interp) {set state(login)}]
		if { [string length $login] && [DoNewUser $session] } {
		  	set Form(errorInfo) [format [mc "The login <EM>%s</EM> already exists"] $login]
			set result FORM_ERROR
		} 
	
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------

proc DoNewUser {session} {
	
	set nusers [Form_DBSearchFields $session login]
	if {!$nusers} {
		set error 0
	} else {
		set error 1
	}
	return $error
}

#-------------------------------------------------------------
#
# Processing of alta2.frm
#
#-------------------------------------------------------------

proc VerifyNewUserFormHandler {formid session action} {
	upvar #0 Session:$session state 
	global Form
	
	set result FORM_SUBST
	if {[string match $action [mc Accept]]} { 		
		set result [DoVerifyNewUserData $session]
			
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------

proc DoVerifyNewUserData {session} {

	global Form
    upvar #0 Session:$session state
    set interp $state(interp)
    set result FORM_SUBST
    
    set password [interp eval $state(interp) {set state(password)}]
    set len [string length $password]
	if { $len > 0 && $len < 6 } {
		set Form(errorInfo) [mc "The new password has less than 6 characters"]
		set result FORM_ERROR
	} 
    
    set email [interp eval $state(interp) {set state(email)}]
    if { [string length $email] && ![regexp "\[^@]@\[^@]" $email]} {
    		set Form(errorInfo) [format [mc "The e-mail address <EM>%s</EM> has incorrect format"] $email]
		set result FORM_ERROR
	}	
	return $result
}

#-------------------------------------------------------------
#
# Processing of alta3.frm
#
#-------------------------------------------------------------

proc InsertUserFormHandler {formid session action} {
	
	upvar #0 Session:$session state 
	global Form
		
	set result FORM_SUBST
	if {[string match $action [mc Save]]} { 		
		
		DoInsertUser $session
		set result [KillSessionAndRedirect $session $action]
	
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------
# Insert new Users data to the Metakit database

proc DoInsertUser {session} {
    upvar #0 Session:$session state
    set interp $state(interp)
    
   set db    [db::GetTag $session]
   set view  [db::GetView $session]
   	
    
    # Get the names from the database definition
    # (get rid of :I or :B, etc)
    # and the values form the slave interpreter
    # Also encrypts password and handles the special case
    # of adminflag as a checkbox
    
    set dbfields [mk::view info $db.$view]
    foreach name $dbfields {
      regsub (:\[SILBDF\]) $name "" name      
      
       switch -exact -- $name {
         password {
           if {[interp eval $interp [list info exists state($name)]]}  {
             set value [interp eval $interp [list set state($name)]]
             set value [string trim $value]
             set value [tclcrypt $value [config::cget CryptSeed]] 
             lappend field_values $name $value               
           }    
         }
         
         adminFlag {
           if {[interp eval $interp [list info exists state($name)]]}  {
             set value [interp eval $interp [list set state($name)]]
            } else {
             set value 0
            }
            lappend field_values $name $value
         }
         
         default {
           set value [interp eval $interp [list set state($name)]]
           lappend field_values $name $value
         }
      }
    } 
 
    # Append to database   
   
    eval mk::row append $db.$view $field_values    
    mk::file commit $db
    return 0
}

#-------------------------------------------------------------
#
# Processing of modif.frm and baja.frm
#
#-------------------------------------------------------------

proc ExistingUserFormHandler {formid session action} {

	upvar #0 Session:$session state 
	global Form
	
	set result FORM_SUBST
	if {[string match $action [mc Accept]]} { 		
		
		set login [interp eval $state(interp) {set state(login)}]
		if { [string length $login] && [DoExistingUser $session] } {
		  	set Form(errorInfo) [format [mc "The login <EM>%s</EM> does not exists"] $login]
			set result FORM_ERROR
		} 
	
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------

proc DoExistingUser {session} {
	
	set nusers [Form_DBSearchFields $session login]
	if {$nusers} {
	  	set error 0
	} else {
		set error 1
	}
	return $error
}


#-------------------------------------------------------------
#
# Processing of modif3.frm
#
#-------------------------------------------------------------

proc ModifyUserFormHandler {formid session action} {
	
	upvar #0 Session:$session state 
	global Form
		
	set result FORM_SUBST
	if {[string match $action [mc Save]]} { 
			
		if { [DoModifyUser $session] } {		 
		  set result FORM_ERROR
		} else {
		  set result [KillSessionAndRedirect $session $action]
		}
	
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------
# Replace new Users data to the Metakit database

proc DoModifyUser {session} {

    upvar #0 Session:$session state
    global Form
    
    set interp $state(interp)
    
    set db     [db::GetTag $session]
    set view   [db::GetView $session]
    set key    [db::GetKeyName $session]
    set modified  [interp eval $interp {set modify}]
    
    foreach name $modified {      
      if {[string match "adminFlag" $name]} {
        if {[interp eval $interp [list info exists state($name)]]}  {
          set value [interp eval $interp [list set state($name)]]
        } else {
          set value 0
        }
      } else {
         set value [interp eval $interp [list set state($name)]]        
      }
      lappend field_values $name $value
    } 
 
    set value [interp eval $interp [list set state($key)]]
    
    if { [db::LockRowByKey $session $db $view $key $value] } {
      set Form(errorInfo) [mc "Another administrator has locked the current record"]
      return 1
    }
    
    # replace the database
    
    set position [mk::select $db.$view -exact $key $value]
    eval mk::set $db.$view!$position $field_values
    mk::file commit $db
    return 0
}

#-------------------------------------------------------------
#
# Processing of baja2.frm
#
#-------------------------------------------------------------

proc DeleteUserFormHandler {formid session action} {
	
	upvar #0 Session:$session state 
	global Form
		
	set result FORM_SUBST
	if {[string match $action [mc Delete]]} { 		
		
		if { [DoDeleteUser $session] } {
		  set result FORM_ERROR
		} else {
		  set result [KillSessionAndRedirect $session $action]
		}		 
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------

proc DoDeleteUser {session} {

    global Form
    upvar #0 Session:$session state
    set interp $state(interp)
    
   set db    [db::GetTag $session]
   set view  [db::GetView $session]
   set key   [db::GetKeyName $session]
   set value [interp eval $interp [list set state($key)]]
    
    if { [string match $value admin]} {
    	set Form(errorInfo) [mc "Cannot delete the special user <EM>admin<EM>"]
     	return 1
    }
    
    if { [db::LockRowByKey $session $db $view $key $value] } {
    	set Form(errorInfo) [mc "Another administrator has locked the current record"]
      	return 1
    }
    
    set position [mk::select $db.$view -exact $key $value]
    if {[string match "" $position]} {
    	set Form(errorInfo) [format [mc "Cannot find a key <EM>%s</EM> with value <EM>%s</EM>"] $key $value]
        return 1  
    }
       
   mk::row delete $db.$view!$position 
   mk::file commit $db
   return 0
}

#-------------------------------------------------------------
#
# Processing of passwd.frm
#
#-------------------------------------------------------------

proc CheckPasswdFormHandler {formid session action} {

	upvar #0 Session:$session state 
	global Form
		
	set result FORM_SUBST
	if {[string match $action [mc Accept]]} { 		
		
		set password [interp eval $state(interp) {set state(password)}]
		if { [string length $password] && [DoCheckPassword $session] } {
				set result FORM_ERROR
		} 
	
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------

proc DoCheckPassword {session} {

	global Form
 	upvar #0 Session:$session state
 
    set interp $state(interp)
    set db     [db::GetTag $session]
    set view   [db::GetView $session]
    set key    [db::GetKeyName $session]
    set value  [interp eval $interp [list set state($key)]]
    
    set position [mk::select $db.$view -exact $key $value]
    if {[string match "" $position]} {
        set Form(errorInfo) [mc "Sorry, invalid authentication"]
        return 1
    }
    set dbvalue  [eval mk::get $db.$view!$position password]
    
    set password [interp eval $interp [list set state(password) ]]
    set passcript [tclcrypt $password [config::cget CryptSeed]]
    if {![string match $passcript $dbvalue]} {
      set Form(errorInfo) [mc "Sorry, invalid authentication"]
      return 1
    }   
    return 0
}

#-------------------------------------------------------------
#
# Processing of passwd2.frm
#
#-------------------------------------------------------------

proc VerifyPasswdFormHandler {formid session action} {

	upvar #0 Session:$session state 
	global Form
		
	set result FORM_SUBST
	if {[string match $action [mc Accept]]} { 		
		
		set newpass [interp eval $state(interp) {set state(newpass)}]
		if { [string length $newpass]} {
		  if {[DoVerifyPassword $session] } {
			set result FORM_ERROR
		  } else {
		  	set result [KillSessionAndRedirect $session $action]
		  }
		}
	
	} elseif {[string match $action [mc Cancel]]} {
		set result [KillSessionAndRedirect $session $action]
	}
	return $result
}

#-------------------------------------------------------------

proc DoVerifyPassword {session} {

 	upvar #0 Session:$session state
 	global Form
 	
    set interp $state(interp)
   
    set newpass [interp eval $interp [list set state(newpass) ]]
    set vfypass [interp eval $interp [list set state(vfypass) ]]
    if {![string match $newpass $vfypass]} {
      set Form(errorInfo) [mc "The verify password field and the new password field are not the same"]
      return 1
    }
    
    if {[string length $newpass]<6} {
    	set Form(errorInfo) [mc "The new password has less than 6 characters"]
     	return 1
    }
    
    # Modify the user's password
    
    set passcrypt [tclcrypt $newpass [config::cget CryptSeed]]
    interp eval $interp [list set modify password]
    interp eval $interp [list set state(password) $passcrypt]    
    return [DoModifyUser $session]
}

#-------------------------------------------------------------
