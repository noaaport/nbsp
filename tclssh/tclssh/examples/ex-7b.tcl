#!/usr/bin/tclsh

source ../src/ssh.tcl

proc exec_host {host script} {

    ::ssh::connect $host;
    ::ssh::push $host $script;
    ::ssh::send $host;
    ::ssh::hfileevent $host readable "fileevent_handler $host";
}

proc fileevent_handler {host} {
    
    set r [::ssh::pop_line $host line];
    if {$r >= 0} {
	puts $line;
    } else {
	::ssh::disconnect $host;
	incr ::gcounter -1;
    }
}

set script {
    set options [list -n -s -r];
    after 10000
    puts [exec uname -m {*}$options]
    exit 0
};

set gcounter 0;    
foreach h [list dl585a dl585b] {
    exec_host $h $script;
    incr gcounter;
}

while {$gcounter != 0} {
    vwait gcounter;
}
