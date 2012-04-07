#!%TCLSH%

source ../src/ssh.tcl

# This is a script that will be sent to the slave.
set script {
puts "I am a remote host named [info hostname] executing a script
that my master sent me. I am going to sent him back the results of some
commands. Here they are:

    The date is: [exec date]
    My info is: [exec uname -a]

Now I need to inform my master that I have finished.

DONE 0"
};

set slave "diablo";

::ssh::connect -t %TCLSHT% -- $slave
::ssh::push $slave $script
::ssh::send $slave;
while {[::ssh::pop_line $slave line] >= 0} {
    if {[regexp {^DONE\s+(\d+)} $line match code]} {
	break;
    }
    puts $line;
}
::ssh::disconnect $slave;

puts "Slave finished with code: $code";
