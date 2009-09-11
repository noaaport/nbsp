#!/bin/sh

# radprodlist="n0r n0s n0v n0z \
#    n1p n1r n1s n1v \
#    n2r n2s n2v \
#    n3p n3r n3s n3v \
#    nco ncr ncz net nhl nla nll nml ntp nup nvl nvw"
#

radprodlist="n0r n0s n0v"
#
# The radar site groups are
#
#	noaaport.rad.img.sss
#	noaaport.rad.raw.sss

if [ $# -eq 1 ]
then
    site=$1
    file=
elif [ $# -eq 2 ]
then
    site=$1
    file=$2
else
    echo "One site as argument."
    exit 1
fi

subgrouplist="rad.img.$site \
rad.raw.$site"

if [ -z "$file" ]
then
    cd
    for g in $subgrouplist
    do
      for p in $radprodlist
      do
	echo -n "noaaport.$g.$p: "
	bin/ctlinnd newgroup noaaport.$g.$p y
      done
    done
else
    for g in $subgrouplist
    do
      for p in $radprodlist
      do
	echo "noaaport.$g.$p" >> $file
      done
    done
fi
