#!/bin/sh

subdirs="nbspfilter
gpfilter
rstfilter
dafilter
nntpfilter
rstnntpfilter
inventory
rssfilter
filterlib
ldmfilter
metarfilter
uafilter
wsfilter
gisfilter
panfilter
gribfilter
netfilter
spoolfilter
msgfilter
trackfilter
craftfilter
masterfilter
scheduler
recover"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
