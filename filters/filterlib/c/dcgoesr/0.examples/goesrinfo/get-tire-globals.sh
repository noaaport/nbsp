#!/bin/sh
#
# From the set of files downloaded from "nbsp.firth.xyz",
# get the global parameters. This is useful to confirm which
# files are processable by our goers tools.
#
dir="/home/nieves/devel/untracked/noaaport/noaaport-goesr/sat-nbsp-files/nc"

for f in `ls $dir`
do
    echo -n "$f: "
    nbspgoesrinfo ${dir}/${f}
done
