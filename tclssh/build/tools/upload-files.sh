#!/bin/sh

. upload.conf

cp ../../LICENSE ../../RELEASE_NOTES ../../doc/*.README .
upload_files="LICENSE RELEASE_NOTES *.README"

release_file=RELEASE_NOTES
dt=`date +%d%b%G`

ftp -n -v $uploadhost <<EOF
user $uploaduser
prompt
mkdir $uploaddir
cd $uploaddir
mkdir Docs
cd Docs
mdelete *
mput $upload_files
rename $release_file $release_file-$dt
quit
EOF

rm $upload_files
