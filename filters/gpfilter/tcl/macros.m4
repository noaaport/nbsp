dnl
dnl $Id$
dnl
define(m4GEMTBL, -e GEMTBL=$gpfilter(dec_GEMTBL))dnl
define(m4GEMPAK, -e GEMPAK=$gpfilter(dec_GEMPAK))dnl
dnl
dnl Common options for the decoders
dnl 
define(m4DCOPTS,
-d $gpfilter(dec_logdir)/$1.log m4GEMTBL -t $gpfilter(dec_timeout))dnl
