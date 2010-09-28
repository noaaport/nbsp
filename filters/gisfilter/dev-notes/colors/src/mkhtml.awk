#
# $Id$
#
# awk -f <this script> <cvt output> > body.html
#
BEGIN {
  cell_width = "36";
}

! /^#/ {
  printf("<tr>\n");
  printf("  <td bgcolor=\"%s\" width=\"%s\"></td>\n", $4, cell_width);
  printf("  <td>%s</td>\n", $4);
  printf("  <td>%s %s %s</td>\n", $1, $2, $3);
  printf("</tr>\n");
}
