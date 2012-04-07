#
# $Id$
#
# Usage: http://<server>[:<port>]/_catalog[/<directory>]
#        http://<server>[:<port>]/_catalog_get/<directory>/<file>
#
# Examples:
#
## wget 'http://diablo:8015/_catalog'
## wget 'http://diablo:8015/_catalog/digatmos/surface'
## wget 'http://diablo:8015/_catalog_get/digatmos/surface/2010032409.sao'
#
Url_PrefixInstall /_catalog [list _catalog /_catalog];
Url_PrefixInstall /_catalog_get [list _catalog_get /_catalog_get];
#
set catalog(conf) "catalog.conf";

# Site configurable options
set catalog(confdir) $Config(confdir);
set catalog(localconfdirs) $Config(localconfdirs);
set catalog(databasedir) $Config(docRoot);
set catalog(exportdirs) [list "digatmos" "gis" "gempak" "grib"];
set catalog(templates) "catalog-templates.def";

# The local overrides
set _conf [file join $catalog(confdir) $catalog(conf)];
if {[file exists ${_conf}]} {
    source ${_conf};
}
unset _conf;

# The templates file
set _conf [file join $catalog(confdir) $catalog(templates)];
if {[file exists ${_conf}]} {
    source ${_conf};
} else {
    Stderr "Disabling the tclhttpd catalog extension. ${_conf} not found.";
    #
    # This will prevent reading the rest of the file, and therefore the
    # _catalog and _cataglog_get url's will be undefined.
    #
    return 1;
}
unset _conf;

proc _catalog {prefix sock suffix} {

    global catalog;

    # Check that there are no "../" constructs
    if {[regexp {\.{2}/} $suffix]} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    #
    # suffix is of the form "/<subdir_1>/.../<subdir_n>",
    # where the tail must be a directory.
    #
    if {$suffix eq ""} {
	# Return the first exported directory
	append suffix "/" [lindex $catalog(exportdirs) 0];
    }

    append dfpath $catalog(databasedir) $suffix;
    if {[file isdirectory $dfpath] == 0} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    #
    # Get the file list in subdir_n, and return the xml formatted catalog.
    #
    set subdir [string trim $suffix "/"];   # remove leading "/"
    set r [catalog_output_xml_start $subdir];

    foreach f [lsort [glob -directory $dfpath -nocomplain "*"]] {
	set fbasename [file tail $f];
	if {[file isdirectory $f]} {
	    append r [catalog_output_xml_dir $subdir $fbasename];
	} else {
	    append r [catalog_output_xml_file $subdir $fbasename];
	}
    }

    append r [catalog_output_xml_end];
    set type "text/xml";

    Httpd_AddHeaders $sock "Accept-Ranges" "bytes";
    Httpd_ReturnData $sock $type $r;
}

proc _catalog_get {prefix sock suffix} {

    global catalog;
    global Httpd;
    upvar #0 Httpd$sock data;

    # Check that there are no "../" constructs
    if {[regexp {\.{2}/} $suffix]} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    # suffix is one of the form "/<subdir_1>/.../<subdir_n>/<file>"
    append fpath $catalog(databasedir) $suffix;
    set type "application/octet-stream";

    if {[file exists $fpath] == 0} {
        Httpd_Error $sock 404 "$suffix not found.";
	return;
    }

    # This function is overriden in overeid.tcl in order to
    # support Bytes Ranges.
    Httpd_ReturnFile $sock $type $fpath;
}

proc catalog_output_xml_file {subdir fbasename} {

    global catalog;

    set fpath [file join $catalog(databasedir) $subdir $fbasename];
    set mtime [file mtime $fpath];

    # substitute subdir, fbasename and mtime
    set r [subst $catalog(xml_tmpl_file)];

    return $r;
}

proc catalog_output_xml_dir {subdir fbasename} {
#
# This must be called when "fbasename" is actually a directory; for example
# subdir is digatmos/nexrad and fbasename is nids.  
#
    global catalog;

    # substitute subdir and fbasename
    set r [subst $catalog(xml_tmpl_dir)];

    return $r;
}

proc catalog_output_xml_start {subdir} {

    global catalog;

    set r [subst $catalog(xml_tmpl_start)];

    return $r;
}

proc catalog_output_xml_end {} {

    global catalog;

    set r $catalog(xml_tmpl_end);

    return $r;
}
