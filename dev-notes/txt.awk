#
# $Id$
#
# This was used like this
#
# (1) awk -f txt.awk text-list.txt | sort > tmp
#
# then
#
# (2) awk -f txt.awk tmp > text-list
#

$1 !~ /^#/ {
awips = tolower($1);
#print awips;			# for (1)
printf("%s%s", awips, "|");	# for (2)
}
