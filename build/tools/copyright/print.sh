#!/bin/sh

[ -f nbsp.ps ] && rm nbsp.ps

filelist="../../LICENSE"
filelist="$filelist `find ../../src -name \*.c -or -name \*.h -or -name \*.in -or -name \*.sh`"

enscript -b '$n|$V$%|Page $p' --toc -pnbsp.ps $filelist

