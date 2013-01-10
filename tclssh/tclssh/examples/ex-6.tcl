#!%TCLSH%

source ../src/ssh.tcl

# This is a script that will be sent to the slave.
set script {
puts "I am a remote host named [info hostname] executing a script
that my master sent me. I am going to sent him back the results of some
commands. Here they are:

    The date is: [exec date]
    My info is: [exec uname -m]

Now I need to inform my master that I have finished."
};

set slave "diablo";

::ssh::connect -t %TCLSHT% -- $slave;
::ssh::rexec $slave $script output
::ssh::disconnect $slave;

puts $output;
