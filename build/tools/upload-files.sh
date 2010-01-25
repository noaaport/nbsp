#!/bin/sh
#
# $Id$
#
. upload.conf

cp ../../QUICK_START/* ../../RELEASE_NOTES ../../UPGRADING \
    ../../conf/CONFIGURING ../../READMEs/*.README .
upload_files="QUICK* RELEASE_NOTES UPGRADING CONFIGURING *.README"

release_file=RELEASE_NOTES
dt=`date +%d%b%G`

lftp -c "\
$lftpoptions;
open -u $uploaduser $uploadhost;
cd $uploadbasedir;
mkdir -p $filesdir;
cd $filesdir;
mrm *
mput $upload_files
mv $release_file $release_file-$dt
quit"

rm $upload_files
