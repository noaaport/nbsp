#
# $Id$
#
# This package contains a set of functions to be used by the
# filters to read from the spool cache bdb (cspool).
#
# The package requires the cmd line program
#
# nbspcspoolr
#
# from the nbsp libspool library.
#
# ::nbsp::cspoolbdb::init {conffile {localconfdirs [list]}}
# ::nbsp::cspoolbdb::open {}
# ::nbsp::cspoolbdb::close {}
# ::nbsp::cspoolbdb::read {key {size ""}}
# ::nbsp::cspoolbdb::get {key {size ""}}
# ::nbsp::cspoolbdb::query_enable {}
# ::nbsp::cspoolbdb::query_open {}
#
package provide nbsp::cspoolbdb 1.0;

namespace eval nbsp::cspoolbdb {} {

    variable cspoolbdb;

    # Configuration settings
    set cspoolbdb(dir) "";
    set cspoolbdb(name) "";
    set cspoolbdb(ndb) 0;
    set cspoolbdb(dbcache_mb) 0;
    set cspoolbdb(mpool_nofile) 0;
    set cspoolbdb(pagesize) 0;
    set cspoolbdb(enable) 0;
    set cspoolbdb(background) 0;
    set cspoolbdb(verbose) 0;

    # Return codes from nbspcspoolr
    set cspoolbdb(code_ok) "000";
    set cspoolbdb(code_notfound) "001";
    set cspoolbdb(code_error) "255";

    # Variables
    set cspoolbdb(F) "";
    set cspoolbdb(F_open) 0;

    # The return values from cspoolr
    set cspoolbdb(code) "";
    set cspoolbdb(data) "";
    set cspoolbdb(data_size) 0;
}

proc nbsp::cspoolbdb::init {conffile {localconfdirs [list]}} {
#
# This function should be called immediately after "package require".
#
    variable cspoolbdb;

    if {[file exists $conffile] == 0} {
	set cspoolbdb(code) $cspoolbdb(code_error);
	return -code error "$conffile not found.";
    }

    source $conffile;

    foreach k [list dir name ndb dbcache_mb mpool_nofile pagesize \
		   enable background verbose ] {
	eval set cspoolbdb($k) \$cspoolbdb_$k;
    }
}

proc nbsp::cspoolbdb::open {} {

    variable cspoolbdb;

    if {[::nbsp::cspoolbdb::query_enable] == 0} {
	return -code error "cspoolbdb is not enabled.";
    }

    if {$cspoolbdb(F_open) == 1} {
	set cspoolbdb(code) $cspoolbdb(code_error);
	return -code error "Already open.";
    }
    
    set cmd [list "|nbspcspoolr" \
		 -d $cspoolbdb(dir) \
		 -f $cspoolbdb(name) \
		 -n $cspoolbdb(ndb) \
		 -c $cspoolbdb(dbcache_mb) \
		 -s $cspoolbdb(pagesize)];

    if {$cspoolbdb(background) != 0} {
	lappend cmd "-b";
    }

    if {$cspoolbdb(mpool_nofile) != 0} {
	lappend cmd "-m";
    }

    if {$cspoolbdb(verbose) != 0} {
	lappend cmd "-v";
    }

    if {$cspoolbdb(verbose) > 1} {
	lappend cmd "-v";
    }

    set status [catch {
	set F [::open $cmd "r+"];
	fconfigure $F -translation binary -encoding binary;
    } errmsg];

    if {$status != 0} {
	set cspoolbdb(code) $cspoolbdb(code_error);
	return -code error $errmsg;
    }

    set cspoolbdb(F) $F;
    set cspoolbdb(F_open) 1;
}

proc nbsp::cspoolbdb::close {} {

    variable cspoolbdb;
    
    if {$cspoolbdb(F_open) == 0} {
	return;
    }

    set status 0;

    set status [catch {
	catch {puts "" $cspoolbdb(F)};	# an empty line makes cspoolr quit
	::close $cspoolbdb(F);
    } errmsg];

    set cspoolbdb(F) "";
    set cspoolbdb(F_open) 0;

    if {$status != 0} {
	set cspoolbdb(code) $cspoolbdb(code_error);
	return -code error $errmsg;
    }
}

