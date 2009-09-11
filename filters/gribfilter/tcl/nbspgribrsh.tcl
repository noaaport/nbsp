#!%WISH%
#
# $Id$
#
# Usage: nbspgribrsh [-- [-c ctlcatalog] [-d local_datadir] [-f conffile]] \ 
#		[baseurl]
#
# The names of the subdirectories inside the "grib" directory can be
# configured, but it is assumed that the structure of the tree is
# the default one:  {cat,ctl,grb,idx}/<model>/<time>/*files*
#
package require cmdline;
package require fileutil;

set usage {Usage: nbspgribrsh [-- [-c ctlcatalog] [-d local_datadir]
    [-f conffile]] [baseurl]};
set optlist {{c.arg ""} {d.arg ""} {f.arg ""}};
    
## The common gribrsh tools initialization
set initfile "/usr/local/libexec/nbsp/nbspgribrsh.init";
if {[file exists $initfile] == 0} {
    puts "$initfile not found.";
    exit 1;
}
source $initfile;
unset initfile;

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$option(f) ne ""} {
    source $option(f);
}
if {$argc == 1} {
    set nbspgribrsh(baseurl) [lindex $argv 0];
} elseif {$argc == 0} {
    puts "Error: $usage";
    exit 1;
}

if {$option(c) ne ""} {
    set nbspgribrsh(ctlcatalog) $option(c);
}

if {$option(d) ne ""} {
    set nbspgribrsh(datadir) $option(d);
}

# Variables
set nbspgribrsh(grburl) "$nbspgribrsh(baseurl)/$nbspgribrsh(grbbasedir)";
set nbspgribrsh(ctlurl) "$nbspgribrsh(baseurl)/$nbspgribrsh(ctlbasedir)";
set nbspgribrsh(caturl) "$nbspgribrsh(baseurl)/$nbspgribrsh(catbasedir)";
set nbspgribrsh(catfile) [file join $nbspgribrsh(datadir) \
			  $nbspgribrsh(catdatadir) $nbspgribrsh(ctlcatalog)];
set nbspgribrsh(catalogurl) "$nbspgribrsh(caturl)/$nbspgribrsh(ctlcatalog)";

#
# Support functions
#
proc proc_make_local_names {grbsubdir grbfbasename} {
#
# grbsubdir is the subdirectory of "grb" where the file is; i.e.,
# <model>/<time> in the default settings.
#
    global nbspgribrsh;

    set fname [file rootname $grbfbasename];
    set grbfile [file join $nbspgribrsh(datadir) $nbspgribrsh(grbdatadir) \
			$grbsubdir $grbfbasename];
    set ctlfile [file join $nbspgribrsh(datadir) $nbspgribrsh(ctldatadir) \
		     $grbsubdir ${fname}$nbspgribrsh(ctlfext)];
    set idxfile [file join $nbspgribrsh(datadir) $nbspgribrsh(idxdatadir) \
		     $grbsubdir ${fname}$nbspgribrsh(idxfext)];

    return [list $grbfile $ctlfile $idxfile];
}

proc proc_make_local_names_by_curix {} {
# 
# The grb file name is extracted from the first line of the ctl
# file that is currently displayed. The subdirectory structure
# of the grb files is assumed to parallel the ctl.
#
    global nbspgribrsh;
    global browser;

    set firstline [lindex [split $browser(current_output) "\n"] 0];
    set grbfbasename [file tail [lindex $firstline 1]];
    set ctlrpath [lindex $browser(rlist) $browser(curix)];
    set grbsubdir [file dirname $ctlrpath];

    return [proc_make_local_names $grbsubdir $grbfbasename];
}

proc proc_make_grb_url {grbsubdir grbfbasename} {
#
# grbsubdir is the subdirectory of "grb" where the file is; i.e.,
# <model>/<time> in the default settings.
#
    global nbspgribrsh;

    set fileurl "$nbspgribrsh(grburl)/$grbsubdir/$grbfbasename";

    return $fileurl;
}

proc proc_make_grb_url_by_curix {} {
#
# The grb file name is extracted from the first line of the ctl
# file that is currently displayed. The subdirectory structure
# of the grb files is assumed to parallel the ctl, and with those
# two the url of the grb file is constructed.
#
    global nbspgribrsh;
    global browser;

    set firstline [lindex [split $browser(current_output) "\n"] 0];
    set grbfbasename [file tail [lindex $firstline 1]];
    set ctlrpath [lindex $browser(rlist) $browser(curix)];
    set grbsubdir [file dirname $ctlrpath];

    return [proc_make_grb_url $grbsubdir $grbfbasename];
}

