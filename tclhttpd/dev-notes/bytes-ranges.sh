#!/bin/sh

curl -D header.txt -H "Range: bytes=1-10" \
    http://www.opennoaaport.net:8015/digatmos/warnings/warnings_20120401_23.txt
