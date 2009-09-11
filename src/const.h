/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef CONST_H
#define CONST_H

#include "common.h"

/* 
 * When there is an awips line, the file name (fname) is of the form
 * <station>_<wmo_dataid>-<awips>. If there is no awips line, we use the
 * first 6 letters of the second line instead of the awips code,
 * and then use + as the separating char. If the second line is pure binary
 * the name is just <station>_<wmo_dataid>. The name of the file as saved
 * in the spool directory is  <fname>.<fseqkey> where <fseqkey> is a sequence
 * number used to identify different instances of the same product; we use
 * <ddhhmm>_<seq> where <ddhhmm> is the time stamp of the wmo header
 * and <seq> is the sequence number.
 */
#define FNAME_WMOID_SEP_CHAR	'_'
/* #define FNAME_AWIPS_SEP_CHAR	'-'  (common.h) */
#define FNAME_NOTAWIPS_SEP_CHAR	'+'
#define FNAME_FSEQKEY_SEPCHAR	'.'
#define FNAME_SEQNUM_SEPCHAR	'_'

/*
 * Sizes of the product_control_element fields (pctl.h)
 * These are the true lenghts. The declarations must leave room
 * for the terminating '\0'.
 */
#define FNAME_SIZE		18	/* KKKK_TTAAii-xxxxxx */
/* #define WMO_ID_SIZE		6	   TTAAii (common.h) */
#define WMO_STATION_SIZE	4	/* KKKK */
#define WMO_TIME_SIZE		6	/* ddhhmm */
#define WMO_AWIPS_SIZE		6	/* xxxxxx */
#define WMO_NOTAWIPS_SIZE	6	/* maximum */
#define FSEQNUM_SIZE		10	/* seqnum & ffffffff */
#define FSEQKEY_SIZE		17	/* ddhhmm_seqnum[10] */
#define FSEQNUM_SIZE		10
#define FBASENAME_SIZE		36	/* fname[18].fseqkey[17] */

/*
 * Format used to save the data files (the various formats differ only
 * in the way that the ccb header in the product file is handled.
 * (See save_frame() in nbsp.c)
 */
#define SAVEFORMAT_WITH_CCB	0    /* save ccb if present */
#define SAVEFORMAT_NO_CCB	1    /* strip ccb (and T1T2A1A2 of wmo) */

/*
 * The protocols supported.
 */
#define PROTOCOL_NBS1		1	/* send file content */
#define PROTOCOL_NBS2		2	/* send fpath; as for filters */
#define PROTOCOL_EMWIN		4
#define PROTOCOL_UNKNOWN	8	/* a client that has not identified */
#define PROTOCOL_DEFAULT	PROTOCOL_EMWIN

/* The id string that the client must send to specify what it wants */
#define PROTOCOL_NBS1_STR	"NBS1"
#define PROTOCOL_NBS2_STR	"NBS2"
#define PROTOCOL_EMWIN_STR	"EMWIN"

/*
 * Protocols served by the built-in server
 */
#define BUILTIN_SERVER_NONE 0
#define BUILTIN_SERVER_ALL  (PROTOCOL_NBS1 + PROTOCOL_NBS2 + PROTOCOL_EMWIN)

/*
 * spool methods - The functions in spooltype.h should be used and not
 * these constants directly.
 */
#define SPOOLTYPE_FS	1	/* flat file system based (fsspool) */
#define SPOOLTYPE_MBDB	2	/* memory based bdb (mspool) */
#define SPOOLTYPE_CBDB  3	/* file backed bdb spool (cspool) */
#define SPOOLTYPE_MCBDB  4	/* shared memory based cspool (mpool_nofile) */
#define SPOOLTYPE_MIN_VAL 1
#define SPOOLTYPE_MAX_VAL 4

/*
 * Logfiles that are meant to be read by other programs will be written
 * to a temporary file with this extension and then copied to the "real"
 * file using rename().
 */
#define TEMP_FILE_EXT	".tmp"

/*
 * Some constants cannot be determined at compile time, and they must
 * be computed at initialization. These functions are used for that.
 */
int const_fpath_maxsize(void);		/* framep.c */
int const_fpath_packet_maxsize(void);	/* pack.c */

#endif
