#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";

grads::exec set gxout shaded;
grads::exec d tmpprs;
grads::exec printim "ex-printim.png";

grads::end;
