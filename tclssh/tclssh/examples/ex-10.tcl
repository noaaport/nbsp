#!%TCLSH%

#
# Ideas for a higher level protocol
#
source ../src/ssh.tcl

proc get_slave_output {slave } {

    ::ssh::pop_line $slave output;

    set c [string trim $output];
    if {[regexp {^\d+$} $c] == 0} {
	# An error
	::ssh::disconnect $slave;
	return -code error "Error from $slave: $c";
    } else {
	::ssh::pop_read $slave $c output; 
    }

    return $output;
}

proc send_slave_init {slave} {

    set script {

	proc send_master_output {output} {
	    
	    global i;

	    set output "This is line $i: $output";
	    set c [string length $output];
	
	    puts $c;
	    puts -nonewline $output;
	    flush stdout;
	    
	    incr i;
	};

	set i 0;
    };

    ::ssh::push $slave $script;
    ::ssh::send $slave;
}

proc send_slave_script {slave line} {

    set script "send_master_output \"$line\"";
	
    ::ssh::push $slave $script;
    ::ssh::send $slave;    
}

set slave "diablo";
::ssh::connect -t %TCLSHT% -- $slave;

send_slave_init $slave;

send_slave_script $slave "One input line";
puts [get_slave_output $slave];

send_slave_script $slave "Another input line";
puts [get_slave_output $slave];

::ssh::disconnect $slave;
