# These packages are for the SNMP demo application
# "snmp" is a poorly-named module that generates HTML forms to view
#	MIB info
# "Tnm" is the SNMP interface from the Scotty extension

if {![catch {package require Tnm}]} {
    package require httpd::snmp
    package require httpd::telnet
    Stderr "SNMP Enabled"
}

