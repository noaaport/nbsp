#!/bin/sh

. upload.conf

filename=${name}-${version}.tgz
TAR=tar
os=`uname`
case $os in
    FreeBSD|OpenBSD) pkgdir=bsd;;
    SunOS)           pkgdir=solaris; TAR=gtar;;
    Linux)           pkgdir=rpm;;
esac

grab_release(){
    _infofile=$1

    set `grep '^release' ${_infofile}`
    release=$3
}

grab_rpmroot(){
    _infofile=$1

    set `grep '^rpmroot' ${_infofile}`
    rpmroot=$3
}

wget $downloadproto://$downloadhost/$srcdownloaddir/$filename

${TAR} -xzf $filename
cd ${name}-${version}/${pkgdir}

grab_release pkginfo.mk
case $os in
    Linux) 
    grab_rpmroot pkginfo.mk
    cp ../../$filename $rpmroot/SOURCES
    arch=`uname -i`
    pkgfile=${name}-${version}-${release}.${arch}.rpm
    ;;
    *) pkgfile=${name}-${version}_${release}.tbz;;
esac

make package
cp ${pkgfile} ../..
cd ../..
rm -r ${filename} ${name}-${version}



