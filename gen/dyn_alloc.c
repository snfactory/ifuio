/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        dyn_alloc.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     None
! PURPOSE      Facilities for memory allocation of 2D arrays
! VERSION      4.0  1992-June-12 : Creation, AR    
______________________________________________________________________________*/

#include <stdlib.h>
#include <stdio.h>
#include <IFU_datatypes.h>
#include <error_codes.h>

/*-----------------------------------------------------------------------------
!
!.blk            Dynamic allocation/free of bidimensional arrays
!
!.func                             alloc2d()
!
!.purp        dynamic allocation of a bidimensional array : tab[i][j]
!.desc
! int alloc2d(pt_tabl,nrow,ncol,datatype)
!
! (type) ***pt_tabl;     name of array   
! int nrow,ncol;         nb of rows and columns to allocate
! short datatype;        type of data (defined in alloc.h) 
!.ed
------------------------------------------------------------------------------*/

int
alloc2d(void ***pt_tabl, int nrow, int ncol, short datatype)
{
	int nc = ncol, nr = nrow;
	short **pt_short;
	long **pt_long;
	float **pt_float;
	double **pt_double;
	char **pt_char;
	long i;

	switch(datatype) {

	case CHAR :
		*(char ***)pt_tabl = (char **)malloc(nr*sizeof(char *));
		pt_char = *(char ***)pt_tabl;
		if (pt_char == NULL) return(ERR_ALLOC);
		pt_char[0] = (char *)malloc(nr*nc*sizeof(char));
		if (pt_char[0] == NULL) return(ERR_ALLOC);
		for (i=1;i<nr;i++)
			pt_char[i] = pt_char[i-1] + nc;
	break;	

	case SHORT :
		*(short ***)pt_tabl = (short **)malloc(nr*sizeof(short *));
		pt_short = *(short ***)pt_tabl;
		if (pt_short == NULL) return(ERR_ALLOC);
		pt_short[0] = (short *)malloc(nr*nc*sizeof(short));
		if (pt_short[0] == NULL) return(ERR_ALLOC);
		for (i=1;i<nr;i++)
			pt_short[i] = pt_short[i-1] + nc;
	break;	
             
	case INT :
	case LONG :
		*(long ***)pt_tabl = (long **)malloc(nr*sizeof(long *));
		pt_long = *(long ***)pt_tabl;
		if (pt_long == NULL) return(ERR_ALLOC);
		pt_long[0] = (long *)malloc(nr*nc*sizeof(long));
		if (pt_long[0] == NULL) return(ERR_ALLOC);
		for (i=1;i<nr;i++)
			pt_long[i] = pt_long[i-1] + nc;
	break;	
            
	case FLOAT :
		*(float ***)pt_tabl = (float **)malloc(nr*sizeof(float *));
		pt_float = *(float ***)pt_tabl;
		if (pt_float == NULL) return(ERR_ALLOC);
		pt_float[0] = (float *)malloc(nr*nc*sizeof(float));
		if (pt_float[0] == NULL) return(ERR_ALLOC);
		for (i=1;i<nr;i++)
			pt_float[i] = pt_float[i-1] + nc;
	break;	
           
	case DOUBLE :
		*(double ***)pt_tabl = (double **)malloc(nr*sizeof(double *));
		pt_double = *(double ***)pt_tabl;
		if (pt_double == NULL) return(ERR_ALLOC);
		pt_double[0] = (double *)malloc(nr*nc*sizeof(double));
		if (pt_double[0] == NULL) return(ERR_ALLOC);
		for (i=1;i<nr;i++)
			pt_double[i] = pt_double[i-1] + nc;
	break;	

	default :
		return(ERR_BAD_TYPE);           /* unkwnown data type */

	}
	return(0);
}

/*------------------------------------------------------------------------------
!
!.func                         free2d()
!
!.purp          to free previous 2D allocated array (see alloc2d())
!.desc
! int free2d(pt_tabl,datatype)
!
! (type) ***pt_tabl;    name of array              
! short datatype;       type of data
!.ed
------------------------------------------------------------------------------*/

int 
free2d(void ***pt_tabl, short datatype)

{
	short **pt_short;
	long **pt_long;
	float **pt_float;
	double **pt_double;
	char **pt_char;

	switch(datatype) {
                    
	case CHAR :
		pt_char = *(char ***)pt_tabl;
		if (pt_char == NULL) return(ERR_FREE);
		if (pt_char[0] == NULL) return(ERR_FREE);
		free((char *)pt_char[0]);
		free((char *)pt_char);
	break;	
                           
	case SHORT :
		pt_short = *(short ***)pt_tabl;
		if (pt_short == NULL) return(ERR_FREE);
		if (pt_short[0] == NULL) return(ERR_FREE);
		free((char *)pt_short[0]);
		free((char *)pt_short);
	break;	
                          
	case INT :
	case LONG :
		pt_long = *(long ***)pt_tabl;
		if (pt_long == NULL) return(ERR_FREE);
		if (pt_long[0] == NULL) return(ERR_FREE);
		free((char *)pt_long[0]);
		free((char *)pt_long);
	break;	
         
	case FLOAT :
		pt_float = *(float ***)pt_tabl;
		if (pt_float == NULL) return(ERR_FREE);
		if (pt_float[0] == NULL) return(ERR_FREE);
		free((char *)pt_float[0]);
		free((char *)pt_float);
	break;	
        
	case DOUBLE :
		pt_double = *(double ***)pt_tabl;
		if (pt_double == NULL) return(ERR_FREE);
		if (pt_double[0] == NULL) return(ERR_FREE);
		free((char *)pt_double[0]);
		free((char *)pt_double);
	break;	

	default :
		return(ERR_BAD_TYPE);                /* unkwnown data type */

	}
	return(0);
}

