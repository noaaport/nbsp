#!/bin/sh

# set -- `grep %dist /etc/rpm/macros.dist`
# echo $2

set -- `rpm --eval "%{dist}"`
echo $1
