#!/bin/sh
#
# $Id$
#

template=newsyslog.tmpl
target=newsyslog.conf
logdir=/var/log/noaaport

files="
dccsig.log
dcffg.log
dcgmos.log
dcgrib.log
dchrcn.log
dcidft.log
dcisig.log
dclsfc.log
dcmetr.log
dcmsfc.log
dcmsfc_6hr.log
dcnmos.log
dcrdf.log
dcscd.log
dcstorm.log
dcsvrl.log
dctaf.log
dctrop.log
dcuair.log
dcuair_drop.log
dcwarn.log
dcwatch.log
dcwcn.log
dcwou.log
dcwstm.log
dcwtch.log
dcxmos.log"

cat $template > $target

for f in $files
do
    printf "$logdir/$f\t\t644  4      100 *     ZN\n"
done >> $target
