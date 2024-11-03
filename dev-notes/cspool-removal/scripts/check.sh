#!/bin/sh

# filterlib_cspool_nbspfile
# query_enable
# background_processing
# cspoolbdb

fdir="nbsp/filters"

for f in `find $fdir -type f`
do
    grep -l cspool $f
done

