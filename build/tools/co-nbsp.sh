#!/bin/sh

project=nbsp
#
host="bzr+ssh://linode.1-loop.net"
#
repo="home/repo/bzr/noaaport"
tag=trunk
#
site=${host}/${repo}

# nbsptclhttpd receives special treatment
tcllibs="tclgrads tclgempak tclmetar tclssh tclupperair"
srclibs="libconnth libqdb libspoolbdb libtclconf"
tclhttpd=${project}tclhttpd

bzr branch $site/$project/$tag $project
cd $project
for p in $tcllibs
do
  bzr branch $site/$p/$tag $p
done
bzr branch $site/$tclhttpd/$tag tclhttpd

cd src
for p in $srclibs
do
  bzr branch $site/$p/$tag $p
done
