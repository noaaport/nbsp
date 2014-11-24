#!/bin/sh

topdir="$HOME/docs/devel/rpmbuild"

#
# for d in BUILD RPMS/i386 RPMS/i686 RPMS/noarch SOURCES SPECS SRPMS tmp
# do
#    mkdir -p $topdir/$d
# done
#

# yum install rpmdevtools

# echo "%_topdir $topdir" > $HOME/.rpmmacros
# echo "%_tmppath $topdir/tmp" >> $HOME/.rpmmacros

rpmdev-setuptree

