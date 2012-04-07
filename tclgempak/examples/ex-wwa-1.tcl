#!%TCLSH%
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# main
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
