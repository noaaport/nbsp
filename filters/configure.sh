#!/bin/sh

subdirs="nbspfilter
gpfilter
rstfilter
dafilter
nntpfilter
rstnntpfilter
inventory
rssfilter
capfilter
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
recover
clusterfilter
arcfilter"

for d in $subdirs
do
    cd $d
    ./configure.sh
    cd ..
done
