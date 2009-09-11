#!/bin/sh

for file in filters.c serverm.c pfilter.c pfilter.h
do
    cp sh-$file ../../src/$file
done
