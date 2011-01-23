#!/bin/sh

project=nbsp
#
#masterhost="http://svn.1-loop.net"
masterhost="svn+ssh://jfnieves@svn.1-loop.net/home/jfnieves/svn"
#
masterrepo="nbsprepo"
tag=trunk
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# nbsptclhttpd receives special treatment
tcllibs="tclgrads tclgempak tclmetar tclssh tclupperair"
srclibs="libconnth libqdb libspoolbdb libtclconf"
tclhttpd=${project}tclhttpd

# Override tag with the cmd line argument ("tags/nbsp-2.0.r1")
[ $# -ne 0 ] && tag=$1

svn co $mastersite/$project/$tag $project
cd $project
for p in $tcllibs
do
  svn co $mastersite/$p/$tag $p
done
svn co $mastersite/$tclhttpd/$tag tclhttpd

cd src
for p in $srclibs
do
  svn co $mastersite/$p/$tag $p
done
