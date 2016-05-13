/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        3D_items.h
! LANGUAGE     C
! 
! AUTHOR       A. Pecontal
! 
! KEYWORDS     Structures definition 
! PURPOSE      
! COMMENT     
! VERSION      4.0  1994-Jul-21 : Creation    
!              4.1  2002-Oct-01 : Added quality flags
!              4.2  2002-Dec-02 : Added 3D features (spaxels, groups)
______________________________________________________________________________*/

/* Euro3d Format predefined values */

/* columns */

#define E3D_VERS       "E3D_VERS"
#define E3D_ADC        "E3D_ADC"
#define E3D_DATA       "E3D_DATA"
#define E3D_GRP        "E3D_GRP"
#define E3D_COL_ID     "SPEC_ID"
#define E3D_COL_INT    "DATA_SPE"
#define E3D_COL_DQ     "QUAL_SPE"
#define E3D_COL_RMS    "STAT_SPE"
#define E3D_COL_GRP    "GROUP_N"
#define E3D_COL_NSPAX  "NSPAX"
#define E3D_COL_NPIX   "SPEC_LEN"
#define E3D_COL_IDX    "SPEC_STA"
#define E3D_COL_SPAXID "SPAX_ID"
#define E3D_COL_XPOS   "XPOS"
#define E3D_COL_YPOS   "YPOS"
#define E3D_COL_FLAG   "SELECTED"
#define E3D_COL_SHAPE  "G_SHAPE"
#define E3D_COL_SIZE1  "G_SIZE1"
#define E3D_COL_ANGLE  "G_ANGLE"
#define E3D_COL_SIZE2  "G_SIZE2"
#define E3D_COL_PWAVE  "G_POSWAV"
#define E3D_COL_AIRM   "G_AIRMAS"
#define E3D_COL_PANG   "G_PARANG"
#define E3D_COL_PRES   "G_PRESSU"
#define E3D_COL_TEMP   "G_TEMPER"
#define E3D_COL_HUM    "G_HUMID"

/* descriptors */

#define E3D_KW_START   "CRVALS"
#define E3D_KW_STEP    "CDELTS"
#define E3D_KW_NPTS    "NPTS"
#define E3D_KW_UNITS   "CTYPES"
#define E3D_KW_REFPIX  "CRPIXS"

/* shapes */

#define RECTANG 'R'
#define SQUARE  'S'
#define CIRCLE  'C'
#define HEXAGON 'H'

/*_________________________________ SPAXEL ___________________________________*/

typedef struct 
{
    int specId;                   /* spectrum ID  */
    char spaxelId;                /* spaxel ID    */               
    int group;                    /* group number */
    float xpos;                   /* x position   */
    float ypos;                   /* y position   */

} SPAXEL;

/*_________________________________ GROUPS ___________________________________*/

typedef struct
{
    int  groupId;                 /* group number                   */
    char shape;	          	  /* shape keyword                  */
    float size1;	          /* first spaxel size parameter    */
    float size2;	          /* second spaxel size parameter   */
    float angle;		  /* orientation of the spaxel      */
    float poswav;	          /* wavelength for ADC             */
    float airmass;	          /* airmass                        */
    float parang;	          /* paralactic angle for ADC       */
    float pressure;	          /* pressure for ADC               */
    float temperature;	          /* temperature for ADC            */
    float rel_humidity;	          /* relative humidity              */

} GROUP;

/*__________________________________ E3D_file _____________________________*/

typedef struct            
{
    int      specId;              /* no of associated lens          */
    double   start;               /* coordinate of 1st pixel        */
    double   end;                 /* coordinate of last pixel       */
    int      npix;                /* number of pixels               */
    unsigned int data_offset;     /* offset for data values         */

} E3Dspec_desc;

typedef struct 
{
    char   name[lg_name+1];       /* name of data frame             */
    int    imno;                  /* image number                   */
    short  file_type;             /* Euro 3D file                   */
    short  data_format;           /* file format                    */
    int    iomode;                /* I_MODE, O_MODE, IO_MODE        */
    int    nwcs; 
    struct wcsprm *wcs;           /* world coordinates system      */
    char   history[lg_hist+1];    /* history                        */
    void   *external_info;        /* for descriptors                */
    char   ident[lg_ident+1];     /* identifier                     */
    char   cunit[lg_unit+1];      /* unit                           */
    char   table_name[lg_name+1]; /* name of associated table       */
    double step;                  /* step size                      */
    short  data_type;             /* data_type                      */
    char   version[lg_version+1]; /* version of file format         */
    int    nbspec;                /* number of included spectra     */
    int    crop;                  /* flag for cropping spectra      */
    char   *crop_mask;            /* crop mask (data quality values)*/
    short  common_bounds;         /* TRUE if all spectra have the   */
                                  /* same bound parameters          */
    double common_parameters[3];  /* common param. if common bounds */
    int    extra_hd_off;          /* offset for extra header data   */
    unsigned int data_offset;     /* offset for data                */
    short  swapbytes;             /* if bytes need to be swapped    */

    E3Dspec_desc *signal;         /* spectra description            */
    E3Dspec_desc *noise;          /* spectra description            */

    int    ngroups;               /* number of groups               */
    GROUP  *groups;               /* description of each group      */

    char   s1_unit[lg_label+1];	  /* units of first spaxel size     */
    char   s2_unit[lg_label+1];	  /* units of first spaxel size     */
    char   ang_unit[lg_label+1];  /* angle units                    */
    char   pw_unit[lg_label+1];	  /* wavelength units               */
    char   pa_unit[lg_label+1];	  /* paralactic angle units         */
    char   pr_unit[lg_label+1];	  /* pressure units                 */
    char   temp_unit[lg_label+1]; /* temperature units              */
    char   hum_unit[lg_label+1];  /* rel. humidity units            */
    
} E3D_file;

/*______________________________________ SLICE _______________________________
*/

typedef struct 
{
    int    index;                 /* index                         */
    int    npts;                  /* length                        */
    int    *specId;               /* pointer to associated spectrum*/
    short  data_type;             /* data_type                     */
    union {
      short  *s_data;             /* pointer to first data         */
      long   *l_data;             /* pointer to first data         */
      float  *f_data;             /* pointer to first data         */
      double *d_data;             /* pointer to first data         */
    } data;
    unsigned long *quality;       /* pointer to quality flags      */

} SLICE;

#include <3D_iofunc.h>
