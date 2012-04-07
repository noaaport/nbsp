#!/bin/sh
# htpass.tcl
# Crude interface to generate encrypted passwords for htaccess files
#
# \
exec wish8.0 "$0" ${1+"$@"}

lappend auto_path [file join [file dirname [info script]] ../lib]
package require auth
if {[catch {package require crypt}]} {
    package require tclcrypt
}

set filename /usr/local/htaccess/passwd

if {[file exists $filename]} {
    set in [open $filename]
    while {[gets $in line] >= 0} {
	if [regexp {^([^:]+):[      ]*(.+)} $line x key value] {
	    set info(user,$key) $value
	}
    }
    close $in
}

set msg "Set Web Password"
message .msg -textvar msg
grid .msg - -sticky news

set i 1
foreach {label var} {
    "User Name" user
    "Old Password" oldpass
    "New Password" pass1
    "Password (again)" pass2
	} {
    label .l$i -text $label
    entry .e$i -textvar $var
    grid .l$i .e$i -sticky news
    if {$var != "user"} {
	.e$i config -show *
    }
    incr i
}

button .cancel -text "Exit" -command exit
button .doit -text "Set Password" -command Doit
grid .doit .cancel

proc Doit {} {
    global info user oldpass pass1 pass2 msg
    if {[info exist info(user,$user)]} {
	set salt [string range $info(user,$user) 0 1]
	set check [crypt $oldpass $salt]
	if {[string compare $check $info(user,$user)]} {
	    set msg "Old password incorrect"
	    set oldpass ""
	    return
	}
    }
    if {[string compare $pass1 $pass2] != 0} {
	set msg "New passwords are not equal"
	set pass2 ""
	set pass1 ""
	return
    }
    set salt ""
    append salt [format %c [expr 65+int(rand()*26)]]
    append salt [format %c [expr 97+int(rand()*26)]]
    set newpass [crypt $pass1 $salt]

    global filename
    set done 0
    set out [open $filename.new w]
    if {[file exists $filename]} {
	set in [open $filename]
	while {[gets $in line] >= 0} {
	    if [regexp {^([^:]+):[      ]*(.+)} $line x key value] {
		if {[string compare $key $user] == 0} {
		    puts $out "$user: $newpass"
		    set done 1
		} else {
		    puts $out $line
		}
	    }
	}
	close $in
	file rename -force $filename $filename.old
    }
    if {!$done} {
	puts $out "$user: $newpass"
    }
    close $out
    file rename -force $filename.new $filename
    set msg "Done"
}
