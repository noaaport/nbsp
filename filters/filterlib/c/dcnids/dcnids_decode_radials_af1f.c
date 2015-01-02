/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id: dcnids_decode_radials_af1f.c,v f4498a73fe59 2015/01/01 17:31:34 nieves $
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <math.h>
#include "err.h"
#include "util.h"
#include "dcnids.h"
#include "dcnids_extract.h"
#include "dcnids_decode.h"

/* used only in the nids_decode_radials_af1f_grided() */
#define DEG_PER_RAD     57.2173

struct nids_radial_packet_st {
  int num_rle_halfwords;
  int angle_start;	/* in tenth of degree */
  int angle_delta;
  /* derived */
  double angle_start_deg;	/* in degrees */
  double angle_delta_deg;
  /* runs */
};

static void nids_decode_radials_af1f_grided(struct nids_data_st *nd);
static void nids_count_polygons_radials_af1f(struct nids_data_st *nd);
static int nids_decode_bref_codetolevel(int pdb_mode, int run_code);
static int nids_decode_rvel_codetolevel(int run_code);
static int nids_decode_nxp_codetolevel(int run_code);

#if 0
void nids_decode_radials_af1f_orig(struct nids_data_st *nd){

  unsigned char *b = nd->data;
  unsigned char *bsave;		/* used when counting the polygons */
  struct dcnids_polygon_st *polygon;
  struct nids_radial_packet_st radial_packet;
  int i, j;
  int run_code, run_bins, total_bins;
  double r1, r2, theta1, theta2;
  double sin_theta1, cos_theta1, sin_theta2, cos_theta2;
  int numpoints = 0;
  int numpolygons = 0;
  int run_level = 0;	/* the level corresponding to a given code */

  /*
   * We will run through the radials twice. The first time is just to
   * count the number of polygons, and the second time to actually
   * fill in the polygons. In other words, we must compute the total
   * number of "run bins".
   */

  /*
   * We must go to the start of the "individual radials". In bytes:
   * symbologoy block => 10 
   * symbology layer =>  6
   * radial packet header => 14
   */
  b += NIDS_PACKET_RADIALS_START_RUNS;
  bsave = b;
  for(i = 0; i < nd->radial_packet_header.numradials; ++i){
    radial_packet.num_rle_halfwords = extract_uint16(b, 1);
    b += 6;
    b += radial_packet.num_rle_halfwords * 2;
    /*
     * This is an upper limit since the last byte may or may not have data.
     */
    numpolygons += radial_packet.num_rle_halfwords * 2;
  }
  b = bsave;

  /*
   * Allocate space for all the polygons.
   */
  nd->polygon_map.numpolygons = numpolygons;
  nd->polygon_map.polygons = malloc(sizeof(struct dcnids_polygon_st) *
				    nd->polygon_map.numpolygons);
  if(nd->polygon_map.polygons == NULL)
    log_err(1, "Error from malloc()");

  /* XXX 
  fprintf(stdout, "numpolygons = %d\n", nd->polygon_map.numpolygons);
  */

  /*
   * Go through the run bins again, this time to get the polygons.
   */

  /*
   * Initialize the polygon pointer to the start of the polygon array,
   * and count them again to check.
   */
  polygon = nd->polygon_map.polygons;
  numpolygons = 0;

  for(i = 0; i < nd->radial_packet_header.numradials; ++i){
    radial_packet.num_rle_halfwords = extract_uint16(b, 1);
    radial_packet.angle_start = extract_int16(b, 2);
    radial_packet.angle_delta = extract_uint16(b, 3);
    b += 6;

    radial_packet.angle_start_deg = (double)radial_packet.angle_start/10.0;
    radial_packet.angle_delta_deg = (double)radial_packet.angle_delta/10.0;

    /* XXX
    fprintf(stdout, "%d %f %f\n",
	    radial_packet.num_rle_halfwords,
	    radial_packet.angle_start_deg,
	    radial_packet.angle_delta_deg);
    */

    /* theta1 and theta2 in degrees */
    theta1 = radial_packet.angle_start_deg;
    theta2 = radial_packet.angle_start_deg + radial_packet.angle_delta_deg;
    dcnids_sine_cosine(theta1, &sin_theta1, &cos_theta1);
    dcnids_sine_cosine(theta2, &sin_theta2, &cos_theta2);

    total_bins = 0;
    for(j = 0; j < radial_packet.num_rle_halfwords * 2; ++j){
      run_code = (b[0] & 0xf);
      run_bins = (b[0] >> 4);
      ++b;      

      /*
       * The last byte may be empty (if there is an odd number of range bins).
       */
      if(run_bins == 0)
	continue;  /* should be the last byte and the loop will break itself */

      numpoints += run_bins;

      /* radius in km - also update "total_bins" */
      r1 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;
      total_bins += run_bins;
      r2 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;

      /* XXX
      fprintf(stdout, "\t%f %f\n", r1, r2);
      */

      /*
       * Translate the code to a "level". The level that corresponds to the
       * code depends on the product type (pdb_code) and if it is a bref,
       * then it depends code on the operational mode.
       */
      if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXR) ||
	 (nd->nids_header.pdb_code == NIDS_PDB_CODE_N0Z)){
	run_level = nids_decode_bref_codetolevel(nd->nids_header.pdb_mode,
						 run_code);
      } else if(nd->nids_header.pdb_code == NIDS_PDB_CODE_NXV){
	run_level = nids_decode_rvel_codetolevel(run_code);
      } else if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXP) || 
		(nd->nids_header.pdb_code == NIDS_PDB_CODE_NTP)){
	  run_level = nids_decode_nxp_codetolevel(run_code);
      } else {
	log_errx(1, "Unsupported value [%d] of nd->nids_header.pdb_code.",
		 nd->nids_header.pdb_code);
      }

      if(nd->polygon_map.flag_usefilter == 1){
	if((run_level < nd->polygon_map.level_min) ||
	   (run_level > nd->polygon_map.level_max)){
	  continue;
	}
      }
      polygon->code = run_code;
      polygon->level = run_level;

      /*
       * The reference lat, lon are the site coordinates
       */
      dcnids_define_polygon(nd->nids_header.lon,
			    nd->nids_header.lat,
			    r1, r2,
			    sin_theta1, cos_theta1, sin_theta2, cos_theta2,
			    polygon);

      /* XXX
      int k;
      for(k = 0; k < 4; ++k){
	fprintf(stdout, "%.2f:%.2f,", polygon->lon[k], polygon->lat[k]);
      }
      fprintf(stdout, "%d\n", polygon->level);
      */

      ++polygon;
      ++numpolygons;
    }

    /* XXX
    fprintf(stdout, "\ntotal_bins: %d\n", total_bins);
    fprintf(stdout, "\n");
    */
  }

  assert(numpolygons <= nd->polygon_map.numpolygons);
  nd->polygon_map.numpolygons = numpolygons;

  dcnids_polygonmap_bb(&nd->polygon_map);

  /* XXX
     fprintf(stdout, "\nnumpoints= %d, numpolygons = %d:%d\n",
     numpoints, numpolygons, nd->polygon_map.numpolygons);
  */
}
#endif

