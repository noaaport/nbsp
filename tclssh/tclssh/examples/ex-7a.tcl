#!%TCLSH%

source ../src/ssh.tcl

set done 0;

proc handle_stdout {} {

    after 1000;
    puts "Waiting for diablo";
}

proc fileevent_handler {} {

    global done;

    ::ssh::pop_line diablo line;
    puts $line;

    set done 1;
}
    
::ssh::connect -t %TCLSHT% -- diablo

::ssh::push diablo {after 5000; puts [exec uname -m]};
::ssh::send diablo;
::ssh::hfileevent diablo readable fileevent_handler;

fileevent stdout writable handle_stdout;

vwait done;

::ssh::disconnect diablo;
