#
# $Id$
#
# gnuplot template
#

# The data file can be an hourly summary or the minutely data, and
# the gnuplot `set timefmt format' command must be chosen accordingly.

set firstline [string trim [exec head -n 1 $gplot(datafile)]];
if {[string range $firstline 2 2] eq ":"} {
  set gplot(_timefmt) {"%H:%M"};
  set gplot(_xformat) {"%H\n%M"};
  set gplot(_setboxwidth) "#";
} else {
  set gplot(_timefmt) {"%H"};
  set gplot(_xformat) {"%H"};
  set gplot(_setboxwidth) "set boxwidth 0.6 relative";
}

set gnuplot(script) {

  set terminal $gplot(fmt) $gplot(fmtoptions)
  set output "$gplot(output)"

  set ylabel "MBytes"
  set xlabel "Local Hour"
  set title "Throughput"

  set lmargin 12

  # time series specification for x axis.
  set xdata time
  set timefmt $gplot(_timefmt);
  set format x $gplot(_xformat)

  $gplot(_setboxwidth)
  set style fill solid 1.0

  plot \
      "$gplot(datafile)" using 1:7 notitle with boxes

  quit
}
