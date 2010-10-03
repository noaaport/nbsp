#!/bin/sh

cd ..
rm -rf res
mkdir res

for file in *.conf
do
    name=${file%.conf}
    for res in 1 3 5
    do
 	sed -e "/map_rad.tmpl/ s//map_rad_${res}.tmpl/" \
	$file > res/${name}_${res}.conf
    done
done
