#!/bin/sh

#
# as user news
#
list=`cat newsgroups`

for g in $list
do
    /usr/local/news/bin/ctlinnd newgroup $g y
done
