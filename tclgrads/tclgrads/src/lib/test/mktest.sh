#!/bin/sh

rm -rf test-output

for f in *.tcl
do
    name=${f%.tcl}
    echo "Executing $f ..."
    ./$f > ${name}.output
done

mkdir test-output
mv *.png *.output test-output

