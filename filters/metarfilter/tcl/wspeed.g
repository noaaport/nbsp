#
# $Id$
#
# gnuplot template
#
set gnuplot(script) {

  set terminal $gplot(fmt) $gplot(fmtoptions)
  set output "$gplot(output)"

  set ylabel "Wind (mph)"
  set title "$gplot(STATION)\n$gplot(start) - $gplot(end)"

  set size 0.5,0.5
  set lmargin 8
  set bmargin 4

  # time series specification for x axis, such as
  # set xdata time
  # set timefmt "%d %H:%M"
  # set format x "%d\n%H"
  # set xtics 21600
  #
  # and ploting with "using 2:10" gives problems when the days in
  # the data file wrap from 31/30 back to 01.
  # The solution is to compute the xtics in the script that calls
  # this template.
  set xtics $gplot(xtics)

  plot \
      "$gplot(datafile)" using 4 notitle with linespoints ls 1

  quit
}
