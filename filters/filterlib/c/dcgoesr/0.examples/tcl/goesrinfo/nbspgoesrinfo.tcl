#!/usr/local/bin/tclsh8.6

source "goesr_info.lib";

#
# main
#

if {$argc == 0} {
   puts "file?";
   return TCL_OK;
} else {
    set file [lindex $argv 0];
}

set status [catch {
    set goesrinfo [goesr_info $file];
} errmsg];

if {$status != 0} {
    puts $errmsg;
    return TCL_ERROR; 
}

set rc(goesr_product_name) [lindex $goesrinfo 0];
set rc(goesr_start_date_time) [lindex $goesrinfo 1];
set rc(goesr_source_scene) [lindex $goesrinfo 2];
set rc(goesr_satellite_id) [lindex $goesrinfo 3];
set rc(goesr_abi_mode) [lindex $goesrinfo 4];
set rc(goesr_channel_id) [lindex $goesrinfo 5];

# check
puts [join $goesrinfo " "];
puts $rc(goesr_product_name);
puts $rc(goesr_start_date_time);
puts $rc(goesr_source_scene);
puts $rc(goesr_satellite_id);
puts $rc(goesr_abi_mode);
puts $rc(goesr_channel_id);
