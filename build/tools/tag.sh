#!/bin/sh

. ../../VERSION

masterrepo="nbsprepo"
#
project=$name
tag=${name}-${version}
#
#masterhost="http://svn.1-loop.net"
masterhost="svn+ssh://jfnieves@svn.1-loop.net/home/jfnieves/svn"
#
## mastersite="svn+ssh://diablo/home/svn"
mastersite=${masterhost}/${masterrepo}

[ $# -ne 0 ] && tag=$1

cd ../../../
echo svn copy $project $mastersite/$project/tags/$tag
