match_pipe($wmoid, wtpz4, $gdec_bindir/dctrop,
-d $gdec_logdir/dctrop.log m4GEMTBL, storm/tropic/epacific/YYYY_@@.gem)dnl

match_pipe($wmoid, wtpn3, $gdec_bindir/dctrop,
-d $gdec_logdir/dctrop.log m4GEMTBL, storm/tropic/wpacific/YYYY_@@.gem)dnl

match_pipe($wmoid, wtnt4, $gdec_bindir/dctrop,
-d $gdec_logdir/dctrop.log m4GEMTBL, storm/tropic/atlantic/YYYY_@@.gem)dnl

match_pipe($wmoid, wtpa4, $gdec_bindir/dctrop,
-d $gdec_logdir/dctrop.log m4GEMTBL, storm/tropic/cpacific/YYYY_@@.gem)dnl

dnl
dnl dchrcn
dnl
match_pipe($wmoid, wt(nt|pz|pa)2[1-5], $gdec_bindir/dchrcn,
-d $gdec_logdir/dchrcn.log m4GEMTBL, storm/hrcn/YYYYMMDDHH.hrcn)dnl

dnl
dnl Decoder for Winter Warnings, Watches and Advisories
dnl
match_pipe($wmoid, wwus4[1-6], $gdec_bindir/dcwstm,
-d $gdec_logdir/dcwstm.log m4GEMTBL, storm/wstm/YYYYMMDDHH.wstm)dnl
