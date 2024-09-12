#!/bin/sh

for f in *.tmpl
do
    #sed -f ch.sed $f > ${f}.output
    mv ${f}.output ../$f
done
