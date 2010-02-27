/*
 * Rainfall rate
 *
 * The output from this is:
 *
    5 0.00
    10 0.01
    15 0.01
    20 0.03
    25 0.05
    30 0.11
    35 0.22
    40 0.45
    45 0.93
    50 1.91
    55 3.93
    60 8.07
    65 16.58
    70 34.04
    75 69.91
*/
#include <stdio.h>
#include <math.h>

static double rainfall_rate(double dbz);

int main(void){

  double rr;
  double dbz;
  int i;

  for(i = 5; i <= 75; i += 5){
    dbz = (double)i;
    rr = rainfall_rate(dbz);
    fprintf(stdout, "%.0f\t%.2f\n", dbz, rr/25.4); /* convert from mm to in) */
  }

  return(0);
}

static double rainfall_rate(double dbz){
  /*
   * This gives the rr in (mm/h)
   *
   * rr = x^{0.65}
   * x = \frac{10^y}{200}
   * y = dbz/10
   */
  double rr;
  double x;
  double y;

  y = dbz/10.0;
  x = pow(10.0, y)/200.0;
  rr = pow(x, 0.625);
  
  return(rr);
}
