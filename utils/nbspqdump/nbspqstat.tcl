#!%TCLSH%
#
# $Id$
#
# Usage: nbspqstat [dbhome]
#
# Here "dbhome" is the directory containing the nbsp db queues. It is
#
#	/var/noaaport/nbsp/db
#
# by default, and that is what is used unless it is specified in the comand
# argument. Ths program simply calls db_stat.

# The directory of the db programs
set db_bindir "%DB_BINDIR%";
if {[file isdirectory $db_bindir]} {
    append env(PATH) ":" $db_bindir;
}

# The name of the db5 stat program
set db_stat_bin "%DB_STAT_BIN%";

# The default directory
set dbhome "/var/noaaport/nbsp/db";

proc errx {s} {
    puts $s;
    exit 1;
}

#
# main
#
if {$argc > 1} {
    errx "Too many arguments.";
} elseif {$argc == 1} {
    set dbhome [lindex $argv 0];
}

set status [catch {
    puts [exec $db_stat_bin -h $dbhome -m];
} errmsg];

if {$status != 0} {
    errx $errmsg;
}
