#!/usr/local/bin/tclsh8.4

source gempak.tcl

# first one
gempak::map_color 1;
gempak::map_width 2;
::gempak::set_map

# second one
gempak::map_color 2;
gempak::map_width 3;
::gempak::append_map

::gempak::puts_all_param_vars;
