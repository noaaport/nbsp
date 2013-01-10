#!/bin/sh

set -- `grep %dist /etc/rpm/macros.dist`
echo $2
