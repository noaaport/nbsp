#!/bin/sh

project=nbsp
mastersite="svn+ssh://diablo/home/svn"

# nbsptclhttpd receives special treatment
tcllibs="tclgrads tclgempak tclmetar tclupperair"
srclibs="libconnth libqdb libspoolbdb libtclconf"
tclhttpd=${project}tclhttpd

svn co $mastersite/$project/trunk $project
cd $project
for p in $tcllibs
do
  svn co $mastersite/$p/trunk $p
done
svn co $mastersite/$tclhttpd/trunk tclhttpd

cd src
for p in $srclibs
do
  svn co $mastersite/$p/trunk $p
done
