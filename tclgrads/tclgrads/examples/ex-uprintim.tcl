#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";

gradsu::mset gxout shaded;
gradsu::display tmpprs;
gradsu::printim "ex-uprintim.png";

grads::end;
