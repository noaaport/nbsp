#!/bin/sh

. ../../../configure.inc

for f in util.c util.h err.c err.h
do
  cp ../../../filters/filterlib/c/$f .
done

configure_default
