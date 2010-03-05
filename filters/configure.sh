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
panfilter
gribfilter
netfilter
spoolfilter
msgfilter
trackfilter
scheduler
recover"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
