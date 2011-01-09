/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_PROJECTIONS_H
#define DCGINI_PROJECTIONS_H

/* polar str */
struct nesdis_proj_str_st {
  double s;
  double lov_rad;	/* this is a copy from the pdb */
  double alpha;		/* auxiliary parameter in the formulas */
  double x1;
  double y1;
  double dx;
  double dy;
};

/* llc */
struct nesdis_proj_llc_st {
  double s;
  double lov_rad;	/* this is a copy from the pdb */
  double cos_psi;
  double r_E;
  double alpha;
  double x1;
  double y1;
  double dx;
  double dy;
};

/* mercator */
struct nesdis_proj_mer_st {
  double lon0_rad;	/* center longitude */
  double x1;
  double y1;
  double x2;
  double y2;
  double dx;
  double dy;
};

#endif
