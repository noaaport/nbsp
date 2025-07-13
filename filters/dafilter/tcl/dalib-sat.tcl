proc filter_sat {seq fpath savedir savename {aws3flag 0}} {

    global dafilter;

    if {[is_sat_rule_enabled $savedir] == 0} {
	return;
    }

    if {$dafilter(archive_sat_enable) == 2} {
	return;
    }

    set _pwd [pwd];
    cd $dafilter(datadir);
    file mkdir $savedir;

    # the full output path
    set datafpath [file join $dafilter(datadir) $savedir $savename];

    set status [catch {
	#
	# If we want to remove the ccb and do not add the gempak header
	# and footer,
	#
	#   filterlib_exec_nbspfile $seq $fpath $savedir $savename "-t";
	#
	# If we want to remove also the wmo header (that is, leave only
	# the data (e.g. the nc files starting with the \211HDF) we could use
	# (-w implies -t)
	#
	#   filterlib_exec_nbspfile $seq $fpath $savedir $savename "-w";
	#
	# Since these files do not have the awips line, only the first line
	# with ccb+wmoheader, it would seem that this is the simplest way,
	#
	#   exec tail -n +2 $fpath > $datafpath
	#
	# However, if the ccb by chance contains a byte "\n", this would not
	# be correctly interpreted by tail and the output would contain
	# part of the header. In this sense it would be safer to use
	#
	#   exec tail -c +25 $fpath | tail -n +2 > $datafpath
	#
	# This assumes that the input files have the ccb, which is the
	# default configuration in freebsd. But in principle nbsp can be
	# configured to save the files in the spool without the ccb. An
	# alternative is to use directly nbspfile, which detects the presence
	# or absence of the ccb, or to be more consistent use
	# filterlib_exec_nbspfile which is aware of the configuration used
	# in nbsp.
	#
	# Furthermore, the "seq" argument to filterlib_exec_nbspfile
	# is used only when nbspfile adds the gempak header which is not
	# the case here so we simply pass 0.
	#
	if {$aws3flag == 0} {
	    filterlib_exec_nbspfile 0 $fpath $savedir $savename "-w";
	} else {
	    # aws s3 files do not have a ccb, and the gempak header
	    # will not be inserted. It is then simpler to
	    # copy it directly rather than calling nbspfile with options.
	    file copy -force $fpath $datafpath;
	}
    } errmsg];
    
    if {$status != 0} {
	file delete $datafpath;
	log_msg $errmsg;
    }

    if {$status == 0} {
	filter_sat_insert_inventory $savedir $datafpath;

	# Create the link to the latest
	make_sat_latest $savedir $savename;
    }
    
    cd $_pwd;
}
