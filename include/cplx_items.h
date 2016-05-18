/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! IDENT        cplx_items.h
! LANGUAGE     C
! AUTHOR       P.FERRUIT
! KEYWORDS
! PURPOSE      Include file for the definition of the
!              complex structures.
!              Contains the definition of the complex image
!              structures and functions.
!
! COMMENT
! VERSION      1.0  2002, March, 27 PF Creation
! VERSION      1.1  2005, Feb, 10   AP renamed and included in IO lib
---------------------------------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk              EXTENSION NAMES DEFINITIONS
!
----------------------------------------------------------------------*/

#define CPLX_REAL_EXT "REAL_PART"
#define CPLX_IMAG_EXT "IMAGINARY_PART"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk              COMPLEX DATA STRUCTURES
!
----------------------------------------------------------------------*/

/* ----------------------------------------------------------------- */
/* COMPLEX IMAGE STRUCTURE                                           */
/* COMPLEX DATA (alpha + i * beta) WILL BE STORED THE FOLLOWING      */
/* WAY :                                                             */
/* complex[k][l] = real[k][l] + i * imag[k][l]                       */
/* with real[k][l] = data[2 *(l + ny * k)]                           */
/* and  imag[k][l] = data[1 + 2 *(l + ny * k)]                       */
/*                                                                   */
/* Total size of the array data is : 2 * nx * ny                     */
/* This storage is compatible with a bi-dimensional array of         */
/* fftw_complex or gsl_complex structures using a cast.              */
/* DATA ARE STORED IN DOUBLE FORMAT SYSTEMATICALLY                   */
/* ----------------------------------------------------------------- */

typedef struct          
{
    char   name[lg_name+1];  /* name of data frame            */
    int    imno;             /* image number                  */
    short  file_type;        /* type of file (2D frame)       */
    short  data_format;      /* data format                   */
    int    iomode;           /* I_MODE, O_MODE, IO_MODE       */
    int    nwcs;
    struct wcsprm *wcs;      /* world coordinates system      */
    char   history[lg_hist+1];/* history                      */
    void   *external_info;   /* for FITS descriptors          */
    char   ident[lg_ident+1];/* identifier                    */
    char   cunit[lg_unit+1]; /* unit                          */
                                  /* frame                    */
    int    nx;               /* size in x-dimension           */
    int    ny;               /* size in y-dimension           */
    double startx;           /* start coordinate in x         */
    double starty;           /* start coordinate in y         */
    double endx;    	     /* end coordinate in x           */
    double endy;             /* end coordinate in y           */
    double stepx;            /* step size in x                */
    double stepy;            /* step size in y                */
    double *data;            /* pointer to first data         */

} COMPLEX_IMAGE2D;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk              COMPLEX DATA ACCESS
!
----------------------------------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  RD_REAL_cplxima()
!       RD_IMAG_cplxima()
!
!.purp	Read the content of pixel (i,j)
!       of a complex image. 
!       These functions are coded as #define commands.
!
!.desc
!
!double RD_REAL_imacplx(COMPLEX_IMAGE2D *cplxima, int i, int j)
!double RD_IMAG_imacplx(COMPLEX_IMAGE2D *cplxima, int i, int j)
!
!COMPLEX_IMAGE2D   *cplxima;  pointer on a COMPLEX_IMAGE2D structure
!int            i;         pixel number (x-axis)
!int            j;         pixel number (y-axis)
!
!.ed
----------------------------------------------------------------------*/

#define RD_REAL_cplxima(cplxima,i,j) (((cplxima)->data)[2 *((j) + \
                    (i) * (cplxima)->ny)])
#define RD_IMAG_cplxima(cplxima,i,j) (((cplxima)->data)[1 + 2 * ((j) + \
                    (i) * (cplxima)->ny)])

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  WR_REAL_cplxima()
!       WR_IMAG_cplxima()
!
!.purp	Write values into a complex image.
!       These functions are coded as #define commands.
!
!.desc
!
!WR_REAL_imacplx(COMPLEX_IMAGE2D *frame, int i, int j, double value)
!WR_IMAG_imacplx(COMPLEX_IMAGE2D *frame, int i, int j, double value)
!
!COMPLEX_IMAGE2D   *frame;  pointer on a COMPLEX_IMAGE2D structure
!int            i;         pixel number (x-axis)
!int            j;         pixel number (y-axis)
!
!double         value;   real or imaginary part to be stored in
!                        pixel (i,j)
!.ed
----------------------------------------------------------------------*/

#define WR_REAL_cplxima(cplxima, i, j, value) \
        ((cplxima)->data)[2 *((j) + (i) * (cplxima)->ny)] = \
                 (value);
#define WR_IMAG_cplxima(cplxima, i, j, value) \
        ((cplxima)->data)[1 + 2 *((j) + (i) * (cplxima)->ny)] = \
                 (value);

#include <cplx_iofunc.h>
