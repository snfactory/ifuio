/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  2005 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        cplx_iofunc.h
! LANGUAGE     C
! AUTHOR       A.Pecontal
! KEYWORDS     
! PURPOSE      Functions declarations for usual complex i/o
! COMMENT      
! VERSION      1.0  2005-Feb-10 : Creation,   AP 
!---------------------------------------------------------------------*/

#define CreateComplexImage create_COMPLEX_IMAGE2D
#define AllocComplexImageMemory alloc_COMPLEX_IMAGE2D_mem
#define FreeComplexImage free_COMPLEX_IMAGE2D_mem
#define CloseComplexImage close_COMPLEX_IMAGE2D
#define WriteComplexImageToDisk save_COMPLEX_IMAGE2D
#define OpenComplexImage open_COMPLEX_IMAGE2D

int alloc_COMPLEX_IMAGE2D_mem(COMPLEX_IMAGE2D *);
int free_COMPLEX_IMAGE2D_mem(COMPLEX_IMAGE2D *);
int create_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *, char *, int *, double *,
                        double *, char *, char *);
int close_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *);
int save_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *);
int open_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *, char *, char *);
