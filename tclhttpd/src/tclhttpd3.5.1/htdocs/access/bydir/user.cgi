#!/bin/sh
# \
exec tclsh "$0"

# This script decodes the HTTP_AUTHORIZATION environment variable.

# When running as a cgi script, uncomment out the next line
#puts "Content-Type: text/html\n"

package require tcllib

# Here it is.  The authorization is a type followed by data:

if ![info exists env(HTTP_AUTHORIZATION)] {
    puts "(no user)"
    exit 0
}

set parts [split $env(HTTP_AUTHORIZATION)]
set type [lindex $parts 0]		;# Should be "Basic"
set code [lindex $parts 1]

# For basic authentication, the code is base64 encoded user:password

set parts [split [base64::decode $code] :]
set user [lindex $parts 0]
set pass [lindex $parts 1]

puts $user
exit 0
