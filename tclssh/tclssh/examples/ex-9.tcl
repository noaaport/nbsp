#!%TCLSH%

source ../src/ssh.tcl

# This is a script that will be sent to the slave.

set slave "diablo";

::ssh::connect -t %TCLSHT% -- $slave;


set script {
    set output "This is the first line of output.";
    set c [string length $output];

    puts $c;
    puts -nonewline $output;
    flush stdout;
};
::ssh::push $slave $script;
::ssh::send $slave;

::ssh::pop_line $slave output;
set c [string trim $output];
::ssh::pop_read $slave $c output; 
puts "first: $output";

set script {
    set output "This is the second line of output.";
    set c [string length $output];

    puts $c;
    puts -nonewline $output;
    flush stdout;
};
::ssh::push $slave $script;
::ssh::send $slave;

::ssh::pop_line $slave output;
set c [string trim $output];
::ssh::pop_read $slave $c output; 
puts "second: $output";

::ssh::disconnect $slave;
