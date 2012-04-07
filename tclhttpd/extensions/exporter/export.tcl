#
# $Id$
#
# Usage: http://<server>[:<port>]/<command>
#
# where <command> is
#
# /_export/query_list?select=<regexp>[&format=xml|csv]
# /_export/query_dir?dir=<dir>&select=<regexp>[&format=xml]
# /_export_get/<dir>/<file>
#
# Examples:
#
## wget 'http://diablo:8015/_export/query_list?select=nwx'
## wget 'http://diablo:8015/_export/query_dir? \
#    dir=digatmos/nwx/watch_warn/flw&select=20081211'
## wget 'http://diablo:8015/_export_get/ \
#    digatmos/nwx/watch_warn/flw/2008121115.flw'
#
Direct_Url /_export _export;
Url_PrefixInstall /_export_get [list _export_get /_export_get];

set export(conf) "export.conf";
set export(confdir) $Config(confdir);
set export(localconfdirs) $Config(localconfdirs);
set export(databasedir) "/var/noaaport/data";
set export(exportdirsdef) [list "export-da.def" "export-grib.def"];

# The local overrides
set _exportconf [file join $export(confdir) $export(conf)];
if {[file exists ${_exportconf}]} {
    source ${_exportconf};
}
unset _exportconf;

# The exported directories
foreach _name $export(exportdirsdef) {
    set _f [file join $export(confdir) ${_name}];
    if {[file exists ${_f}]} {
	source ${_f};
    }
}
unset _f;
unset _name;

proc _export/query_list {select {format "csv"}} {

    global export;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _export/query_list "text/plain";

    if {$format eq "xml"} {
	set listtype "dirlist";
	set r [export_output_xml_start $listtype];
	set _export/query_list "text/xml";
    } else {
	set r "";
    }
    foreach alias [array names export "dirs,*"] {
	set d [lindex [split $export($alias) ","] 1];
	if {[regexp $select $d]} {
	    if {$format eq "xml"} {
		append r [export_proddir_csvtoxml $export($alias)] "\n";
	    } else {
		append r $export($alias) "\n";
	    }
	}
    }
    if {$format eq "xml"} {
	append r [export_output_xml_end $listtype];
    }
    return $r;
}

proc _export/query_dir {dir select {format "csv"}} {

    global export;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _export/query_dir "text/plain";

    if {$format eq "xml"} {
	set listtype "filelist";
	set r [export_output_xml_start $listtype];
	set _export/query_list "text/xml";
    } else {
	set r "";
    }

    set d [file join $export(databasedir) $dir];
    if {[file isdirectory $d] == 0} {
	if {$format eq "xml"} {
	    append r [export_output_xml_end $listtype];
	}
	return $r;
    }

    set filelist [glob -directory $d -nocomplain -tails "*"];
    foreach f $filelist {
	if {[regexp $select $f]} {
	    if {$format eq "xml"} {
		append r "<fname>" $f "</fname>\n";
	    } else {
		append r $f "\n";
	    }
	}
    }
    if {$format eq "xml"} {
	append r [export_output_xml_end $listtype];
    }
    return $r;
}

proc _export_get {prefix sock suffix} {

    global export;

    # Check that there are no "../" constructs
    if {[regexp {\.{2}/} $suffix]} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    # suffix is of the form "/<dir>/<file>"
    append fpath $export(databasedir) $suffix;
    set type "application/octet-stream";

    if {[file exists $fpath] == 0} {
        Httpd_Error $sock 404 "$suffix not found.";
	return;
    }

    Httpd_ReturnFile $sock $type $fpath;
}

proc export_proddir_csvtoxml {s} {

    set parts [split $s ","];
    set desc [lindex $parts 0];
    set dir [lindex $parts 1];
    set fmt [lindex $parts 2];

    set r "<product>\n";
    append r "  <desc>$desc</desc>\n  <dir>$dir</dir>\n  <fmt>$fmt</fmt>\n";
    append r "</product>";

    return $r;
}

proc export_output_xml_start {list_type} {

    set r "<?exporter_${list_type} version=\"1.0\"?>\n\n";
    append r "<exporter_${list_type}>\n";

    return $r;
}

proc export_output_xml_end {list_type} {

    set r "</exporter_${list_type}>\n";

    return $r;
}
