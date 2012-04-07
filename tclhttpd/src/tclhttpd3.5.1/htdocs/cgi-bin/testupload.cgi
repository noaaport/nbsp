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
exec tclsh "$0"
#
puts "Content-Type: text/html"
puts ""

puts "<h2>Environment</h2>"
puts "<table>"
foreach {n v} [array get env] {
    puts "<tr><td>$n</td><td>$v</td></tr>"
}
puts "</table>"

puts <pre>
fconfigure stdin -translation binary
puts [read stdin $env(CONTENT_LENGTH)]
puts </pre>
