#!/bin/sh

# This script creates the directory hierarchy
# based on $root

root=/home/www/pub/software

# get $name and $version
. ../VERSION

base=${name}-${branch}

dirs="src
packages/freebsd-6.2-i386
packages/freebsd-6.2-amd64
packages/fedoracore-8-i386
packages/fedoracore-8-x86_64"

cd $root
install -d -m 755 $base

cd $base
for d in $dirs
do
     install -d -m 755 $d
done
