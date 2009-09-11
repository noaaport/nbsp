#!/bin/sh

. upload.conf

filename=${name}-${version}.tgz

[ ! -f $filename ] && { echo "$filename not found"; exit 0; }

ftp -n -v $uploadhost <<EOF
user $uploaduser
prompt
cd $uploaddir
mkdir $srcuploadsubdir
cd $srcuploadsubdir
mdelete ${name}-*.tgz
put $filename
quit
EOF


