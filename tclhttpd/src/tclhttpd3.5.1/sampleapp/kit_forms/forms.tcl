
# DESCRIPTION
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

package provide forms 1.0

package require http
package require html
package require ncgi
package require mime
package require msgcat

namespace import msgcat::*
mcload [file join [file dirname [info script]] msgs]

# Register Document type handler.

Mtype_Add .frm application/x-tcl-frm


# Set the time in seconds that a session will last for.

array set Form [list age [config::cget FormSessionTimeout]]



# Doc_application/x-tcl-frm
#
#	Document type handler for .frm pages
#	Used to process self-posting-forms
#	in short-lived sessions
#
#	This is the main entry point
#
# Arguments:
#	
#	sock - the open socket for the response
#	error - the error message 
#	environ - additional environment array 
#
# Results:
#	HTML to the client.

proc Doc_application/x-tcl-frm {path suffix sock} {
    upvar #0 Httpd$sock data
    global Form

    append data(query) ""
    set queryList [Url_DecodeQuery $data(query)]
    
   
    StuffFromTML $path $suffix $sock
    
    
    # Destroy any old session that are laying around. In this instance
    # 5 minutes is the setting.

    Session_Reap $Form(age) Form

    # Find the current session (or start a new one if session=new).

    set session [Session_Match $queryList Form error]

    if {$session == {}} {
        Form_Error $sock [mc "Session does not exists"]
        return
    }


    # Sets the environment for the form template processing
    # data(uri) => with query data.
    # data(url) => without query data
   
    # stores the decoded query list once for all in the session array
    upvar #0 Session:$session state
    set state(query)   $queryList
    puts $queryList
      
    # Process the query data.
 
    if [catch {FormProcess $sock $session $queryList} result] {
        Httpd_ReturnData $sock text/html $result
        return
    }

    # Expand the page in the correct session interpreter, or treat
    # the page as ordinary html if the session has ended.


    switch -exact -- $result {
        FORM_FILE  { Httpd_ReturnFile $sock text/html $path }
        FORM_SUBST { Doc_Subst $sock $path interp$session }
        FORM_REDIR { Httpd_RedirectSelf $Form(redirect) $sock}
        FORM_ERROR { Session_Destroy $session ; Form_Error $sock $Form(errorInfo) }       
    }
    return
}



# StuffFromTML
#
#	This procedure encapsulate all the TML-like
#	processing that was copied from the doc.tcl
#	module

