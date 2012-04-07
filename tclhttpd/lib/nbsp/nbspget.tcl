#
# $Id$
#
# The function nbsp_get implements the domain handler
#
#	http://<host>:8015/_get/spool/<fbasename>,
#	http://<host>:8015/_get/inv/<hhmm>,
#	http://<host>:8015/_get/rss/<rsschannel>,
#
# which can be used to request file transfers after a panfilter notification,
# or periodically request the latest minutely inventory file.
# (Note that there is no <station> subdir between <fbasename> and "spool".)
# The transfered file is the raw file that nbsp saves in the spool directory.
# If nbsp is configured to save the spool files with the CCB, then the
# transfered file will contain the (24 bytes) CCB.
# For the inventory files, <hhmm> should not contain the ".log" extension.
#
# The function nbsp_query implements
#
#	http://<host>:8015/_query/spool/station=<kkkk>&select=<glob pattern>
#
# An additional argument can be passed as "&format=xml" to return an xml
# formatted list.
#
# Functions analogous to this one, and to the _get handler,
# but acting on the data files (rather then the spool files) are implemented
# in the "exporter" module.
# 
Direct_Url /_query nbsp_query
Url_PrefixInstall /_get [list nbsp_get /_get]

proc nbsp_get {prefix sock suffix} {

    global Config;

    # Check that there are no "../" constructs, and then go past the "/spool".
    if {[regexp {\.{2}/} $suffix]} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    if {[regexp {^(/spool/)(.+)} $suffix match s1 s2]} {
	# $suffix contains "<fname>" without the leading "/"
	set suffix $s2;
	nbsp_get_spool $sock $suffix;
    } elseif {[regexp {^(/inv/)(.+)} $suffix match s1 s2]} {
	set suffix $s2;
	nbsp_get_inv $sock $suffix;
    } elseif {[regexp {^(/rss/)(.+)} $suffix match s1 s2]} {
	set suffix $s2;
	nbsp_get_rss $sock $suffix;	
    } else {
        Httpd_Error $sock 400 "Invalid specification.";
    }
}

proc nbsp_get_spool {sock suffix} {

    global Config;

    set fbasename $suffix;
    #
    # This assumes that the files are saved in the <station> subdirs,
    # and the <fbasename> contains the <station> prefix.
    #
    set station [string range $fbasename 0 3];
    set fpath [file join $Config(nbspspooldir) $station $fbasename];
    set type "application/octet-stream";

    if {[file exists $fpath] == 0} {
        Httpd_Error $sock 404 "$fbasename not found.";
	return;
    }

    Httpd_ReturnFile $sock $type $fpath;
}

proc nbsp_get_inv {sock suffix} {

    global Config;

    # The suffix does not include the leading slash, and it does _not_
    # include the extension of the inventory file.

    set pname "";
    append pname $suffix $Config(nbspinvfext);
    set fpath [file join $Config(nbspinvdir) $pname];
    set type "application/octet-stream";

    if {[file exists $fpath] == 0} {
	Httpd_Error $sock 404 "$pname not found.";
	return;
    }

    Httpd_ReturnFile $sock $type $fpath;
}

proc nbsp_get_rss {sock suffix} {

    global Config;

    # The suffix does not include the leading slash, and it does _not_
    # include the extension of the rss file.

    set pname "";
    append pname $suffix $Config(nbsprssfext);
    set fpath [file join $Config(nbsprssdir) $pname];
    set type "text/xml";

    if {[file exists $fpath] == 0} {
	Httpd_Error $sock 404 "$pname not found.";
	return;
    }

    Httpd_ReturnFile $sock $type $fpath;
}

proc nbsp_query/spool {station select {format "csv"}} {

    global Config;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set nbsp_query/spool "text/plain";

    if {$format eq "xml"} {
	set listtype "filelist";
	set r [nbsp_query_output_xml_start $listtype];
	set nbsp_query/spool "text/xml";
    } else {
	set r "";
    }
    
    set d [file join $Config(nbspspooldir) $station];
    if {[file isdirectory $d] == 0} {
	if {$format eq "xml"} {
	    append r [nbsp_query_output_xml_end $listtype];
	}
	return $r;
    }

    set filelist [glob -directory $d -nocomplain -tails $select];
    foreach f $filelist {
	if {$format eq "xml"} {
	    append r "<fname>" $f "</fname>\n";
	} else {
	    append r $f "\n";
	}
    }
    if {$format eq "xml"} {
	append r [nbsp_query_output_xml_end $listtype];
    }
    return $r;
}

# This is an auxiliary function that is used in the inventory
# functions in order to decide whether or not to output a link
# to the file or just the name.
#
proc nbsp_get_file_exists fname {

    global Config;

    set station [string range $fname 0 3];
    set fpath [file join $Config(nbspspooldir) $station $fname];

    return [file exists $fpath];
}

proc nbsp_query_output_xml_start {list_type} {

    set r "<?spooler_${list_type} version=\"1.0\"?>\n\n";
    append r "<spooler_${list_type}>\n";

    return $r;
}

proc nbsp_query_output_xml_end {list_type} {

    set r "</spooler_${list_type}>\n";

    return $r;
}
