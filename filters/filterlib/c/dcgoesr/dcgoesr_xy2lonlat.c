/*
 * Based on the xy2lonlat-example.
 */
/* #include <stdio.h> for test */
#include <stdlib.h>
#include <math.h>
#include "dcgoesr_xy2lonlat.h"

/* constants */
#define SEMI_MAJOR 6378137.0
#define SEMI_MINOR 6356752.31414
#define PERSPECTIVE_POINT_HEIGHT 35786023.0
/*
 * These are here only for documentation - they must be passed to the function,
 * in case they change.
 * #define LON_ORIGIN_EAST -75.0
 * #define LON_ORIGIN_WEST -137.0
 */

/* convenience constants */
#define RAD_PER_DEG     0.017453		/* pi/180 */
#define DEG_PER_RAD	57.295780		/* 180/pi */

void xy2lonlat(double x, double y, double *lon, double *lat,
		      double lorigin) {
  /*
   * The parameters x,y,lon,lat in the argument are in radians.
   * The longitude_of_projection_origin(lorigin) is in degrees.
   */
  double H = PERSPECTIVE_POINT_HEIGHT + SEMI_MAJOR;
  double req = SEMI_MAJOR;
  double rpol = SEMI_MINOR;
  double lambda0;
  double a, b, c, rs, sx, sy, sz;

  /* convert to radians */
  lambda0 = lorigin*RAD_PER_DEG;

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
}

void lonlat2xy(double lon, double lat, double *x, double *y, 
	       double lorigin) {
  /*
   * The parameters x,y,lon,lat in the argument are in radians.
   * The longitude_of_projection_origin(lorigin) is in degrees.
   */
  double H = PERSPECTIVE_POINT_HEIGHT + SEMI_MAJOR;
  double req = SEMI_MAJOR;
  double rpol = SEMI_MINOR;
  double e = 0.0818191910435;
  double lambda0;
  double phic, rc, sx, sy, sz;

  lambda0 = (lorigin * M_PI)/180.0;
  
  phic = atan(pow(rpol/req, 2.0)*tan(lat));
  rc = rpol/sqrt(1.0 - pow(e*cos(phic), 2.0));

  sx = H - rc*cos(phic)*cos(lon - lambda0);
  sy = -rc*cos(phic)*sin(lon - lambda0);
  sz = rc*sin(phic);

  if(H*(H - sx) < pow(sy, 2.0) + pow((req/rpol)*sz, 2.0)) {
    /*
     * the location is not visible from the satellite
     */
    *x = 0.0;
    *y = 0.0;
  } else {  
    *x = asin(-sy/sqrt(pow(sx, 2.0) + pow(sy, 2.0) + pow(sz, 2.0)));
    *y = atan(sz/sx);
  }
}

/*
 * test
 *
int main(void) {

  double x, y, lon, lat;

  x = -0.024052;
  y = 0.095340;

  xy2lonlat(x, y, &lon, &lat, -75.0);
  fprintf(stdout, "%f %f --> %f %f\n", x, y, lon, lat);

  lon = -1.478136;
  lat = 0.590727;

  lonlat2xy(lon, lat, &x, &y, -75.0);
  fprintf(stdout, "%f %f --> %f %f\n", lon, lat, x, y);

  return(0);
}
*
* test
*/