void nids_decode_radials_af1f(struct nids_data_st *nd){

  nids_decode_radials_af1f_grided(nd);
}

static void nids_decode_radials_af1f_grided(struct nids_data_st *nd){
  /*
   * If this function is used, and in particular if we make it the
   * default, then the -D option should be passed to nbspradgis in order
   * to discard the data beyond the thresholds and avoid unnecessary
   * processing and large shapefiles.
   */
  unsigned char *b = nd->data;
  struct dcnids_polygon_st *polygon;
  struct nids_radial_packet_st radial_packet;
  int i, j;
  int run_code, run_bins, total_bins;
  double r1, r2, theta1, theta2;
  double sin_theta1, cos_theta1, sin_theta2, cos_theta2;
  double r, dtheta;
  /* double theta1p, sin_theta1p, cos_theta1p; */
  double sin_theta1p, cos_theta1p;    /* theta1p not used */
  double theta2p, sin_theta2p, cos_theta2p;
  int numpolygons = 0;
  int run_level = 0;	/* the level corresponding to a given code */

  /*
   * We will run through the radials twice. The first time is just to
   * count the number of polygons, and the second time to actually
   * fill in the polygons. In other words, we must compute the total
   * number of "run bins".
   */
  nids_count_polygons_radials_af1f(nd);

  /*
   * We must go to the start of the "individual radials". In bytes:
   * symbologoy block => 10 
   * symbology layer =>  6
   * radial packet header => 14
   */
  b += NIDS_PACKET_RADIALS_START_RUNS;

  /*
   * Allocate space for all the polygons.
   */
  nd->polygon_map.polygons = malloc(sizeof(struct dcnids_polygon_st) *
				    nd->polygon_map.numpolygons);
  if(nd->polygon_map.polygons == NULL)
    log_err(1, "Error from malloc()");

  /* XXX 
     fprintf(stdout, "numpolygons = %d\n", nd->polygon_map.numpolygons);
  */

  /*
   * Go through the run bins again, this time to get the polygons.
   */

  /*
   * Initialize the polygon pointer to the start of the polygon array,
   * and count them again to check.
   */
  polygon = nd->polygon_map.polygons;
  numpolygons = 0;

  for(i = 0; i < nd->radial_packet_header.numradials; ++i){
    radial_packet.num_rle_halfwords = extract_uint16(b, 1);
    radial_packet.angle_start = extract_int16(b, 2);
    radial_packet.angle_delta = extract_uint16(b, 3);
    b += 6;

    radial_packet.angle_start_deg = (double)radial_packet.angle_start/10.0;
    radial_packet.angle_delta_deg = (double)radial_packet.angle_delta/10.0;

    /* XXX
    fprintf(stdout, "%d %f %f\n",
	    radial_packet.num_rle_halfwords,
	    radial_packet.angle_start_deg,
	    radial_packet.angle_delta_deg);
    */

    /* theta1 and theta2 in degrees */
    theta1 = radial_packet.angle_start_deg;
    theta2 = radial_packet.angle_start_deg + radial_packet.angle_delta_deg;
    dcnids_sine_cosine(theta1, &sin_theta1, &cos_theta1);
    dcnids_sine_cosine(theta2, &sin_theta2, &cos_theta2);

    total_bins = 0;
    for(j = 0; j < radial_packet.num_rle_halfwords * 2; ++j){
      run_code = (b[0] & 0xf);
      run_bins = (b[0] >> 4);
      ++b;      

      /*
       * The last byte may be empty (if there is an odd number of range bins).
       */
      if(run_bins == 0)
	continue;  /* should be the last byte and the loop will break itself */

      /* radius in km - also update "total_bins" */
      r1 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;
      total_bins += run_bins;
      r2 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;

      /* XXX
	 fprintf(stdout, "\t%f %f\n", r1, r2);
      */

      /*
       * Translate the code to a "level". The level that corresponds to the
       * code depends on the product type (pdb_code) and if it is a bref,
       * then it depends code on the operational mode.
       */
      if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXR) ||
	 (nd->nids_header.pdb_code == NIDS_PDB_CODE_N0Z)){
	run_level = nids_decode_bref_codetolevel(nd->nids_header.pdb_mode,
						 run_code);
      }else if(nd->nids_header.pdb_code == NIDS_PDB_CODE_NXV){
	run_level = nids_decode_rvel_codetolevel(run_code);
      } else if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXP) || 
		(nd->nids_header.pdb_code == NIDS_PDB_CODE_NTP)){
	  run_level = nids_decode_nxp_codetolevel(run_code);
      } else {
	log_errx(1, "Unsupported value [%d] of nd->nids_header.pdb_code.",
		 nd->nids_header.pdb_code);
      }

      if(nd->polygon_map.flag_usefilter == 1){
	if((run_level < nd->polygon_map.level_min) ||
	   (run_level > nd->polygon_map.level_max)){
	  continue;
	}
      }

      r = r1;
      while(r < r2){
	++r;

	/* XXX fprintf(stdout, "%f\n", r - 1); */

	dtheta = DEG_PER_RAD/r;
	if(radial_packet.angle_delta_deg < dtheta)
	  dtheta = radial_packet.angle_delta_deg;

	/* theta1p = theta1; */
	sin_theta1p = sin_theta1;
	cos_theta1p = cos_theta1;
	theta2p = theta1;

	while(theta2p < theta2){
	  theta2p += dtheta;
	  if(theta2p > theta2)
	    theta2p = theta2;

	  /* XXX fprintf(stdout, "%f\n", theta2p); */

	  dcnids_sine_cosine(theta2p, &sin_theta2p, &cos_theta2p);

	  /*
	   * The reference lat, lon are the site coordinates
	   */
	  dcnids_define_polygon(nd->nids_header.lon,
				nd->nids_header.lat,
				r - 1, r,
				sin_theta1p, cos_theta1p,
				sin_theta2p, cos_theta2p,
				polygon);
	  /* XXX
	     int k;
	     for(k = 0; k < 4; ++k){
	     fprintf(stdout, "%.2f:%.2f,", polygon->lon[k], polygon->lat[k]);
	     }
	     fprintf(stdout, "%d\n", polygon->level);
	  */

	  polygon->code = run_code;
	  polygon->level = run_level;

	  ++polygon;
	  ++numpolygons;

	  /* theta1p = theta2p; */
	  sin_theta1p = sin_theta2p;
	  cos_theta1p = cos_theta2p;
	}
      }
    }

  /*
   * XXX - regrid
   *
   * This is where the "regrid" function would be called to regrid
   * the data to a regular grid. The function can use the "totalbins"
   * as a measure how many points to use.
   *
   * dlat = (lat2 - lat1)/(2 * totalbins)
   * dlon = (lon2 - lon1)/(2 * totalbins)
   *
   * choose the smaller of the two as the "cell size" (call it dl),
   * and then compute the numebr of points in each axis (nlat, nlon)
   * for that dl. The rest would be similar to what is done in the sat case.
   */

    /* XXX
       fprintf(stdout, "\ntotal_bins: %d\n", total_bins);
       fprintf(stdout, "\n");
    */
  }

  assert(numpolygons <= nd->polygon_map.numpolygons);
  nd->polygon_map.numpolygons = numpolygons;

  dcnids_polygonmap_bb(&nd->polygon_map);

  /* XXX
  fprintf(stdout, "\nnumpoints= %d, numpolygons = %d:%d\n",
	  numpoints, numpolygons, nd->polygon_map.numpolygons);
  */
}

