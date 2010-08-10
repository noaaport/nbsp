#!/bin/sh
#
# pack the working directory for building in other machines
#
name=nbsp

cd ../../..
tar czf ~/${name}.tgz --exclude=${name}/dev-notes ${name}
