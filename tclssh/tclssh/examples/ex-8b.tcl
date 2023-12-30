#!/usr/local/bin/tclsh8.6

source ./ssh.tcl

# This is a script that will be sent to the slave.
set script {
    set f [open "garbage" w];
puts $f "I am a remote host named [info hostname] executing a script
that my master sent me. I am going to sent him back the results of some
commands. Here they are:

    The date is: [exec date]
    My info is: [exec uname -m]

Now I need to inform my master that I have finished."
close $f
};

set slave "diablo";

set status [catch {
    ::ssh::connect -t tclsh8.6 -- $slave;
    ::ssh::rexec_nopop $slave $script;
} errmsg];

if {$status != 0} {
    puts $errmsg;
} else {
    while {[::ssh::pop_line $slave line] >= 0} {
	puts $line;
    }
}

set status [catch {
    ::ssh::disconnect $slave;
} errmsg];

if {$status != 0} {
    puts $errmsg;
}