static void nids_count_polygons_radials_af1f(struct nids_data_st *nd){

  unsigned char *b = nd->data;
  struct nids_radial_packet_st radial_packet;
  int i, j;
  /* int run_code, run_bins, total_bins; */
  int run_bins, total_bins;  /* run_code not used in this function */
  double r1, r2, theta1, theta2;
  double r, theta, dtheta;
  int numpolygons = 0;

  /*
   * We will run through the radials twice. The first time is just to
   * count the number of polygons, and the second time to actually
   * fill in the polygons. In other words, we must compute the total
   * number of "run bins".
   */

  /*
   * We must go to the start of the "individual radials". In bytes:
   * symbologoy block => 10 
   * symbology layer =>  6
   * radial packet header => 14
   */
  b += NIDS_PACKET_RADIALS_START_RUNS;

  numpolygons = 0;
  for(i = 0; i < nd->radial_packet_header.numradials; ++i){
    radial_packet.num_rle_halfwords = extract_uint16(b, 1);
    radial_packet.angle_start = extract_int16(b, 2);
    radial_packet.angle_delta = extract_uint16(b, 3);
    b += 6;

    radial_packet.angle_start_deg = (double)radial_packet.angle_start/10.0;
    radial_packet.angle_delta_deg = (double)radial_packet.angle_delta/10.0;

    /* theta1 and theta2 in degrees */
    theta1 = radial_packet.angle_start_deg;
    theta2 = radial_packet.angle_start_deg + radial_packet.angle_delta_deg;

    total_bins = 0;
    for(j = 0; j < radial_packet.num_rle_halfwords * 2; ++j){
      /* run_code = (b[0] & 0xf); */
      run_bins = (b[0] >> 4);
      ++b;      

      /*
       * The last byte may be empty (if there is an odd number of range bins).
       */
      if(run_bins == 0)
	continue;  /* should be the last byte and the loop will break itself */

      /* radius in km - also update "total_bins" */
      r1 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;
      total_bins += run_bins;
      r2 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;

      r = r1;
      while(r < r2){
	++r;

	/*
	 * Choose dtheta such that it corresponds to an arc of 1.
	 *
	 * dtheta_rad = 1/r2
	 */
	dtheta = DEG_PER_RAD/r;

	if(radial_packet.angle_delta_deg < dtheta)
	  dtheta = radial_packet.angle_delta_deg;
	
	theta = theta1;
	while(theta < theta2){
	  theta += dtheta;
	  if(theta > theta2)
	    theta = theta2;

	  ++numpolygons;
	}
      }
    }
  }

  nd->polygon_map.numpolygons = numpolygons;
}

