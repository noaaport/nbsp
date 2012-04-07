#!/usr/local/bin/tclsh8.4

lappend auto_path "..";
package require gempak;

# 5/1|3/12/1.25/2|sfstns.tbl

::gempak::stnplt_text_color 5;
::gempak::stnplt_text_size 1;

::gempak::stnplt_marker_color 3;
::gempak::stnplt_marker_type 12;
::gempak::stnplt_marker_size 1.25;
::gempak::stnplt_marker_width 2;

::gempak::stnplt_stnfile_name "sfstns.tbl";
## ::gempak::stnplt_stnfile_column 3

::gempak::set_stnplt;

::gempak::puts_all_param_vars;
