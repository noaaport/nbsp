#!/usr/local/bin/tclsh8.4
#
# $Id$
#

# To test without installing uncomment the nextline
lappend auto_path "..";
package require gempak;

gempak::init gpmap;

gempak::define device "gif|wwa.gif";
gempak::define garea "us";

gempak::define wstm "last";
gempak::define warn "last";
gempak::define watch "last";
gempak::define wcn "last";
gempak::define wcp "last";


gempak::run;
gempak::end;
