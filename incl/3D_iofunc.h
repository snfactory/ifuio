/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        3D_iofunc.h
! LANGUAGE     C
! AUTHOR       A.Rousset
! KEYWORDS     
! PURPOSE      Functions declarations for usual Tiger i/o
! COMMENT      
! VERSION      4.0  1994-Jul-30 : Creation,   AR 
!---------------------------------------------------------------------*/

/*    functions contained in 3D_iolib.c   */

int create_E3D_file(E3D_file *, char *, int, double, double, short, char *, char *);
int open_E3D_file(E3D_file *, char *, char *);
int close_E3D_file(E3D_file *);
/* int delete_E3D_file(E3D_file *);  problem with python wrapper */

int init_new_E3D_spec(E3D_file *, SPECTRUM *, int , double);
int get_E3D_spec(E3D_file *, SPECTRUM *, SPECTRUM *,int);
int put_E3D_spec(E3D_file *, SPECTRUM *, SPECTRUM *,int);
int delete_E3D_spec(E3D_file *, int);

int get_E3D_spaxels(E3D_file *, int, SPAXEL **);
int put_E3D_spaxels(E3D_file *, int, int, SPAXEL *);

int exist_group_ID(E3D_file *, char);
int get_E3D_groups(E3D_file *, int, GROUP **);
int put_E3D_groups(E3D_file *, int, GROUP *);
int delete_E3D_group(E3D_file *, int);

int get_E3D_row(E3D_file *, int, SPECTRUM *, SPECTRUM *, int *, SPAXEL **, int *, GROUP **);
int put_E3D_row(E3D_file *, int, SPECTRUM *, SPECTRUM *, int , SPAXEL *, int , GROUP *);

int get_E3D_frame(E3D_file *, IMAGE2D *, IMAGE2D *);

int init_new_E3D_slice(E3D_file *, SLICE *);
int get_E3D_slice(E3D_file *, SLICE *, SLICE *,int);
int put_E3D_slice(E3D_file *, SLICE *, SLICE *,int);

int alloc_slice_mem(SLICE *, short, int);
int free_slice_mem(SLICE *);

int has_common_bounds(E3D_file *);
int get_common_param(E3D_file *, int *, double *, double *);
int set_common_bounds(E3D_file *);

int exist_spec_ID(E3D_file *, int);
int exist_statistical_error(E3D_file *);

int set_ID_and_coordinates(E3D_file *,int, int *,float *,float *);
int get_ID_and_coordinates(E3D_file *,int *,float *, float *);
int get_spectra_ID(E3D_file *, int *);


/* compat.c (compatibility stuff) */

int set_tiger_group(E3D_file *);
int create_tiger_frame(E3D_file *,char *,int,double,double,short,char *,char *,char *);
int close_tiger_frame(E3D_file *);
int delete_tiger_spec(E3D_file *, SPECTRUM *, SPECTRUM *, int);
int set_bigendian(short);
int get_coord_table_name(char *, char *);
int get_science_table_name(char *, char *);
int get_lenses_no_from_table(E3D_file *, int *);
int get_lenses_coord(E3D_file *, char *, char *, int *, float *, float *, int *);
int set_lenses_coord(E3D_file *,char *,char *,int,int *, float *, float *);
int get_lenses_coord_select(E3D_file *, char *, char *, int *, float *, float *, int *, int *);
int init_new_tiger_slice(E3D_file *, SLICE *, int);
int get_3D_slice_noalloc(E3D_file *, SLICE *, SLICE *, int);
int get_lens_coordinates(E3D_file *, int , float *, float *);
int set_lens_coordinates(E3D_file *, int , float *, float *); 
int interpolate_noise(SPECTRUM *);
