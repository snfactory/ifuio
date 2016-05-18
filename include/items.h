/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        items.h
! LANGUAGE     C
! 
! AUTHOR       A. Rousset
! 
! KEYWORDS     Structures definition 
! PURPOSE      
! COMMENT     
! VERSION      4.0  1992-June-15 : Creation    
!              4.1  2002-Oct-01 : Added quality flags
______________________________________________________________________________*/

/*____________________________________ SPECTRUM ______________________________*/

typedef struct 
{
    char   name[lg_name+1];  /* name of data frame            */
    int    imno;             /* image number                  */
    short  file_type;        /* type of file (1D frame)       */
    short  data_format;      /* data format                   */
    int    iomode;           /* I_MODE, O_MODE, IO_MODE       */
    int    nwcs;   
    struct wcsprm *wcs;      /* world coordinates system      */
    char   history[lg_hist+1];/* history                      */
    void   *external_info;   /* for FITS descriptors          */
    char   ident[lg_ident+1];/* identifier                    */
    char   cunit[lg_unit+1]; /* unit                          */
    int    npts;             /* length                        */
    double start;            /* start coordinates             */
    double end;              /* end of spectrum               */
    double step;             /* step size                     */
    double wstart;           /* window start	              */
    double wend;             /* window end                    */
    int    iwstart;          /* window pixel start            */
    int    iwend;            /* window pixel end              */
    double min;              /* min & max of intensity        */
    double max;       
    double wmin;             /* min & max of intensity        */
    double wmax;       
    short  data_type;        /* data_type                     */
    union {
      short  *s_data;        /* pointer to first data         */
      long   *l_data;        /* pointer to first data         */
      float  *f_data;        /* pointer to first data         */
      double *d_data;        /* pointer to first data         */
    } data;
    unsigned long *quality;  /* pointer to quality flags      */
    
} SPECTRUM;

#define IMAGE1D SPECTRUM

/*____________________________________ IMAGE 2D ______________________________*/

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
    int    nx;               /* size in x-dimension           */
    int    ny;               /* size in y-dimension           */
    double startx;           /* start coordinate in x         */
    double starty;           /* start coordinate in y         */
    double endx;    	     /* end coordinate in x           */
    double endy;             /* end coordinate in y           */
    double stepx;            /* step size in x                */
    double stepy;            /* step size in y                */
    double min;              /* min & max of intensity        */
    double max;       
    short  data_type;        /* data type                     */
    union {
      short  *s_data;        /* pointer to first data         */
      unsigned short  *us_data;  /* pointer to first data     */
      long   *l_data;        /* pointer to first data         */
      float  *f_data;        /* pointer to first data         */
      double *d_data;        /* pointer to first data         */
    } data;
    unsigned long *quality;  /* pointer to quality flags      */
} IMAGE2D;

/*____________________________________ IMAGE 3D ______________________________*/

typedef struct 
{
    char   name[lg_name+1];  /* name of data frame            */
    int    imno;             /* image number                  */
    short  file_type;        /* type of file (3D frame)       */
    short  data_format;      /* data format                   */
    int    iomode;           /* I_MODE, O_MODE, IO_MODE       */
    int    nwcs;   
    struct wcsprm *wcs;      /* world coordinates system      */
    char   history[lg_hist+1];/* history                      */
    void   *external_info;   /* for FITS descriptors          */
    char   ident[lg_ident+1];/* identifier                    */
    char   cunit[lg_unit+1]; /* unit                          */
    int    nx;               /* size in x-dimension           */
    int    ny;               /* size in y-dimension           */
    int    nz;               /* size in z-dimension           */
    double startx;           /* start coordinate in x         */
    double starty;           /* start coordinate in y         */
    double startz;           /* start coordinate in y         */
    double endx;    	     /* end coordinate in x           */
    double endy;             /* end coordinate in y           */
    double endz;             /* end coordinate in y           */
    double stepx;            /* step size in x                */
    double stepy;            /* step size in y                */
    double stepz;            /* step size in y                */
    double min;              /* min & max of intensity        */
    double max;
    short  data_type;        /* data type                     */
    union {
      short  *s_data;        /* pointer to first data         */
      long   *l_data;        /* pointer to first data         */
      float  *f_data;        /* pointer to first data         */
      double *d_data;        /* pointer to first data         */
    } data;
    unsigned long *quality;  /* pointer to quality flags      */

} IMAGE3D;

/*____________________________________ TABLE _________________________________*/

typedef struct
{
    char  name[lg_name+1];   /* name of table                 */
    int   imno;              /* table identifier              */
    short file_type;         /* type of file (table)          */
    short data_format;       /* data format                   */
    int   iomode;            /* I_MODE, O_MODE, IO_MODE       */
    int   nwcs;   
    struct wcsprm *wcs;      /* world coordinates system      */
    char  history[lg_hist+1];/* history                       */
    void  *external_info;    /* used for FITS descriptors     */
    char  ident[lg_ident+1]; /* identifier                    */
    int   col;               /* number of column              */
    int   row;               /* selected number of row        */
    int   allrow;            /* total number of row           */
    int   select_flag;	     /* selection flag TRUE=enable    */
                             /*                FALSE=not      */
    int   *sel_row;          /* line cross reference when     */
                             /* select_flag=TRUE              */
} TABLE;

/*____________________________________ Anyfile _______________________________*/

typedef struct 
{
    char  name[lg_name+1];   /* filename                      */
    int   imno;              /* file descriptor               */
    short file_type;         /* type of file (frame or table) */
    short data_format;       /* data format                   */
    int   iomode;            /* I_MODE, O_MODE, IO_MODE       */
    int   nwcs;   
    struct wcsprm *wcs;      /* world coordinates system      */
    char  history[lg_hist+1];/* history                       */
    void  *external_info;    /* used for FITS descriptors     */

} Anyfile;

/*__________________________________ Descriptors _____________________________*/

/* (only for FITS and TIGER formats) */

typedef struct            
{
    char   *descr_name;      /* name of descriptor            */
    short  data_type;        /* data type                     */
    int    nb_values;        /* number of values in array     */
    union {
      char   *c_data;        /* pointer to first data         */
      short  *s_data;        /* pointer to first data         */
      long   *l_data;        /* pointer to first data         */
      float  *f_data;        /* pointer to first data         */
      double *d_data;        /* pointer to first data         */
	} descr_value;

} Descriptor;

typedef struct            
{
    int nb_descr;             /* number of user defined descriptors    */ 
    Descriptor *descr_list;   /* list of currently defined descriptors */
	
} Descr_Items;

#include <iofuncdecl.h>
