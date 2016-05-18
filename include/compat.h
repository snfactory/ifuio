/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        compat.h (was tiger_def.h>
! LANGUAGE     C
! 
! AUTHOR       A. Rousset
! 
! KEYWORDS     Tiger format definition 
! PURPOSE      
! COMMENT     
! VERSION      4.0  1992-June-15 : Creation    
!              4.1  2002-Oct-01 : Added quality flags
!              4.2  2004-Sep-04 : renamed compat.c and inserted all compatibility stuff
______________________________________________________________________________*/

#define LAB_COL_NO "NO"
#define LAB_COL_XLD "XD"
#define LAB_COL_YLD "YD"
#define LAB_COL_XLND "XND"
#define LAB_COL_YLND "YND"
#define LAB_COL_XTH_LD "XTHD"
#define LAB_COL_YTH_LD "YTHD"
#define LAB_COL_XTH_LND "XTHND"
#define LAB_COL_YTH_LND "YTHND"        

#define E3D_TIGER_EXT "TIGERTBL"

#define TIGERfile E3D_file
#define TIGERspec_desc E3Dspec_desc
#define open_tiger_frame open_E3D_file
#define delete_tiger_frame delete_E3D_file
#define init_new_tiger_spec init_new_E3D_spec
#define get_tiger_spec get_E3D_spec
#define put_tiger_spec put_E3D_spec
#define get_tiger_slice get_E3D_slice
#define put_tiger_slice put_E3D_slice
#define exist_noise exist_statistical_error
#define exist_lens exist_spec_ID
#define get_lenses_no get_spectra_ID
#define create_E3D_frame create_E3D_file
#define open_E3D_frame open_E3D_file
#define close_E3D_frame close_E3D_file
#define delete_E3D_frame delete_E3D_file
#define load_TIGER_max load_3D_max
#define alloc_TIGER_max alloc_3D_max
#define save_TIGER_max save_3D_max
#define open_fits_primary_hd open_primary_hd

/*____________________ MAXIMA of cross-dispersion profiles __________________*/

typedef struct
{
    float xcoord;                 /* x coordinate of the maximum     */
    float intens;                 /* intensity found for the PSF     */
    float sigma[2];               /* width found for the PSF         */
    float alpha;                  /* alpha coef of the PSF           */

} max_param;

typedef struct
{
    float     ycoord;             /* y coordinate of this set        */
    int       nb_max;             /* number of maxima found          */
    max_param *maxima;            /* detailed list of maxima         */

} max_lines;

typedef struct            
{

  char config_name[lg_ident+1];

    struct                      /* enlarger :                     */
    {
      double gamma;             /* enlarging factor               */
    } enlarger;

    struct                      /* wedge (angles are in radian)   */
    {
      double theta_w;           /* rotation between the 2 prisms  */
    } wedges; 

    struct                      /* grating prism :                */
    {
      int index_coef;           /* - index of coefficient         */
      double A;                 /* - angle of the prism           */
      int    g_per_mm;          /* - number of grooves per mm     */
    } grism; 

    struct
    {
      double inf_util, sup_util;
    } filter;

    struct
    {
      float fratio;
    } telescope;

    struct
    {
      int nx, ny; 
	  float pixsize;
    } ccd;

    int nb_ycoords;         /* number of coordinates on Y axis */
    max_lines *line;        /* details of maxima line by line  */

} Maxima_Set;

/*    functions contained in compat.c   */

int get_lenses_no_from_table(E3D_file *, int *);

int alloc_3D_max(Maxima_Set *, int, int);
int load_3D_max(Maxima_Set *, char *);
int save_3D_max(Maxima_Set *, char *);
int interpolate_noise(SPECTRUM *);
