#!/bin/sh

for f in `cat filterlib_init.txt`
do
    #mv $f ${f}.orig
    #sed -e "/filterlib_init/d" -e "/filterlib_end/d" ${f}.orig > $f
    rm ${f}.orig
done

