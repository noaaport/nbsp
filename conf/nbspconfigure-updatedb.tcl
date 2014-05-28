#!%TCLSH%
#
# $Id$
#

# Configuration
set file "/etc/updatedb.conf";

# Override with command line argument if given
if {$argc > 1} {
    puts "Too many arguments.";
    return 1;    
} elseif {$argc == 1} {
    set file [lindex $argv 0];
}

proc main {file} {

    set newbody [list];
    set edited 0;

    set body [split [exec cat $file] "\n"];
    foreach line $body {
	if {[regexp {^\#} $line]} {
	    lappend newbody $line;
	    continue;
	}

	if {[regexp {^PRUNEPATHS="(.+)"} $line match val] == 0} {
	    lappend newbody $line;
	    continue;
	} else {
	    if {[string first "/var/noaaport" $val] == -1} {
		append val " /var/noaaport";
		lappend newbody "PRUNEPATHS=\"${val}\"";
		set edited 1;
	    } else {
		lappend newbody $line;
	    }
	}
    }

    if {$edited == 1} {
	file copy -force $file ${file}.bck
	exec echo [join $newbody "\n"] > ${file};
    } 
}

#
# main
#
set status [catch {
    main $file;
} errmsg];

if {$status != 0} {
    puts "Error processing $file.";
    puts $errmsg;
}

return $status;
