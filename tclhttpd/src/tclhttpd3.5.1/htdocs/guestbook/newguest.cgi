#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

set dir [file dirname [info script]]
lappend auto_path $dir [file join $dir ../cgi-bin]

set datafile [file join \
	[file dirname [info script]] guestbook.data]

Cgi_Parse

# Open the datafile in append mode

if [catch {open $datafile a} out] {
	Cgi_Header "Guestbook Registration Error" \
		{BGCOLOR=black TEXT=red}
	P
		puts "Cannot open the data file"
	P
	puts $out	;# the error message
	exit 0
}

# Append a Tcl set command that defines the guest's entry

puts $out ""
puts $out [list set Guestbook($cgi(name)) \
	[list $cgi(url) $cgi(html)]]
close $out

# Return a page to the browser

Cgi_Header "Guestbook Registration Confirmed" \
	{BGCOLOR=white TEXT=black}

puts "
<DL>
<DT>Name
<DD>$cgi(name)
<DT>URL
<DD>[Link $cgi(url) $cgi(url)]
</DL>
$cgi(html)
"

Cgi_Tail


