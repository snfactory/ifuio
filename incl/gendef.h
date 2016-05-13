/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        gendef.h
! LANGUAGE     C
!
! AUTHOR       A.Rousset
!
! KEYWORDS     
! PURPOSE      variables generales a l'application
! COMMENT      
! VERSION      1.0  1990-Jan-03 : Creation,   AR 
!
---------------------------------------------------------------------*/

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <macro.h>
#ifdef local
#include <IFU_io_config.h>
#endif

#ifndef MINSHORT
#define MINSHORT        SHRT_MIN
#define MININT          INT_MIN
#define MINLONG         LONG_MIN
#define MINDOUBLE       DBL_MIN
#endif
#ifndef MINFLOAT
#define MINFLOAT        FLT_MIN
#endif
#ifndef MAXSHORT
#define MAXSHORT        SHRT_MAX
#define MAXINT          INT_MAX
#define MAXLONG         LONG_MAX
#define MAXDOUBLE       DBL_MAX
#endif
#ifndef MAXFLOAT
#define MAXFLOAT        FLT_MAX
#endif
#ifndef DMINEXP
#define DMINEXP         DBL_MIN_EXP
#define FMINEXP         FLT_MIN_EXP
#define DMAXEXP         DBL_MAX_EXP
#define FMAXEXP         FLT_MAX_EXP
#endif

#include <IFU_datatypes.h>

/* Ignore malloc.h if we have STDC_HEADERS */
#if defined(HAVE_MALLOC_H) && !defined(STDC_HEADERS)
#       include <malloc.h>
#endif

#include <error_codes.h>

#define lg_name        80L /* to be set to 256L, but need changes for tigerfiles */
#define lg_ident       72L
#define lg_unit        48L
#define lg_label       16L
#define lg_hist        1024L
#define lg_version     4L

                            /* default tables size */
#define tble_nb_row   450L  /* number of rows      */
#define tble_nb_col   256L  /* number of columns   */

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

		/* File types : */

#define T_IMA1D  's'      /* type for 1D image        */
#define T_IMA2D  'i'      /* type for 2D image        */
#define T_IMA3D  'c'      /* type for 3D image        */
#define T_TABLE  't'      /* type for table           */
#define T_TIGER  'T'      /* type for tiger data cube */
#define T_TIGMAX 'M'      /* type for tiger maxima    */
#define T_PRIMHD 'P'      /* type for primary header  */

/* flip flags */

#define LeftToRight 1
#define TopToBottom 2

/* global variables */

extern int DEBUG;
extern int VERBOSE;
extern int TK;
extern int ASK;
extern int ASK_BACK;

#include <fclass.h> /* File classes definitions */

/*        dynamic allocation debugger           */
#ifdef DEBUG
#include <dbmalloc.h>
#endif
