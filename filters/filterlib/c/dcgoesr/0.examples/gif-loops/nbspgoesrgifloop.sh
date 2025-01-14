#!/bin/sh
#
# The files seem to come in two sets (two different "tiles"). One set
# at minutes 01, 11, 21, ..., and the other at
# 06, 16, 21, ...
#

for file in *18?1.goesr
do
    echo ${file}
    nbspgoesr ${file} | pngtopnm | ppmtogif > ${file}.gif
done

gifsicle -d 50 -l -O2 *.gif > loop.gif.tmp
mv loop.gif.tmp loop.gif
