#!%WISH%
#
# $Id$
#
package require fileutil;

## The common grib tools initialization
set initfile "/usr/local/libexec/nbsp/nbspgribtools.init";
if {[file exists $initfile] == 0} {
    puts "$initfile not found.";
    return 1;
}
source $initfile;
unset initfile;

package require nbsp::errx;

# Configuration 
set browser(catfile) [file join \
    $gribfilter(datadir) $gribfilter(catalogdir) $gribfilter(ctlcatalog)];
if {[file exists $browser(catfile)] == 0} {
    ::nbsp::errx::err "$browser(catfile) not found.";
}

wm minsize . 30 20;
wm title . "Grib browser";

proc proc_browse {file} {
    
    global browser;
    
    set browser(current) [file tail $file];
    set browser(curix) [lsearch $browser(list) $file];
    set t $browser(text);
    $t config -state normal;
    $t delete 1.0 end;
    $t insert end [exec cat $file];
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

proc proc_wgrib {} {

    global browser;

    if {$browser(curix) == -1} {
	puts "No file selected.";
	return;
    }

    set ctlfile [lindex $browser(list) $browser(curix)];
    set grbfile [lindex [split [exec head -n 1 $ctlfile]] 1];
    set t $browser(text);
    $t config -state normal;
    $t delete 1.0 end;
    $t insert end [exec nbspwgrib $grbfile];
    $t config -state disabled;
}

proc proc_grads {} {

    global browser gribfilter;

    if {$browser(curix) == -1} {
	puts "No file selected.";
	return;
    }

    set ctlfile [lindex $browser(list) $browser(curix)];

    exec xterm -e grads -l -c "open $ctlfile" &; 
}

#
# main
#
set browser(list) [list];
foreach line [lsort -dictionary [split [exec cat $browser(catfile)] "\n"]] {
    set parts [split $line ":"];
    set model [lindex $parts 0];
    set grid [lindex $parts 1];
    set time [lindex $parts 2];
    set ftime [lindex $parts 3];
    set rpath [lindex $parts 4];
    set name [file rootname [file tail $rpath]];
    set fpath [file join $gribfilter(datadir) $gribfilter(ctldatadir) $rpath];
    lappend browser(list) $fpath;
    lappend filelist($model,$time) $fpath;

    if {([info exists modeltimes($model)] == 0) || \
	    ([lsearch -exact $modeltimes($model) $time] == -1)} {
	lappend modeltimes($model) $time;
    }
}
if {[llength $browser(list)] == 0} {
    ::nbsp::errx::err "No control files found.";
}
set browser(curix) -1;

# Top Buttons
set f [frame .menubar];
pack $f -fill x;

button $f.quit -text Quit -command exit;
button $f.next -text Next -command proc_next;
button $f.prev -text Previous -command proc_previous;
button $f.wgrib -text Wgrib -command proc_wgrib;
button $f.grads -text Grads -command proc_grads;
pack $f.quit $f.next $f.prev -side right;

label $f.label -textvariable browser(current);
pack $f.label;

menubutton $f.ctl -text "File List" -menu $f.ctl.m;
pack $f.ctl $f.wgrib $f.grads -side left;
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