static int nids_decode_bref_codetolevel(int pdb_mode, int run_code){

  int run_level = NIDS_BREF_LEVEL_MIN_VAL;

  /*
   * The "level" that corresponds to the code depends on the operational
   * mode.
   */
  if(pdb_mode == NIDS_PDB_MODE_PRECIPITATION)
    run_level = run_code * 5;
  else if(pdb_mode == NIDS_PDB_MODE_CLEAR)
    run_level = (run_code * 4) - 32;
  else if(pdb_mode == NIDS_PDB_MODE_MAINTENANCE)
    log_errx(1, "Radar is in maintenance mode.");
  else
    log_errx(1, "Invalid value of radar operational mode.");

  return(run_level);
}

static int nids_decode_rvel_codetolevel(int run_code){

  int run_level = NIDS_RVEL_LEVEL_ND_MIN;

  switch(run_code){
  case 0:
    run_level = NIDS_RVEL_LEVEL_ND_MIN;
    break;
  case 1:
    run_level = -64;
    break;
  case 2:
    run_level = -50;
    break;
  case 3:
    run_level = -36;
    break;
  case 4:
    run_level = -26;
    break;
  case 5:
    run_level = -20;
    break;
  case 6:
    run_level = -10;
    break;
  case 7:
    run_level = -1;
    break;
  case 8:
    run_level = 0;
    break;
  case 9:
    run_level = 10;
    break;
  case 10:
    run_level = 20;
    break;
  case 11:
    run_level = 26;
    break;
  case 12:
    run_level = 36;
    break;
  case 13:
    run_level = 50;
    break;
  case 14:
    run_level = 64;
    break;
  case 15:
    run_level = NIDS_RVEL_LEVEL_ND_MAX;
    break;
  default:
    log_errx(1, "Invalid value of run_code.");
    break;
  }

  return(run_level);
}

