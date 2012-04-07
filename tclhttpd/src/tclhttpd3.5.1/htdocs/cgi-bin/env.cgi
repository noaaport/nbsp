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

#close [open /tmp/iwashere w]

puts "Content-Type: text/html"
puts ""
puts "<title>The environment</title>"
puts "<H1>The environment</h1>"
puts <table>
foreach name [lsort [array names env]] {
	puts "<tr><td>$name</td><td>$env($name)</td></tr>"
}
puts </table>
exit 0
