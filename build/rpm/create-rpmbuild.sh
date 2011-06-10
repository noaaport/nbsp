#!/bin/sh

topdir="rpmbuild"

for d in BUILD RPMS/i386 RPMS/i686 RPMS/noarch SOURCES SPECS SRPMS tmp
do
    mkdir -p $topdir/$d
done

echo "%_topdir $HOME/$topdir" > $HOME/.rpmmacros
echo "%_tmppath $HOME/$topdir/tmp" >> $HOME/.rpmmacros
