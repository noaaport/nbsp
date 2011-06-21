#!/bin/sh

. ./configure.inc

sed \
    -e "/@include@/ s||$INCLUDE|" \
    -e "/@q@/ s||$Q|g" \
    -e "/@INSTALL@/ s||$INSTALL|" \
    -e "/@TCLSH@/ s||$TCLSH|" \
    -e "/@RCINIT@/s||$RCINIT|" \
    -e "/@RCFPATH@/s||$RCFPATH|" \
    -e "/@RCCONF@/s||$RCCONF|" \
    -e "/@CLEANUP@/s||$CLEANUP|" \
    -e "/@HOURLYCONF@/s||$HOURLYCONF|" \
    -e "/@STARTCLEANCONF@/s||$STARTCLEANCONF|" \
    -e "/@STARTSTOPRC@/s||$STARTSTOPRC|" \
    Makefile.in > Makefile

if [ `grep '#!/bin/sh' $POSTINSTALL_SRC` ]
then
    cat $POSTINSTALL_SRC > $POSTINSTALL_TARGET
else
    echo '#!/bin/sh' > $POSTINSTALL_TARGET
    cat $POSTINSTALL_SRC >> $POSTINSTALL_TARGET
fi
