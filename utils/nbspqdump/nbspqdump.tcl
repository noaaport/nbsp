#!%TCLSH%
#
# $Id$
#
# Usage: nbspqdump <dump_output>
#
# Here <dump_output> is a file created by executing
#
#	db_dump -f <dump_output> q0.db	(or q1.db)
#
# Read until the line that starts with "re_len=" is found. Then read
# up to the line with "HEADER=END" to get to the start of the records,
# and then unpack each record until the line with "DATA=END" is found.
# The records are unpacked by calling nbspqdc, which essentially uses
# the unpack() functions in the nbsp sources src/packfpu.{h,c}.

# The directory of the db programs
set db_bindir "%DB_BINDIR%";
if {[file isdirectory $db_bindir]} {
    append env(PATH) ":" $db_bindir;
}

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

# If it is a path name, assume that the dbenv is the parent directory
# and pass that as the [-h] option to db_dump.
set dbhome [file dirname $dbfile];

set status [catch {
    if {$dbhome != "."} {
	set f [open "|db_dump -h $dbhome $dbfile" r];
    } else {
	set f [open "|db_dump $dbfile" r];
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
