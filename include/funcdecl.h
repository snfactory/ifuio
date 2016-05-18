/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        funcdecl.h
! LANGUAGE     C
! AUTHOR       A.Rousset
! KEYWORDS     
! PURPOSE      Functions declarations 
! COMMENT      
! VERSION      4.0  1992-June-12 : Creation,   AR 
 ---------------------------------------------------------------------*/

/* functions contained in io_utils.c   */

int print_msg(char *,...);
int print_label(char *);
int print_progress(char *,float,float);
int reset_print_progress(); 
int print_warning(char *,...);
int print_error(char *,...);
int print_msg_nocr(char *);

/* functions contained in io_error.c   */

void set_control_level(int);	
void Handle_Error(char *, int);
int  get_tiger_errcode(short, int);
void get_tiger_errmsg(int, char *);
void disable_user_warnings();
void restore_user_warnings();
void disable_erase_flag();
void restore_erase_flag();

/* functions in dyn_alloc.c   */

int alloc2d(void *, int, int, short);
int free2d(void *, short);

/* functions in files.c   */

int exist(char *);
char *get_path(char *);
int remove_path(char *);
int exist_extension(char *);
int read_DOS(int, void *, int);
int write_DOS(int, void *, int);
int f_lines(char *);

/* functions in strings.c   */

int first_blk(char *);
int last_char(char *);
int last_char_before(char *, char);
int fill_blk(char *, int);
int string_compar(char *, char *);
void lower_strg(char *);
void upper_strg(char *);
int is_upper_string(char *);
int is_lower_string(char *);
int is_numeric(char *);

/* functions in parse_arg.c   */

int set_arglist(char *);
int parse_arg(char **, int, char ***, char ***);

void get_argval(int, char *, void *);

/* functions in convert.c (gen.a) */

void convert_spec(char *in,char *out);
void convert_table(char *in,char *out);
void convert_image(char *in,char *out);

/* functions in decode.c   */

double trunc_double(double, int);
int decode_argval_char(char *, char **);
int decode_argval_double(char *, double *, char *);
int decode_argval_int(char *, int *, char *);
int decode_argval_float(char *, float *, char *);
int find_selected_arg(char **, char **, int *, int);

/* functions in julian.c */

double julian(int ,int ,int ,int ,int ,double );
void fromjulian(double ,int *,int *,int *,int *,int *,double *);
char *Mjulian(long *);
