/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        macro.h
! LANGUAGE     C
! AUTHOR       various
! KEYWORDS     
! PURPOSE      General definitions for usual functions
! COMMENT      
! VERSION      4.0  1992-Month-Day : Creation
!
------------------------------------------------------------------------*/
#include <math.h>
#include <strings.h>

#define stringify(x) #x
#define really_stringify(x) stringify(x)

#define CRAD (M_PI/180.)

#define MAX(x,y)   ((x) >= (y) ?  (x) : (y)) 	/* maximum of 2 values */
#define MIN(x,y)   ((x) <= (y) ?  (x) : (y)) 	/* minimum of 2 values */
#define ABS(x)     ((x) <   0  ? (-(x)) : (x)) 	/* absolute value */
												/* nearest integer of x */
#ifndef NINT
	#define NINT(x)    ((x) <   0  ? ((long)(x-0.5)) : ((long)(x+0.5))) 
#endif
#define SQ(x)      ((x) * (x))					/* square of x */

#define is_num(car) (('0' <= car && car <= '9') ? 1 : 0)
#define is_alphanum(car) (('0' >= car || car >= '9') ? 1 : 0)

#define is_true(val) (strcmp(val,"true") == 0 ? 1 : 0)
#define is_false(val) (strcmp(val,"false") == 0 ? 1 : 0)
#define is_set(val) ((strcmp(val,"NULL") != 0) && (strcmp(val,"null") != 0) ? 1 : 0)
#define is_unset(val) ((strcmp(val,"NULL") == 0) || (strcmp(val,"null") == 0) ? 1 : 0)

#define RD_varlist(strg,nbargs) va_list vlist; va_start(vlist); \
nbargs = 0; strg[nbargs] = (char *)va_arg(vlist,char *); \
while (strg[nbargs] != (char *)NULL) { nbargs++; \
strg[nbargs] = (char *)va_arg(vlist, char *);} \
va_end(vlist);
