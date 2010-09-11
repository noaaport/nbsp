#!/bin/sh
#
# $Id$
#
. upload.conf

# Create the index
echo $updateindexstring > $updateindexfile

lftp -c "\
$lftpoptions;
open -u $uploaduser $uploadhost;
cd $uploadbasedir;
mkdir -p $pkgdir;
put -O $pkgdir $pkglocaldir/$pkgfilename;
put -O $updateindexdir $updateindexfile;
quit"

# rm -f $updateindexfile
