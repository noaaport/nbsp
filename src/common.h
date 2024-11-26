/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef COMMON_H
#define COMMON_H

/*
 * This file contains definitions of constants that used to be in
 * various files (e.g., const.h) but which are also used by the
 * programs in filterlib/c. The relevant explanations of the
 * definitions still appear in the original files, but the definitions
 * there have been commented out.
 *
 * The make file in filterlib/c creates a copy of this file
 * during compilation. The master copy of this file is src/common.h
 * and any editing should be done there.
 */
#define FNAME_AWIPS_SEP_CHAR	'-'	/* src/const.h */
#define WMO_ID_SIZE		6	/* src/const.h */
#define WMO_AWIPS_SIZE          6       /* src/const.h */
#define MAX_FRDATA_SIZE		5200	/* src/sbn.h */
#define CCB_SIZE		24	/* src/sbn.h */
#define CTRLHDR_WMO_SIZE	21	/* src/sbn.h */

/* There are not used by nbspd and were added for nbspmcast, nbspfile (2024) */
#define CCB0 64			/* value of byte[0] of the ccb */
#define CCB1 12			/* value of byte[1] of the ccb */

#endif
