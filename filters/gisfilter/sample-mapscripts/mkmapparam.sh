#!/bin/sh

for name in tige01 tige02 tigw01 tigw02 tigp02 tigq02
do
    awk -f mkmapparam.awk -v name=$name data/sat/${name}.asc
done
