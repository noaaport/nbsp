#!/bin/sh

project=nbsp
tag="tags/nbsp-2.0.r1"
#
masterhost="http://svn.1-loop.net"
masterrepo="nbsprepo"
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# nbsptclhttpd receives special treatment
tcllibs="tclgrads tclgempak tclmetar tclupperair"
srclibs="libconnth libqdb libspoolbdb libtclconf"
tclhttpd=${project}tclhttpd

cd ../../../
svn copy $project $mastersite/$project/$tag
cd $project

for p in $tcllibs
do
  svn copy $p $mastersite/$p/$tag
done
svn copy tclhttpd $mastersite/$tclhttpd/$tag

cd src
for p in $srclibs
do
  svn copy $p $mastersite/$p/$tag
done
