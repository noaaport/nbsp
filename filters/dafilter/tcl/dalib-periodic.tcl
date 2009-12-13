proc dafilter_periodic {} {
#
# The hourly function to send the files to the news server.
# (Disabled if running as a worker slave.)

    global dafilter;

    if {$dafilter(option_s) == 0} {
	dafilter_nntp;
    }
}
