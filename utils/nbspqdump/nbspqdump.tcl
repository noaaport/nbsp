#!%TCLSH%
#
# $Id$
#
# Usage: nbspqdump <dbfile|dbdump_output>
#
# An input file ending with ".db" is assumed to be a dbfile. Otherwise it is
# assumed to be the output of "db_dump", created by executing
#
#	db_dump -f <dump_output> q0.db	(or q1.db)
#
# Read until the line that starts with "re_len=" is found. Then read
# up to the line with "HEADER=END" to get to the start of the records,
# and then unpack each record until the line with "DATA=END" is found.
# The records are unpacked by calling nbspqdc, which essentially uses
# the unpack() functions in the nbsp sources src/packfpu.{h,c}.
#
# If it is a dbfile, then db_dump is called first, and then the output
# processed as above.

# The directory of the db programs
set db_bindir "%DB_BINDIR%";
if {[file isdirectory $db_bindir]} {
    append env(PATH) ":" $db_bindir;
}

# The name of the db5 dump program
set db_dump_bin "%DB_DUMP_BIN%";

proc errx {s} {
    puts $s;
    exit 1;
}

#
# main
#
if {$argc == 0} {
    errx "No file argument.";
}
set dbfile [lindex $argv 0];

set status [catch {
    if {[file extension $dbfile] eq ".db"} {
	#
	# If it is a path name, assume that the dbenv is the parent directory
	# and pass that as the [-h] option to db_dump.
	#
	set dbhome [file dirname $dbfile];
	if {$dbhome != "."} {
	    set f [open "|$db_dump_bin -h $dbhome $dbfile" r];
	} else {
	    set f [open "|$db_dump_bin $dbfile" r];
	}
    } else {
	set f [open $dbfile r];
    }
} errmsg];

if {$status != 0} {
    errx "Error opening $dbfile: $errmsg";
}

while {[gets $f line] > 0} {
    if {[regexp {^re_len} $line]} {
	set parts [split $line "="];
	set size [lindex $parts 1];
    } elseif {[regexp {^HEADER=END} $line]} {
	break;
    }
}

# Assume that nbspqdc is in PATH (as it should be).
set status [catch {
    set F [open "|nbspqdc -s $size" w];
} errmsg];
if {$status != 0} {
    close $f;
    errx "Error opening nbspqdc: $errmsg";
}

while {[gets $f line] > 0} {
    if {[regexp {^DATA=END} $line]} {
	break;
    }

    puts $F [string trimleft [string trimright $line]];
}

close $f;
close $F;
