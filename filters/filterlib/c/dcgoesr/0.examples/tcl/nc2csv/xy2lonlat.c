/*
 * Based on the xy2lonlat-example.
 * The program takes as input (on stdin) the x,y values (in radians)
 * and outputs the corresponding lon,lat values (in radians). The
 * default is for the east satellite (longitude_of_projection_origin = -75.0)
 * but that parameter can be sperfied through the [-l] option.
 *
 * The input lines must contain the values of x,y and the observed parameter,
 * separated by a comma, e.g.,
 *   x,y,r
 *
 * The program converts x and y to lon and lat, and emits the trio
 * lon, lat and r , also comma separated.
 *
 * Usage: xy2lonlat -l lon_origin < x y r
 */
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
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

static void xy2lonlat(double x, double y, double *lon, double *lat,
		      double lorigin);

static void xy2lonlat(double x, double y, double *lon, double *lat,
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

  /* Example:
  fprintf(stdout, "a: %f\n", a);
  fprintf(stdout, "b: %f\n", b);
  fprintf(stdout, "c: %f\n", c);
  fprintf(stdout, "rs: %f\n", rs);
  fprintf(stdout, "sx: %f\n", sx);
  fprintf(stdout, "sy: %f\n", sy);
  fprintf(stdout, "sz: %f\n", sz);
  fprintf(stdout, "lambda0: %f\n", lambda0);
  */
}

int main(int argc, char **argv) {

  char *usage = "Usage: xy2lonlat -l lon_origin < x y r";
  char *optstr = "l:";
  double x, y, r;	/* stdin parameters */
  double lorigin;	/* [-l] */
  double lon, lat;	/* calculated output, in radians */
  /* double londeg, latdeg; in degrees */
  int status = 0;
  int c;
  int N = 80;
  char line[80];

  /* default */
  lorigin = LON_ORIGIN_EAST;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'l':
      if(sscanf(optarg, "%lf", &lorigin) != 1) {
	errx(1, "Error scaning [-l] option.");
      }
      break;
    default:
      fprintf(stderr, "%s\n", usage);
      exit(1);
      break;
    }
  }
  
  /* Example:
   * x(1539) = -0.024052 rad
   * y(558) = 0.095340 rad
   *
   * londeg = lon *180.0/M_PI;
   * latdeg = lat *180.0/M_PI;
   * fprintf(stdout, "%f %f %f %f %f %f\n", x, y, lon, lat, londeg, latdeg);
   */

  /* In freebsd we can use gets_s(line, N) but not in debian (yet) */
  while(fgets(line, N, stdin) != NULL) {
    if(sscanf(line, "%lf, %lf, %lf", &x, &y, &r) == 3) {
      xy2lonlat(x, y, &lon, &lat, lorigin);
      fprintf(stdout, "%f,%f,%f\n", lon, lat, r);
    } else {
      status = 1;
      break;
    }
  }
    
  return(status);
}
