#!/bin/sh

cd data/sat

for f in *gini
do
    name=`echo $f | cut -c 1-6`
    echo "asc: $name"
    nbspunz $f | nbspginishp -S -n $name
    echo "shp: $name"
    nbspunz $f | nbspginishp -n $name
done
