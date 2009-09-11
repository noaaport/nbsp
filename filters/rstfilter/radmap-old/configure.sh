#!/bin/sh

config_subdirs=

# defaults (FreeBSD)
INSTALL=install
MAKEFILEINC=".include \"../../../Makefile.inc\""

os=`uname`

case $os in
    *BSD) 
        # use the defaults
	;;
    SunOS)
	INSTALL=ginstall
	MAKEFILEINC="include ../../../Makefile.inc"
	;;
    Linux)       
	MAKEFILEINC="include ../../../Makefile.inc"
	;;
esac

sed -e "/@MAKEFILEINC@/s##$MAKEFILEINC#" \
    -e "/@INSTALL@/s##$INSTALL#" Makefile.in > Makefile

for d in $config_subdirs
do
  cd $d
  ./configure.sh
  cd ..
done