static int nids_decode_nxp_codetolevel(int run_code){
  /*
   * The value of the level returned here is in hundredth of an inch. That is,
   * 200 is 2 in, and 5 is 0.05 inches. The correspondnce is taken from the
   * interpretation/meaning of the run_code, according to the code color table
   * in page 28-1 of 2620003T.pdf; for example in (2014-12-29)
   * 
   *   http://www.roc.noaa.gov/wsr88d/PublicDocs/ICDs/2620003T.pdf
   */
  int run_level = 0;

  switch(run_code){
  case 0:
    run_level = 0;
    break;
  case 1:
    run_level = 5;
    break;
  case 2:
    run_level = 10;
    break;
  case 3:
    run_level = 25;
    break;
  case 4:
    run_level = 50;
    break;
  case 5:
    run_level = 75;
    break;
  case 6:
    run_level = 100;
    break;
  case 7:
    run_level = 125;
    break;
  case 8:
    run_level = 150;
    break;
  case 9:
    run_level = 175;
    break;
  case 10:
    run_level = 200;
    break;
  case 11:
    run_level = 250;
    break;
  case 12:
    run_level = 300;
    break;
  case 13:
    run_level = 400;
    break;
  case 14:
    run_level = 600;
    break;
  case 15:
    run_level = 800;
    break;
  default:
    log_errx(1, "Invalid value of run_code.");
    break;
  }

  return(run_level);
}
