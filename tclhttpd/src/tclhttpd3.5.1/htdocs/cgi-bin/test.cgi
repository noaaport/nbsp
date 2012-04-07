#!/bin/sh
# \
if [ -e /usr/bin/tclsh ]; then exec /usr/bin/tclsh "$0" ${1+"$@"} ; fi
# \
if [ -e /usr/local/bin/tclsh8.4 ]; then exec /usr/local/bin/tclsh8.4 "$0" ${1+"$@"} ; fi
# \
if [ -e /usr/local/bin/tclsh8.3 ]; then exec /usr/local/bin/tclsh8.3 "$0" ${1+"$@"} ; fi
# \
if [ -e /usr/bin/tclsh8.4 ]; then exec /usr/bin/tclsh8.4 "$0" ${1+"$@"} ; fi
# \
exec tclsh "$0" ${1+"$@"}

if {[catch {
    package require ncgi
    package require html

    ncgi::header
    html::head Hello
    flush stdout

    set query [ncgi::nvlist]

    puts [html::h1 "Hello, World!"]
    puts [html::h2 [clock format [clock seconds]]]

    puts [html::h3 "CGI Values"]
    puts [html::tableFromList $query]

    puts [html::h3 Environment]
    puts [html::tableFromArray env]
    flush stdout
    exit 0
}]} {
    puts "Content-Type: text/html\n"
    puts "<h1>CGI Error</h1>"
    puts "<pre>$errorInfo</pre>"
    puts "<h2>auto_path</h2>"
    puts "<pre>[join $auto_path \n]</pre>"
}
