/*
 * GS-Series-416-R-PUG-L2-Plus-0349-Vol5-v2.4.pdf (pag 26)
 *
 * Example:
 * x(1539) = -0.024052 rad
 * y(558) = 0.095340 rad
 *
 * See also
 * https://www.star.nesdis.noaa.gov/\
 * atmospheric-composition-training/satellite_data_goes_imager_projection.php
 */
#include <stdio.h>
#include <math.h>

/* constants */
#define SEMI_MAJOR 6378137.0
#define SEMI_MINOR 6356752.31414
#define PERSPECTIVE_POINT_HEIGHT 35786023.0
/*
 * These are here only for documentation - they must be passed to the function,
 * in case they change.
 */
#define LON_ORIGIN_EAST -75.0
#define LON_ORIGIN_WEST -137.0

void xy2lonlat(double x, double y, double *lon, double *lat, double lorigin) {
  /*
   * The parameters x,y,lon,lat in the argument are in radians.
   * The longitude_of_projection_origin(lorigin) is in degrees.
   */
  double H = PERSPECTIVE_POINT_HEIGHT + SEMI_MAJOR;
  double req = SEMI_MAJOR;
  double rpol = SEMI_MINOR;
  double lambda0;
  double a, b, c, rs, sx, sy, sz;

  lambda0 = (lorigin * M_PI)/180.0;

  a = pow(sin(x), 2.0) +
    pow(cos(x), 2.0) * (pow(cos(y), 2.0) + pow((req/rpol)*sin(y), 2.0));
  b = -2.0*H*cos(x)*cos(y);
  c = pow(H, 2.0) - pow(req, 2.0);
  rs = -(b + sqrt(b*b - 4*a*c))/(2.0*a);
  sx = rs*cos(x)*cos(y);
  sy = -rs*sin(x);
  sz = rs*cos(x)*sin(y);
    
  *lat = atan((req/rpol)*(req/rpol) * sz/sqrt((H - sx)*(H - sx) + sy*sy));
  *lon = lambda0 - atan(sy/(H - sx));

  /* Example: */
  fprintf(stdout, "a: %f\n", a);
  fprintf(stdout, "b: %f\n", b);
  fprintf(stdout, "c: %f\n", c);
  fprintf(stdout, "rs: %f\n", rs);
  fprintf(stdout, "sx: %f\n", sx);
  fprintf(stdout, "sy: %f\n", sy);
  fprintf(stdout, "sz: %f\n", sz);
  fprintf(stdout, "lambda0: %f\n", lambda0);
}

int main(void) {

  double x,y;
  double lon, lat;
  double londeg, latdeg;

  /* x,y in radians */
  x = -0.024052;
  y = 0.095340;

  xy2lonlat(x, y, &lon, &lat, -75.0);
  londeg = lon *180.0/M_PI;
  latdeg = lat *180.0/M_PI;

  fprintf(stdout, "%f %f %f %f %f %f\n", x, y, lon, lat, londeg, latdeg);

  return(0);
}
