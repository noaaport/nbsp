#!/bin/sh

project=nbsp
masterhost="http://svn.1-loop.net"
masterrepo="nbsprepo"
tag=trunk
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

# nbsptclhttpd receives special treatment
tcllibs="tclgrads tclgempak tclmetar tclupperair"
srclibs="libconnth libqdb libspoolbdb libtclconf"
tclhttpd=${project}tclhttpd

exclude="build dev-notes examples"
tmpdir=tmp

# Override tag with the cmd line argument ("tags/nbsp-2.0.r1")
[ $# -ne 0 ] && tag=$1

# read name and version
. ../../VERSION

rm -r -f $tmpdir
mkdir $tmpdir
cd $tmpdir

svn export $mastersite/$name/$tag ${name}-$version
cd ${name}-${version}
rm -r $exclude
for p in $tcllibs
do
  svn export $mastersite/$p/$tag $p
done
svn export $mastersite/$tclhttpd/$tag tclhttpd

cd src
for p in $srclibs
do
  svn export $mastersite/$p/$tag $p
done

cd ../.. 
tar -czf ../${name}-${version}.tgz ${name}-${version}

cd ..
rm -r $tmpdir
