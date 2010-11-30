/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
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

struct nids_radial_packet_st {
  int num_rle_halfwords;
  int angle_start;	/* in tenth of degree */
  int angle_delta;
  /* derived */
  double angle_start_deg;	/* in degrees */
  double angle_delta_deg;
  /* runs */
};

static int nids_decode_bref_codetolevel(int pdb_mode, int run_code);
static int nids_decode_rvel_codetolevel(int run_code);

void nids_decode_radials_af1f(struct nids_data_st *nd){

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
    fprintf(stdout, "num_rle_halfwords = %d %f %f\n",
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
      }else{
	log_errx(1, "Unsupported value of nd->nids_header.pdb_code.");
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
