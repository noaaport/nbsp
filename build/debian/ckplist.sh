#!/bin/sh
#
# $Id$
#

#
# config
#
plist_file="plist"
deb_plist_file=deb_plist"

#
# main
#
[ $# -eq 0 ] && { echo "No pkg name."; exit 1; }
pkgfile=$1

# Get the list of packaged files
dpkg -c $pkgfile | awk '{print substr($6,2)}' > $deb_plist_file

# Run through every file in the plist and check for each if it is installed
echo ""
echo "Checking plist ..."
echo ""
#
for f in `cat $plist_file`
do
  grep -q "^$f\$" $deb_plist_file || echo "Not found: $f"
done

rm $deb_plist_file
