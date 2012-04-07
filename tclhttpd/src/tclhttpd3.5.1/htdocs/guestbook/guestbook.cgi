#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

# Implement a simple guestbook page.
# The set of visitors is kept in a simple database.
# The newguest.cgi script will update the database.

# Load cgilib.tcl from the adacent cgi-bin directory

set dir [file dirname [info script]]
lappend auto_path $dir [file join $dir ../cgi-bin]

Cgi_Header "Brent's Guestbook" {BGCOLOR=white TEXT=black}
P
set datafile [file join \
	[file dirname [info script]] guestbook.data]
if {![file exists $datafile]} {
	puts "No registered guests, yet."
	P 
	puts "Be the first "
	Link {registered guest!} newguest.html
} else {
	puts "The following folks have registered in my GuestBook."
	P 
	Link Register newguest.html
	H2 Guests
#	catch {source $datafile}
	source $datafile
	foreach name [lsort [array names Guestbook]] {
		set item $Guestbook($name)
		set homepage [lindex $item 0]
		set markup [lindex $item 1]
                if {[string length $homepage]} {
                  H3 [Link $name $homepage]
                } else {
                  H3 $name
                }
		puts $markup
	}
}
Cgi_Tail


