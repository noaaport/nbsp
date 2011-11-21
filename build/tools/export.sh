#!/bin/sh

project=nbsp
#
host="bzr+ssh://repo.1-loop.net"
#
repo="home/repo/bzr/noaaport"
tag=trunk
#
site=${host}/${repo}

# nbsptclhttpd receives special treatment
tcllibs="tclgrads tclgempak tclmetar tclssh tclupperair"
srclibs="libconnth libqdb libspoolbdb libtclconf"
tclhttpd=${project}tclhttpd

exclude="build dev-notes examples"
tmpdir=tmp

# read name and version
. ../../VERSION

rm -r -f $tmpdir
mkdir $tmpdir
cd $tmpdir

bzr export ${name}-$version $site/$name/$tag
cd ${name}-${version}
rm -r $exclude
for p in $tcllibs
do
  bzr export $p $site/$p/$tag
done
bzr export tclhttpd $site/$tclhttpd/$tag

cd src
for p in $srclibs
do
  bzr export $p $site/$p/$tag
done

cd ../.. 
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
