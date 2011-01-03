#!/bin/sh
#
# $Id$
#
. ./upload.conf

filename=${name}-${version}.tgz

[ ! -f $filename ] && { echo "$filename not found"; exit 0; }

lftp -c "\
$lftpoptions;
open -u $uploaduser $uploadhost;
cd $uploadbasedir;
mkdir -p $srcdir;
cd $srcdir;
mrm ${name}-*.tgz
put $filename
quit"
