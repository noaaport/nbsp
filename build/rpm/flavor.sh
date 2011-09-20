#!/bin/sh
#
# $Id$

if [ -f /etc/fedora-release ]
then
   flavor=fedoracore
elif [ -f /etc/SuSE-release ]
then
   flavor=opensuse
elif [ -f /etc/redhat-release ]
then
    flavor=redhat
elif [ -f /etc/debian_version ] 
then
   flavor=debian
else
   flavor=unknown
fi

echo $flavor