proc StuffFromTML {path suffix sock} {
	upvar #0 Httpd$sock data
	
	# Initialize the Standard Tcl Library ncgi package so its
    	# ncgi::value can be used to get the data.
    
    	if {[info exist data(mime,content-type)] &&
	    ("$data(proto)" != "GET")} {
		set type $data(mime,content-type)
    	} else {
		set type application/x-www-urlencoded
    	}
	ncgi::reset $data(query) $type
    	ncgi::parse
    	ncgi::urlStub $data(uri)
    	
    	# 
    	# Source the .tml files from the root downward.
    	#
    
    	foreach libfile [DocGetTemplates $sock $path] {
		if {[file exists $libfile]} {
	    		interp eval {} [list uplevel #0 [list source $libfile]]
        	}
   	}
   	return	
}


#-------------------------------------------------------------
# The purpose of this procedure is to process the form query data.
# Parameters:
#   socket:  the socket being used.
#   session:  the session id
#   query:    a list of names and values produced by Url_DecodeQuery

proc FormProcess {sock session query} {
	
	global Form
	upvar #0 Httpd$sock data
	upvar #0 Session:$session state
    	
	set interp $state(interp)
	set formFlag 0
	
	foreach {name value} $query {
        
        	switch -exact --  $name {
        		"action" {
        			set invoke(action) $value
        			continue
        		}
        		
        		"formid" {
        			set formFlag 1
        			set invoke(formid) $value
        			interp eval $interp [list set state($name) $value]
        			continue
        		}
        		
        		default {
        			# Define variables in the slave interpreter so they are there before	
					# we do a Doc_Subst on the page!
 	
 
         			interp eval $interp [list set state($name) $value]
        		}
        	}		
       
	}
	if {$formFlag} {
		
		if {![info exists invoke(action)]} {
			set Form(errorInfo) [format [mc "No action was passed on form %s"] $invoke(formid)]
			return FORM_ERROR 
		}
		
		return [FormInvokeHandler $invoke(formid) $session $invoke(action)]
	}
    	return FORM_SUBST
}

#
#
#
#

proc Form_RegisterHandler {formid handler} {

	global Form
	set Form($formid,handler) $handler
	return
}

#
#
#
#

proc FormInvokeHandler {formid session action} {

	upvar #0 Session:$session state
	global Form
	
	if {![info exists Form($formid,handler)]} {
	  	set Form(errorInfo) [format [mc "No Form handler was specified for %s"] $formid]
	  	return FORM_ERROR
	}
	
	if {![llength $Form($formid,handler)]} {
		set Form(errorInfo) [format [mc "Empty Form handler was specified for %s"] $formid]
		return FORM_ERROR
	}
	
	puts "FormInvokeHandler =>$Form($formid,handler) $formid $session [list $action]"
	return [eval $Form($formid,handler) $formid $session [list $action]]
}


#######################################################
#######################################################
#######################################################
#
# Helper procedures available in slave interpreters
#
#######################################################
#######################################################
#######################################################

# Form_Error
#
#	Generate an error page with the proper layout as the rest of pages
#
# Arguments:
#	
#	sock - the open socket for the response
#	error - the error message 
#
# Results:
#	HTML with the error page.

proc Form_Error {sock errorInfo} {
    
	package require mypage
	
	append html [html::author "Rafael Gonzalez Fuentetaja"]
	append html [html::meta generator {tclhttpd templates}]
	append html [mypage::header ""]
	append html "<H1>Error</H1> <P>$errorInfo</P>"
	append html [mypage::footer]
	Httpd_ReturnData $sock text/html $html
}

#-------------------------------------------------------------
#               ACTION BUTTONS FORM SECTION
#-------------------------------------------------------------

# Form_ActionButtons
#
#	Confirms or cancels previous form data of the self-posting form
#	Displayed at the page following the self-posting form
#
# Arguments:
#	
#	session - the session identifier
#	formid  - the form id
#	fields - list of {label name URL} values to generate buttons
#	formhandler - a callback procedure to process actions
#
# Results:
#	HTML with the error page.

#-------------------------------------------------------------
# Form_ActionButtons
#
proc Form_ActionButtons {session formid fields {formhandler {}}} {

	global Httpd   
  	set sock $Httpd(currentSocket)
  	upvar #0 Httpd$sock data 
  	upvar #0 Session:$session state
  	
  
    	if {![string length $session]} {
      		return "<P>No sen pueden poner botones de formulario sin sesion</P>"
    	} 
    
    	Form_RegisterHandler $formid $formhandler
    	set self $data(url)?[eval http::formatQuery session $session]
    	
 	append html "<FORM name=\"$formid\" action=\"$self\" method=\"POST\"><TABLE><TR>"
 	append html "<INPUT class=\"button\" type=\"hidden\" name=\"formid\" value=\"$formid\">"
 	foreach {label url} $fields {
 	  set trueLabel [mc $label]
 	  set state(url,$trueLabel) $url
 	  append html "<TD><INPUT type=\"submit\" name=\"action\" value=\"$trueLabel\"></TD>"
 	}
	append html "</TR></TABLE></FORM>"		
	return $html	
}

#-------------------------------------------------------------
# Form_AcceptCancelButtons
# Simplified version of above with only two buttons named Accept and Cancel
#

proc Form_AcceptCancelButtons {session formid urlaccept urlcancel} {

	lappend fields [mc "Accept"] $urlaccept [mc "Cancel"] $urlcancel
	Form_ActionButtons $session $formid $fields FormAcceptCancelButtonsHandler
}

#-------------------------------------------------------------
# Handler for Form_AcceptCancelButtons
#

proc FormAcceptCancelButtonsHandler {formid session action} {
	
	set result FORM_SUBST
	if {[string match $action [mc Cancel]] || 
	    [string match $action [mc Accept]]} {
		upvar #0 Session:$session state 
		global Form
		
		set Form(redirect) $state(url,$action)
		set result FORM_REDIR
	
	}
	return $result
}

#-------------------------------------------------------------
# Form_AcceptButton
# Simplified version of above with only one button named Accept
#

proc Form_AcceptButton {session formid urlaccept} {

	lappend fields [mc "Accept"] $urlaccept
	Form_ActionButtons $session $formid $fields FormAcceptButtonHandler
}

#-------------------------------------------------------------
# Handler for Form_AcceptCancelButtons
#

proc FormAcceptButtonHandler {formid session action} {
	
	set result FORM_SUBST
	if {[string match $action [mc Accept]]} { 
		upvar #0 Session:$session state 
		global Form
		
		set Form(redirect) $state(url,$action)
		set result FORM_REDIR
	
	}
	return $result
}


#-------------------------------------------------------------
#               SELF POSTING FORM SECTION
#-------------------------------------------------------------

#-------------------------------------------------------------
#
# FormGenerateButtons
#
# 	Generate the action buttons part in an HTML self-posting-form
#
# Arguments:
#	
#	buffer - reference to the current buffer where the HTML form is being generated
#	bottons - lis of label,url pairs 
#
#

proc FormGenerateButtons {session buffer buttons} {
	upvar $buffer html
	upvar #0 Session:$session state
	
	append html "<TR><TD align=\"center\" colspan=\"3\">"
	foreach {label url} $buttons {
	   set trueLabel [mc $label]
 	   set state(url,$trueLabel) $url
	   append html "<INPUT class=\"button\" type=\"submit\" name=\"action\" value=\"$trueLabel\">"
	}
        append html "<INPUT class=\"button\" type=\"reset\" value=\"[mc Reset]\"></TD></TD></TR>"                  
	return
}

#-------------------------------------------------------------
#
# Form_Form
#
#	Generate a based self posting form using text, password, checkbox and textarea input elements.	
#	Check for missing required fields.
#
# Arguments:
#	
#	session - the session number
#	title - the form title
#	fields - list  of {required type name label} values to generate
#	buttons - list of {label, url} buttons to go after processing
#	nextpage - relative-to-server URL to perform redirection on processing done.
#   	size - number of characters of input texts.
#	handler - form handler
#	
# Results:
#	HTML for the form.
#
# Caveats:
#   INPUT type=checkbox must always set the required flag = 0
#

proc Form_Form {session title fields buttons size handler} {
 
	global Httpd   
  	set sock $Httpd(currentSocket)
  	upvar #0 Httpd$sock data 
  	upvar #0 Session:$session state
  	
          
	set interp $state(interp)
	Form_RegisterHandler $title $handler	
	
	set self $data(url)?[eval http::formatQuery session $session]
		
    	if {[interp eval $interp {info exists state(formid)}]  && 
        	[string match $title [interp eval $interp {set state(formid)}]]} {      
       		 # Incoming form values, check them   
       		
       		 set check 1
   	} else {
       		 # First time through the page in this session
       		 set check 0
       		 		
	}

	set query [array get $state(query)]
		
     	append html "<FORM title=\"$title\" action=$self method=\"POST\">\n"
     	append html "<INPUT type=\"hidden\" name=\"formid\" value=\"$title\">"
     	append html {<TABLE class="form" >}
     	foreach {required type name label} $fields {
       		append html "<TR><TD class=\"missing\">"
       		if {$check && $required && [FormVarEmpty $name $interp]} {
         		lappend missing $label
         		append html "*"
       		}
       		append html "</TD><TD class=name>$label</TD>"       
      		switch -exact -- $type {
         		text     { append html "<TD class=\"value\"><INPUT class=\"text\" size=\"$size\" maxlength=\"$size\" [FormValue $name $state(query)]></TD>" }
        		password { append html "<TD class=\"value\"><INPUT class=\"text\" type=\"$type\" size=\"$size\" maxlength=\"$size\" [FormValue $name $state(query)]></TD>"}
         		textarea { append html "<TD class=\"value\"><TEXTAREA cols=\"$size\" rows=5 name=$name>[FormTextAreaValue $name $state(query)]</TEXTAREA></TD>"}
        		checkbox { append html "<TD class=\"value\"><INPUT type=\"$type\" [FormBoolValue $name $state(query)]></TD>"}       
       		}       
       		append html "</TR>\n"
     	}
	 
	FormGenerateButtons $session html $buttons
     	append html "</TABLE>\n</FORM>\n"
     
     	if {$check} {
       		if {[info exist missing]} {
       			
         		# Display the missing fields required
     
         		set msg {<FONT  class="missing">Por favor rellene los siguentes datos: }
         		append msg [join $missing ", "]
	     		append msg "</FONT >"
         		set html "<P>$msg\n$html</P>"
         
       		} else {
       
         		# No missing fields, so advance to the next page.
        		# and pass session in the URL
         		# Doc_RedirectSelf never returns

			set nextpage [lindex $buttons 1]
           		Doc_RedirectSelf $nextpage?[eval http::formatQuery session $session]          
       		}       
     	} 
    
	return $html
}



#-------------------------------------------------------------
#
# InputForm
#
# 	A typical form with two action buttons named Accept and Cancel
# 	The Accept action in not intercepted in the form handler,
# 	the Cancel action redirects to cancelpage in the form handler.
#	The Accept action will be to navigate to the next screen
#	The input fields get accumulated in sess array in the slave interpreter
#
# Arguments:
#	
#	session - the session number
#	title - the form title
#	fields - list of {required type name label} values to generate
#	nextpage - relative-to-server URL to perform redirection on success.
#	cancelpage - relative-to-server URL to perform redirection on cancel.
#   	size - number of characters of input texts
#	
# Results:
#	HTML for the form.
#
# Caveats:
#   INPUT type=checkbox must always set the required flag = 0
#


proc Form_InputForm {session title fields nextpage cancelpage handler {size 64}} { 
    
    global Httpd   
    set sock $Httpd(currentSocket)
    upvar #0 Httpd$sock data 
    upvar #0 Session:$session state
    
    lappend buttons Accept $nextpage Cancel $cancelpage
    Form_Form  $session $title $fields $buttons $size $handler	
}


#
# helper procedure to be used in the form handlers
# Kills the current session and redirects to a new URL

proc KillSessionAndRedirect {session action} {
 	upvar #0 Session:$session state
 	global Form
 	
 	set Form(redirect) $state(url,$action)	
	Session_Destroy $session
	return FORM_REDIR
}


#
# Only deals with the Cancel button.
#
proc DefaultInputFormHandler {formid session action} {
	
	set result FORM_SUBST
	if {[string match $action [mc Cancel]]} { 
		set result [KillSessionAndRedirect $session $action] 
	
	}
	return $result
}

##################################################
##################################################
##################################################


#-------------------------------------------------------------

proc Form_Author {author} {
    html::author $author
}

#-------------------------------------------------------------

proc Form_Meta {args} {
    html::meta $args
}

#-------------------------------------------------------------

proc Form_Header {title} {
    mypage::header $title
}

#-------------------------------------------------------------

proc Form_Footer {} {
    mypage::footer
}


# Form_ShowSubmitted
#
#	Generate a table with the results of the self posting form
#	for the user to verify before the final submision.
#	Gets the values from the variables set in the slave interpreter
#	created for the session.
#
# Arguments:
#	session - interpreter where to look for global variables set by the session-based document handler
#	
# Results:
#	HTML with the variables of query data 

proc Form_ShowSubmitted {session fields} {
    upvar #0 Session:$session state
    
    set interp $state(interp)	
	append html "<TABLE class=\"listing\">"
	
	foreach {name label} $fields {	    
	    if {[interp eval $interp [list info exists state($name)]]}  {
	       append html "<TR><TD class=\"name\">$label</TD>"
           set value [interp eval $interp [list set state($name)]]
	       append html "<TD class=\"value\">$value</TD></TR>"
	    }
	}
	append html "</TABLE>"
	return $html	
}

#################################################
## HELPER PROCEDURES FOR THE SELF POSTING FORM ##
#################################################

#
# Similar version as html::varEmpty but 
# operates within an inner interpreter
#
proc FormVarEmpty {name interp} {
    upvar $name var  
    
    if {[interp eval $interp [list info exists state($name)]]} {
    	set value [interp eval $interp [list set state($name)]]
    } else {
	   set value ""
    }
    return [expr {[string length [string trim $value]] == 0}]
}

#
# Similar to html::formValue but operates on a given list
# and not in the CGI environment data
#
proc FormValue {key query} {
  set i [lsearch -exact $query $key]
  if { $i != -1 } {
    incr i
    set value [lindex $query $i]
  } else {
    set value ""
  }
  return "name=\"$key\" value=\"[html::quoteFormValue $value]\""
}

#
# Variation for TextAreas
#
proc FormTextAreaValue {key query} {
  set i [lsearch -exact $query $key]
  if { $i != -1 } {
    incr i
    set value [lindex $query $i]
  } else {
    set value ""
  }
  return [html::quoteFormValue $value]
}

#
# Similar to html::formValue but operates on a given list
# and not in the CGI environment data
#
proc FormBoolValue {key query} {
  set i [lsearch -exact $query $key]
  if { $i != -1 } {
    incr i
    set value [lindex $query $i]
    if {$value} {
    	set result "name=\"$key\" value=\"1\" CHECKED"
    } else {
     	set result "name=\"$key\" value=\"1\""
    }
  } else { 
    set result "name=\"$key\" value=\"1\""
  }
  return $result
}