proc proc_make_names_by_curix {} {
# 
# This combines the functionality of proc proc_make_local_names_by_curix {}
# and proc proc_make_grb_url_by_curix {}.
#
    global nbspgribrsh;
    global browser;

    set firstline [lindex [split $browser(current_output) "\n"] 0];
    set grbfbasename [file tail [lindex $firstline 1]];
    set ctlrpath [lindex $browser(rlist) $browser(curix)];
    set grbsubdir [file dirname $ctlrpath];

    return [concat [proc_make_local_names $grbsubdir $grbfbasename] \
		[proc_make_grb_url $grbsubdir $grbfbasename]];
}

proc proc_make_local_ctl {grbsubdir grbfbasename} {
#
# grbsubdir is the subdirectory of "grb" where the file is; i.e.,
# <model>/<time> in the default settings.
#
    global nbspgribrsh;

    set names [proc_make_local_names $grbsubdir $grbfbasename];
    set grbfile [lindex $names 0];
    set ctlfile [lindex $names 1];
    set idxfile [lindex $names 2];

    file mkdir [file dirname $ctlfile];
    file mkdir [file dirname $idxfile];

    set status [catch {
	exec nbspgribctl -c $ctlfile -i $idxfile $grbfile;
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
    }
    
    return $status;
}

proc proc_make_local_ctl_by_curix {} {

    global nbspgribrsh;
    global browser;

    set firstline [lindex [split $browser(current_output) "\n"] 0];
    set grbfbasename [file tail [lindex $firstline 1]];
    set ctlrpath [lindex $browser(rlist) $browser(curix)];
    set grbsubdir [file dirname $ctlrpath];

    set status [proc_make_local_ctl $grbsubdir $grbfbasename];

    return $status;
}

proc proc_local_grb_exists_by_curix {} {

    global nbspgribrsh;
    global browser;

    set names [proc_make_local_names_by_curix];
    set grbfile [lindex $names 0];
    if {[file exists $grbfile]} {
	return 1;
    }

    return 0;
}

#
# Interface functions
#
proc proc_browse {file} {
    
    global browser;

    set browser(current) [file tail $file];
    set browser(curix) [lsearch $browser(list) $file];
    set t $browser(text);
    $t config -state normal;
    $t delete 1.0 end;
    $t insert end [proc_get_ctl $file];
    $t config -state disabled;
}

proc proc_next {} {
    
    global browser;

    if {$browser(curix) < [llength $browser(list)] - 1} {
	incr browser(curix);
    }
    proc_browse [lindex $browser(list) $browser(curix)];
}

proc proc_previous {} {
    
    global browser;

    if {$browser(curix) > 0} {
	incr browser(curix) -1;
    }
    
    proc_browse [lindex $browser(list) $browser(curix)];
}

proc proc_scroll_text {f args} {
    
    frame $f;
    eval {text $f.text -wrap none -xscrollcommand [list $f.xscroll set] \
	      -yscrollcommand [list $f.yscroll set]} $args;
    scrollbar $f.xscroll -orient horizontal \
	-command [list $f.text xview];
    scrollbar $f.yscroll -orient vertical \
	-command [list $f.text yview];
    grid $f.text $f.yscroll -sticky news;
    grid $f.xscroll -sticky news;
    grid rowconfigure $f 0 -weight 1;
    grid columnconfigure $f 0 -weight 1;

    return $f.text;
}

proc proc_download_catalog {} {

    global nbspgribrsh;

    file mkdir [file dirname $nbspgribrsh(catfile)];
    set status [catch {
	exec curl -s -S -o $nbspgribrsh(catfile) $nbspgribrsh(catalogurl);
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
	exit 1;
    }
}

proc proc_get_ctl {fileurl} {

    global browser;

    set browser(current_output) [exec curl -s $fileurl];

    return $browser(current_output);
}

proc proc_make_local_datadirs {} {
#
# If nbspgribrsh(homedir) was not overriden then it is created.
# Otherwise, it is assumed that nbspgribrsh(datadir) exists.
# In any case, its subdirectories are created.

    global nbspgribrsh;

    if {$nbspgribrsh(homedir) eq $nbspgribrsh(default_homedir)} {
	file mkdir $nbspgribrsh(homedir);
    }

    if {[file isdirectory $nbspgribrsh(homedir)] == 0} {
	puts "Directory $nbspgribrsh(datadir) not found.";
	exit 1;
    }

    set currdir [pwd];

    cd $nbspgribrsh(homedir);
    file mkdir $nbspgribrsh(userconfdir);
    file mkdir $nbspgribrsh(datadir);

    cd $nbspgribrsh(datadir);
    file mkdir $nbspgribrsh(grbdatadir);
    file mkdir $nbspgribrsh(grbdatadir);
    file mkdir $nbspgribrsh(ctldatadir);
    file mkdir $nbspgribrsh(idxdatadir);
    file mkdir $nbspgribrsh(catdatadir);

    cd $currdir;
}

proc proc_download_grb {} {

    global nbspgribrsh;
    global browser;

    if {$browser(current_output) eq ""} {
	puts "No file selected.";
	return 1;
    }

    # The grb file name is extracted from the first line of the ctl
    # file that is currently displayed. The subdirectory structure
    # of the grb files is assumed to parallel the ctl, and with those
    # two the url of the grb file is constructed, as well as the
    # relative path for saving the downloaded file.

    set names [proc_make_names_by_curix];
    set grbfile [lindex $names 0];
    set fileurl [lindex $names 3];

    file mkdir [file dirname $grbfile];
    set status [catch {exec curl -s -S -o $grbfile $fileurl} errmsg];
    if {$status != 0} {
	puts $errmsg;
	return 1;
    }

    set status [proc_make_local_ctl_by_curix]
    if {$status == 0} {
	set browser(curix_downloaded) $browser(curix);
    }

    return $status;
}

proc proc_wgrib {} {

    global browser;

    if {$browser(curix) == -1} {
	puts "No file selected.";
	return;
    } elseif {$browser(curix_downloaded) != $browser(curix)} {
	if {[proc_local_grb_exists_by_curix] == 0} {
	    set status [proc_download_grb];
	    if {$status != 0} {
		return;
	    }
	}
    }

    set names [proc_make_local_names_by_curix];
    set grbfile [lindex $names 0];

    set t $browser(text);
    $t config -state normal;
    $t delete 1.0 end;
    $t insert end [exec nbspwgrib $grbfile];
    $t config -state disabled;
}

proc proc_grads {} {

    global browser;

    if {$browser(curix) == -1} {
	puts "No file selected.";
	return;
    } elseif {$browser(curix_downloaded) != $browser(curix)} {
	if {[proc_local_grb_exists_by_curix] == 0} {
	    set status [proc_download_grb];
	    if {$status != 0} {
		return;
	    }
	}
    }

    set ctlfile [lindex $browser(list) $browser(curix)];
    #
    # nbspgradrsh expects the path relative to the ctldir
    #
    set ctldir [file join $nbspgribrsh(datadir) $nbspgribrsh(ctldatadir)];
    set rpath [::fileutil::stripPath $ctldir $ctlfile];
 
    exec xterm -e nbspgradrsh $rpath &;
}

#
# main
#
wm minsize . 30 20;
wm title . "Grib browser";

proc_make_local_datadirs;
proc_download_catalog;
set browser(list) [list];
set browser(rlist) [list];
foreach line [lsort [split [exec cat $nbspgribrsh(catfile)] "\n"]] {
    set parts [split $line ":"];
    set model [lindex $parts 0];
    set grid [lindex $parts 1];
    set time [lindex $parts 2];
    set ftime [lindex $parts 3];
    set rpath [lindex $parts 4];
    set name [file rootname [file tail $rpath]];
    set fpath "$nbspgribrsh(ctlurl)/$rpath";
    lappend browser(list) $fpath;
    lappend filelist($model,$time) $fpath;
    lappend browser(rlist) $rpath;

    if {([info exists modeltimes($model)] == 0) || \
	    ([lsearch -exact $modeltimes($model) $time] == -1)} {
	lappend modeltimes($model) $time;
    }
}
if {[llength $browser(list)] == 0} {
    errx("No files found.");
}
set browser(curix) -1;
set browser(current_output) "";
set browser(curix_downloaded) -1;

# Top Buttons
set f [frame .menubar];
pack $f -fill x;

button $f.quit -text Quit -command exit;
button $f.next -text Next -command proc_next;
button $f.prev -text Previous -command proc_previous;
button $f.download -text Download -command proc_download_grb;
button $f.wgrib -text Wgrib -command proc_wgrib;
button $f.grads -text Grads -command proc_grads;
pack $f.quit $f.next $f.prev -side right;

label $f.label -textvariable browser(current);
pack $f.label;

menubutton $f.ctl -text "File List" -menu $f.ctl.m;
pack $f.ctl $f.download $f.wgrib $f.grads -side left;
set m [menu $f.ctl.m];

set browser(text) [proc_scroll_text .body -width 80 -height 10 -setgrid true];
pack .body -fill both -expand true;

# The menus
option add *Menu.tearOff 0;
set i 0;
foreach model [lsort [array names modeltimes]] {
    $m add cascade -label $model -menu $m.$i;
    set sub1 [menu $m.$i];
    set j 0;
    foreach time [lsort $modeltimes($model)] {
	$sub1 add cascade -label $time -menu $sub1.sub$j;
	set sub2 [menu $sub1.sub$j];
        foreach _f [lsort $filelist($model,$time)] {
            set name [file rootname [file tail ${_f}]];
            $sub2 add command -label $name -command [list proc_browse ${_f}];
        }
	incr j;
    }
    incr i;
}
