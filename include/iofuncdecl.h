/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        iofuncdecl.h
! LANGUAGE     C
! AUTHOR       A.Rousset
! KEYWORDS     
! PURPOSE      Functions declarations for usual i/o
! COMMENT      
! VERSION      4.0  1992-June-12 : Creation,   AR 
 ---------------------------------------------------------------------*/

/*    functions contained in iolib.c   */

void init_session(char **, int , char ***, char ***);
void exit_session(int);
void stop_by_user();
int RD_desc(void *, char *, short, int, void *out_descval);
int WR_desc(void *, char *, short, int, void *in_descval);
int delete_desc(void *, char *);
int get_all_desc(void *, char ***);
int get_descr_type(void *, char *, short *);
int CP_non_std_desc(void *, void *);
int WR_history(void *, void *); 
int create_spec(SPECTRUM *, char *, int, double, double, short, char*, char*);
int header_spec(SPECTRUM *, char *, char *);
int open_spec(SPECTRUM *, char *, char *);
int close_spec(SPECTRUM *);
int delete_spec(SPECTRUM *);
unsigned long RD_qspec(SPECTRUM *, int);
int WR_qspec(SPECTRUM *, int, unsigned long);
int create_spec_mem(SPECTRUM *, int, double, double, short);
int create_frame(IMAGE2D*, char*, int*, double*, double*, short, char*, char*);
int header_frame(IMAGE2D *, char *, char *);
int open_frame(IMAGE2D *, char *, char *);
int close_frame(IMAGE2D *);
int delete_frame(IMAGE2D *);
unsigned long RD_qframe(IMAGE2D *, int, int);
int WR_qframe(IMAGE2D *, int, int, unsigned long);
int create_table(TABLE *, char *, int, int, char, char *); 
int open_table(TABLE *, char *, char *);
int close_table(TABLE *);
int handle_select_flag(TABLE *, char, char *);
int write_selection(TABLE *,int *, char *);
int get_col_ref (TABLE *, char *);
int get_col_info (TABLE *, int, int *, char *, char *);
int get_col_name (TABLE *, int, char *);
int create_col(TABLE *, char *, short, char, char *, char *);
int search_in_col (TABLE *, int, void *);
int delete_col (TABLE *, int);
int delete_row (TABLE *, int);
int RD_col(TABLE *, int, void *out_tblcol);
int RD_tbl(TABLE *, int, int, void *out_tblval);
int WR_tbl(TABLE *, int, int, void *in_tblval);
int WR_null(TABLE *, int, int);
int delete_table(TABLE *);
int create_cube(IMAGE3D*, char*, int*, double*, double*, short, char*, char*);
int header_cube(IMAGE3D *, char *, char *);
int open_cube(IMAGE3D *, char *, char *);
int close_cube(IMAGE3D *);
int delete_cube(IMAGE3D *);
unsigned long RD_qcube(IMAGE3D *, int, int, int);
int WR_qcube(IMAGE3D *, int, int, int, unsigned long);
int RD_catalog(char *, char *);

/*    functions contained in io_utils.c   */

void coord_frame(IMAGE2D *,int ,int , float *, float *);
void pixel_frame(IMAGE2D *, float, float, int *, int *);
int set_subspec(SPECTRUM *, double, double);
int spec_minmax(SPECTRUM *);
int inter_spec(SPECTRUM *,SPECTRUM *);
int image_minmax(IMAGE2D *);
int flip_frame(IMAGE2D *, int);
int cube_minmax(IMAGE3D *);

/*    functions contained in io_misc.c   */

int set_inputformat(char *);
int set_outputformat(char *);
int set_user_dataformat(); 
void append_ima_extension(char *, short);
void append_tbl_extension(char *, short);
void append_datacube_extension(char *, short);
void remove_file_extension(char *);
char *file_format(char *);
int file_type(char *);
int get_iomode_code(short, int);
int get_datatype_code(short, short);
short decode_datatype(short, short);
int sizeof_item(short);
int alloc_spec_mem(SPECTRUM *, short);
int free_spec_mem(SPECTRUM *);
int alloc_frame_mem(IMAGE2D *, short);
int free_frame_mem(IMAGE2D *);
int alloc_cube_mem(IMAGE3D *, short);
int free_cube_mem(IMAGE3D *);
int alloc_new_desc(Anyfile *, short, int); 
int free_all_desc(Anyfile *);
int read_file_class(void *);
int write_file_class(void *, int);
int set_super_class(void *);
int unset_super_class(void *);
int fits_bitpix(short);
int fits_datatype(int);
int fits_non_std_desc(char *);
int copy_table_desc(void *, void *);

/* functions defined in version.c */
int set_version(char *);
int set_purpose(char *);

/* functions defined in fits_primary_hd.c */
int open_primary_hd(Anyfile *, char *, char *);
int close_primary_hd(Anyfile *);

/* parse_wcs */

int parse_wcs(void *);
int wcs_free (void *);
