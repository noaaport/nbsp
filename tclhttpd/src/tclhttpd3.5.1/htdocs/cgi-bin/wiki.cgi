#!/bin/sh
#
# Script to invoke the tclkit wiki
#
# This is a sample that won't work without obvious modifications
# to make it match your own wiki.  For info about the tclkit-based wiki
# called "wikit", go to http://wiki.tcl.tk/wikit/

cd ~welch/kit
WIKIT_BASE=http://medlicott.panasas.com:8015/wiki
export WIKIT_BASE

echo $SCRIPT_NAME > /tmp/wiki.log


echo HTTP/1.0 200 ok
if [ ! -f /usr10/home/welch/kit/tclkit-linux-i686 ]; then
  echo "wiki.cgi not configured"
  exit 0
fi
exec /usr10/home/welch/kit/tclkit-linux-i686 \
        /usr10/home/welch/kit/wikit.kit  /usr10/home/welch/kit/panasas.tkd
