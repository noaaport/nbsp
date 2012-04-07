#
# Support for the on-line surveys at the sunscript site.

package provide survey 1.0

#Direct_Url /surveyd Survey

# This is hit when the user asks for a particular file.
# This picks a survey to display.

# These procs accessed from the develop*.html files
proc DataChoice {p1 p2 p3 p4} {
    global choice
    set n 1
    regexp {[0-9]} $choice n
    upvar 0 p$n price
    return $price
}
proc FormAction {} {
    return /surveyd/showfiles
}

proc Survey/download {file} {
    global Doc choice

    set dir [file join $Doc(root) survey]
    
    # choose the least recently accessed file to return
    
    set files {develop1.html develop2.html develop3.html develop4.html 
		support1.html}
    
    set atime [expr [clock seconds] + 100]
    foreach f $files {
	set f [file join $dir $f]
	set a [file atime $f]
	if {$a < $atime} {
	    set atime $a
	    set choice $f
	}
    }
    
    
    set in [open $choice]
    set html [subst [read $in]]
    close $in

    return $html
}

proc Survey/showfiles {file sendto subject email args} {
    global Doc
    set message [list Data file $file]\n[list Data email $email]\n
    foreach {name value} $args {
	append message [list Data $name $value]\n
    }
    MailInner $sendto $subject $email text/plain $message

    if {[string match *8* $file]} {
	set page 8.0.direct.html
    } else {
	set page 7.6.direct.html
    }
    set in [open $Doc(root)/TclTkCore/$page]
    set html [read $in]
    close $in
    return $html
}
