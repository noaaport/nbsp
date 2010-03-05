#
# $Id$
#
# awk -f <this script> n0r.colors.txt > n0r.colors.html
#
BEGIN {
  cell_width = "36";
}

! /^#/ {
  printf("<tr>\n");
  printf("  <td bgcolor=\"%s\" width=\"%s\"></td>\n", $3, cell_width);
  printf("  <td>%s</td>\n", $1);
  printf("  <td>%s</td>\n", $2);
  printf("</tr>\n");
}
