#!/bin/sh

INSTALL=install

os=`uname`

case $os in
    FreeBSD) 
    	MAKEFILEINC=".include \"..\/..\/Makefile.inc\""
    ;;
    OpenBSD) 
	MAKEFILEINC=".include \"..\/..\/Makefile.inc\""
    ;;
    SunOS)
        MAKEFILEINC="include ..\/..\/Makefile.inc"
        INSTALL=ginstall
    ;;
    Linux)
	MAKEFILEINC="include ..\/..\/Makefile.inc"       
    ;;
esac

sed -e "/@MAKEFILEINC@/s//$MAKEFILEINC/" \
    -e "/@INSTALL@/s//$INSTALL/" Makefile.in > Makefile