proc nbsp::cspoolbdb::read {key {size 0}} {
#
# Returns a tcl list:  <code> <size> <data>
# The <code> is "001" if the file was not found, "255" in case of an error or
# "000" if there were no errors. If <size> is 0 then all the data is
# returned.
#
    variable cspoolbdb;

    # Check if it is closed (perhaps due to a previous error) and reopen it.
    if {$cspoolbdb(F_open) == 0} {
	::nbsp::cspoolbdb::open;
    }

    set cspoolbdb(data_size) 0;
#   set cspoolbdb(data) "";	# Leave it invalid

    set code "";
    set data_size_str "";
    set status [catch {
	set status [catch {
	    puts $cspoolbdb(F) "$key,$size";
	    flush $cspoolbdb(F);
	    # <code><size>
	    # The output from nbspcspoolr does not include the 0x,
	    # so that there are only eight bytes in <size>.
	    set header [::read $cspoolbdb(F) 11];    
	    set code [string range $header 0 2];
	    set data_size_str [string range $header 3 10];
	} errmsg];

	if {($status != 0) || ([string length $code] != 3)} {
	    set cspoolbdb(code) $cspoolbdb(code_error);
	    return -code error $errmsg;
	}

	if {$code eq $cspoolbdb(code_notfound)} {
	    set cspoolbdb(code) $code;
	    return -code error "$key not found in cspoolbdb";
	} elseif {$code eq $cspoolbdb(code_ok)} {
	    set cspoolbdb(code) $code;
	} else {
	    set cspoolbdb(code) $cspoolbdb(code_error);
	    return -code error "Error $code getting $key in ::nbsp::cspoolbdb::read";
	}
	
	if {[string length $data_size_str] != 8} {
	    set cspoolbdb(code) $cspoolbdb(code_error);
	    return -code error "Error 2 getting $key in ::nbsp::cspoolbdb::read";
	}

	scan $data_size_str "%x" data_size;
	set cspoolbdb(data) [::read $cspoolbdb(F) $data_size];
	set cspoolbdb(data_size) $data_size;
    } errmsg];

    if {$status != 0} {
	# In case of an error, close it. We will try to reopen the next time.
	if {$cspoolbdb(code) eq $cspoolbdb(code_error)} {
	    set status [catch {
		::nbsp::cspoolbdb::close;
	    } errmsg1];
	    if {$status != 0} {
		append errmsg "\n" $errmsg1;
	    }
	}
	return -code error $errmsg;
    }

    return [list $cspoolbdb(code) $cspoolbdb(data_size) $cspoolbdb(data)];
}

proc nbsp::cspoolbdb::get {key {size 0}} {
    #
    # Passing size as 0 returns all the data
    #
    ::nbsp::cspoolbdb::open;
    set result [::nbsp::cspoolbdb::read $key $size];
    ::nbsp::cspoolbdb::close;

    return $result;
}

proc nbsp::cspoolbdb::query_enable {} {

    variable cspoolbdb;

    return $cspoolbdb(enable);
}

proc nbsp::cspoolbdb::query_verbose {} {

    variable cspoolbdb;

    return $cspoolbdb(verbose);
}

proc nbsp::cspoolbdb::query_open {} {

    variable cspoolbdb;

    return $cspoolbdb(F_open);
}

proc nbsp::cspoolbdb::query_code_ok {} {

    variable cspoolbdb;

    if {$cspoolbdb(code) eq $cspoolbdb(code_ok)} {
	return 1;
    }

    return 0;
}

proc nbsp::cspoolbdb::query_code_notfound {} {

    variable cspoolbdb;

    if {$cspoolbdb(code) eq $cspoolbdb(code_notfound)} {
	return 1;
    }

    return 0;
}

proc nbsp::cspoolbdb::query_code_error {} {

    variable cspoolbdb;

    if {$cspoolbdb(code) eq $cspoolbdb(code_error)} {
	return 1;
    }

    return 0;
}

proc nbsp::cspoolbdb::query_code {} {

    variable cspoolbdb;

    return $cspoolbdb(code);
}

proc nbsp::cspoolbdb::query_data {} {

    variable cspoolbdb;

    return $cspoolbdb(data);
}

proc nbsp::cspoolbdb::query_data_size {} {

    variable cspoolbdb;

    return $cspoolbdb(data_size);
}

proc nbsp::cspoolbdb::set_verbose {{v 1}} {

    variable cspoolbdb;

    set cspoolbdb(verbose) $v;
}

proc nbsp::cspoolbdb::set_background {{v 1}} {

    variable cspoolbdb;

    set cspoolbdb(background) $v;
}
