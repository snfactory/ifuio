/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        iolib.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      Basic i/o routines
! COMMENT      
! VERSION      4.0  1992-Jun-15 : Creation, AR
!              4.1  2002-Nov-30 : Added access to FITS extension, AP 
!              4.2  2003-Jun-06 : free memory when deleting an open file
!              4.4  2004-Jul-01 : fix WR_desc when keyword has synonyms
!              4.5  2004-Nov-17 : memory fixes in WR_desc (YC), 
!                                 debug in CP_non_std_desc (strcpy, HIRARCH keywords)
!              4.6  2005-Jan-06 : suppress generic types TYPE_TBL et TYPE_IMA
!              4.7  2005-Sep-16 : increased soft_version length for Snifs needs
---------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/times.h>

#include <IFU_io.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>

/* include specific codes for I/O */

#include <data_io.h>

#ifdef MIDAS
#include "../midas_io/midas_defs.h"
#include "../midas_io/tbldef.h"
#endif

#ifdef FITS
#include <fitsio.h>
#include <longnam.h>
#endif

#ifdef IRAF
#include "../iraf_io/incl/iraf_def.h"
#endif

#define STOP_PROG 10000

extern IO_Format InputIO, OutputIO;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk                 Settings (to be red prior to use !)
!
! Values for the data type parameter required in various functions
! are predefined in IFU_datatypes.h. 
! Permitted values are :
!        
!  CHAR, STRING,
!  SHORT,
!  INT, 
!  LONG,
!  FLOAT, 
!  DOUBLE 
!
----------------------------------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!	
!.blk                Message for segmentation fault (needed by TK)
!
!.func                         sig_segm
!
!.purp                    trap segmentation fault signal
!.desc
! sig_segm(int nb)
!
! int nb;               signal number
! 
!.ed
----------------------------------------------------------------------*/

void sig_segm(int nb)
{
  char text[132];
  if (TK)
    {
      printf("@ K\n");
      fflush(stdout);
    }
  sprintf(text,"Memory fault detected, signal %d",nb);
  print_error(text);
  exit(-1);
}

void sig_usr1(int nb)
{
  ASK_BACK = 1;
}

void sig_usr2(int nb)
{
  ASK_BACK = 0;
}

void do_nothing(int nb)
{

}

void confirme_erase(char *name)
{
  if (exist(name)) {
    if (TK) {
      signal(SIGUSR1,sig_usr1);
      signal(SIGUSR2,sig_usr2);

      printf("@ Q \"%s\"\n",name);
      fflush(stdout);
      pause();
      signal(SIGUSR1,do_nothing);
      signal(SIGUSR2,do_nothing);
      if (!ASK_BACK) {
        print_error("Aborting current program");
        exit_session(ERR_ACCESS);
      }
      else print_msg("Overwriting file %s",name);
    }
    else {
      char back, dummy;

      printf("WARNING: %s already exists\n",name);
      printf("Do you really want to overwrite it (Y/N)? ");
      back = getchar();
      /* Discard all the characters in the input buffer until new-line */
      while ((dummy=getchar()) != EOF && dummy != '\n');
      if ((back == 'N') || (back == 'n')) {
        print_error("Aborting current program");
        exit_session(ERR_ACCESS);
      }
      else printf("Overwriting file %s\n",name);

    }
  }
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!	
!.blk                Getting/Exiting SESSION
!
!.func                         init_session()
!
!.purp          gets set-up variables and inits logfile
!.desc
! init_session(argv,argc,arg_labels,arg_values)
!
! char **argv;          parameter list passed to main program
! int  argc;            number of parameters
! char ***arg_labels;   list of labels returned by parse_arg
! char ***arg_values;   list of values returned by parse_arg
!.ed
----------------------------------------------------------------------*/

char Calling_Prog[lg_name+1] = "program";
char Cmd_History[lg_hist+1] = "";
extern int List_Length;
extern char **pt_ArgLabels;
char soft_version[20] = "0.0";	/* non initialized value */

void 
init_session(char **argv, int argc, char ***arg_labels, char ***arg_values)
{
  char errtext[256], *pt_filename, debug_file[lg_name+1], strtime[25], **pt_arg;
  int status, i;
  time_t elapsed;

  signal(SIGSEGV,sig_segm); /* for detecting segmentation fault) */

  if ((pt_filename = strrchr(argv[0],'/')) != NULL)
    strcpy(Calling_Prog,pt_filename+1);
  else
    strcpy(Calling_Prog,argv[0]);

  sprintf(debug_file,"%s.dbg",Calling_Prog);

  set_user_dataformat();

  status = parse_arg(argv,argc,arg_labels,arg_values);
  if (status < 0) {
    Handle_Error("init_session", status);
    exit_session(status);
  }
  time(&elapsed);
  strcpy(strtime,(char *)asctime(localtime(&elapsed)));
  strtime[24]='\0';
  sprintf(Cmd_History,"%s: %s",strtime,Calling_Prog);
  pt_arg = *arg_values;
  for (i=0; i< List_Length; i++) {	
    /* save command for history desc. */
    strcat(Cmd_History," ");
    strcat(Cmd_History,pt_ArgLabels[i]);
    strcat(Cmd_History," ");
    strcat(Cmd_History,pt_arg[i]);
  }

  if (DEBUG) {
    freopen(debug_file,"w",stderr);
    if (stderr == NULL)  
      exit_session(-1);
    else {
      fprintf(stderr, "Date    : %s\n", strtime);
      fprintf(stderr, "Path    : %s\n", getenv("PWD"));
      fprintf(stderr, "Command : %s\n", Calling_Prog);
      for (i=0; i<List_Length; i++)
        fprintf(stderr,"%s %s\n",pt_ArgLabels[i],pt_arg[i]);
      fprintf(stderr,"\n");
    }
  }
  sprintf(errtext,
          "Program: %s  Version %s-%s", Calling_Prog,soft_version,VERSION);

  if (VERBOSE) print_msg(errtext);

#ifdef MIDAS
  if ((InputIO.basic_io == MIDAS_FORMAT) || (OutputIO.basic_io == MIDAS_FORMAT))
    SCSPRO("-");
#endif

#ifdef IRAF
  if ((InputIO.basic_io == IRAF_FORMAT) || (OutputIO.basic_io == IRAF_FORMAT)
      || (InputIO.basic_io == STSDAS_FORMAT) || (OutputIO.basic_io == STSDAS_FORMAT))
    irafmn();
#endif

  set_control_level(0);			/* default value */

  if (TK) printf("@ I %d\n",getpid());
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        exit_session()
!
!.purp                        terminates run 
!.desc
! exit_session(status)
!
! int status;           return status
!.ed
--------------------------------------------------------------------*/
void
exit_session(int status)
{
  char errtext[256];

  switch (status) {

  case 0 :                                   /* everything is OK */
    sprintf(errtext,"End of %s",Calling_Prog);
    if (TK) printf("@ E\n");
    break;
  case STOP_PROG :
    sprintf(errtext,"%s stopped by user",Calling_Prog);
    if (TK) printf("@ S\n");
    break;
  default :
    if (errno == ENOSPC)
      sprintf(errtext,"%s aborted - disk is full",Calling_Prog);
    else
      sprintf(errtext,"%s aborted - status code: %d",Calling_Prog,status);
    if (TK) printf("@ A\n");
    break;
  }
  if (VERBOSE) print_msg(errtext);
  if (DEBUG) {
    fclose(stderr);
  }

#ifdef MIDAS
  if ((InputIO.basic_io == MIDAS_FORMAT) || (OutputIO.basic_io == MIDAS_FORMAT)) {
    SCSEPI();
  }
#endif
  exit(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     stop_by_user()
!
!.purp           ends a program execution on user request
!
!.desc 
! void stop_by_user()
!.ed
--------------------------------------------------------------------*/

void 
stop_by_user()
{
  exit_session(STOP_PROG);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk                Routines for DESCRIPTORS read/write
!
!.func                         get_descr_type()
!
!.purp             returns type of data in given descriptor
!.desc
! get_descr_type(anyfile,descr,type)
!
! (type) *anyfile;      image, spectrum or table
! char *descr;          name of descriptor
! short *type;          type of data
!.ed
----------------------------------------------------------------------*/

int get_descr_type(void *anyfile, char *descr, short *type) 
{
	  Anyfile *pt_file;
	  int stat=0, i;
	  int nbelem, bytelem;
	  Descr_Items *dsc_items;
	  char errtext[132];
	  char data_type[2];

#ifdef FITS
	  fitsfile *fptr;
	  char buffer[4097];
	  char dtype;
	  char *pt_buf;
	  int n;
#endif

  pt_file = (Anyfile *)anyfile;
  switch (pt_file->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    stat = SCDFND(pt_file->imno,descr,data_type,&nbelem,&bytelem);
    if (stat) break;
    switch (data_type[0]) {
    case 'C' : *type = CHAR;
      break;
    case 'I' : *type = INT;
      break;
    case 'R' : *type = FLOAT;
      break;
    case 'D' : *type = DOUBLE;
      break;
    default : stat = -1;
      break;
    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len_descr = strlen(descr);

    uhdgtp(&(pt_file->imno),descr,&datatype,&stat,len_descr);

    *type = decode_datatype(IRAF_FORMAT,(short)datatype);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
  case EURO3D_FORMAT :
    stat = 0;
    fptr = (fitsfile *)pt_file->external_info;
    if  ((strncmp(descr,"COMMENT",7) == 0)
         ||  (strncmp(descr,"HISTORY",7) == 0)) {
      fits_read_key_longstr(fptr, descr, &pt_buf, NULL, &stat);
      if (stat) break;
      n = MIN(strlen(pt_buf),4096);
      strncpy(buffer,pt_buf,n);
      buffer[n] = 0;
      dtype = 'C';
      free(pt_buf);
    }
    else {
      fits_read_keyword(fptr, descr, buffer, NULL, &stat);
      nbelem = 1;
      fits_get_keytype(buffer,&dtype,&stat);
    }
    if (stat == 0) {
      switch(dtype)
        {
        case 'L' : /* logical */
        case 'C' : /* string */
          *type = CHAR;
          nbelem = strlen(buffer);
          if (buffer[0] == '\'') nbelem -= 1;
          if (buffer[strlen(buffer)-1] == '\'') nbelem -= 1;
          if (buffer[0] == '=') nbelem -= 2;
          break;
        case 'I' : /* integer */
          *type = INT;
          break;
        case 'F' : /* float */
          *type = DOUBLE;
          break;
        }
    }
    break;

#endif
  case TIGER_FORMAT :
    stat = 0;
    if (pt_file->external_info == NULL) {
      stat = -1;
      break;
    }
    dsc_items = (Descr_Items *)(pt_file->external_info);
    for (i=0; i<dsc_items->nb_descr
           && strcmp(dsc_items->descr_list[i].descr_name,descr) != 0 ;i++);
    if (i == dsc_items->nb_descr) {
      stat = ERR_NODESC;
      break;
    }
    *type = dsc_items->descr_list[i].data_type;
    nbelem = dsc_items->descr_list[i].nb_values;
    break;
  }
  if (stat) {
    sprintf(errtext, "get_descr_type: file= %s desc=%s", pt_file->name,descr);
    stat = get_tiger_errcode(pt_file->data_format,stat);
    Handle_Error(errtext, stat);
  }

  if (stat<0)
    return(stat);
  else
    return(nbelem);
}

int 
Read_one_desc(Anyfile *anyfile, char *descr, short type, int nb_elt, void *val)
{
  int stat, nb_val=nb_elt, nbread, i;
  int l;
  int nulls;
  Anyfile *pt_file;
  float *pt_float;
  double *pt_double;
  char errtext[132];
  Descr_Items *dsc_items;
  int unit;
#ifdef FITS
  fitsfile *fptr;
  char buffer[4097];
  char *pt_buf;
  int int_datatype;
  int n;
#endif

  pt_file = (Anyfile *)anyfile;

  switch (pt_file->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    switch (type) {
    case CHAR :
      stat = SCDRDC(pt_file->imno,descr,1L,1L,nb_val,&nbread,
                    (char *)val,&unit, &nulls);
      if (! stat) *((char *)val+nbread) = '\0';
      break;
    case SHORT :
    case INT :
    case LONG :
      stat = SCDRDI(pt_file->imno,descr,1L,nb_val,&nbread,
                    (int *)val,&unit, &nulls);
      break;
    case FLOAT :
      stat = SCDRDR(pt_file->imno,descr,1L,nb_val,&nbread,
                    (float *)val,&unit, &nulls);
      break;
    case DOUBLE :
      stat = SCDRDD(pt_file->imno,descr,1L,nb_val,&nbread,
                    (double *)val,&unit, &nulls);
      break;
    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len_descr = strlen(descr);
    /* check for table descriptors */
    if (pt_file->file_type != T_TABLE) {

      switch (type) {		/* frame descriptor */
      case CHAR :
        uhdgvt(&(pt_file->imno),descr,&one,&one,&nbread,val,
               &stat,len_descr,nb_val);
        break;
      case SHORT :
        uhdgvs(&(pt_file->imno),descr,&one,&nb_val,&nbread,val,
               &stat,len_descr);
        break;
      case INT :
        uhdgvi(&(pt_file->imno),descr,&one,&nb_val,&nbread,val,
               &stat,len_descr);
        break;
      case LONG :
        uhdgvl(&(pt_file->imno),descr,&one,&nb_val,&nbread,val,
               &stat,len_descr);
        break;
      case FLOAT :
        uhdgvr(&(pt_file->imno),descr,&one,&nb_val,&nbread,val,
               &stat,len_descr);
        break;
      case DOUBLE :
        uhdgvd(&(pt_file->imno),descr,&one,&nb_val,&nbread,val,
               &stat,len_descr);
        break;
      }
    }
    else {
      switch (type) {		/* table descriptor */
      case CHAR :
        uthgtt(&(pt_file->imno),descr,val,&stat,len_descr,nb_val);
        break;
      case SHORT :
      case LONG :
      case INT :
        uthgti(&(pt_file->imno),descr,val,&stat,len_descr);
        break;
      case FLOAT :
        uthgtr(&(pt_file->imno),descr,val,&stat,len_descr);
        break;
      case DOUBLE :
        uthgtd(&(pt_file->imno),descr,val,&stat,len_descr);
        break;
      }
    }
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
  case EURO3D_FORMAT :

    fptr = (fitsfile *)pt_file->external_info;
    stat = 0;
    nbread = nb_val;
    if (nbread == 0)
      break;
    if (type == CHAR) {
      *((char *)val) = '\0';
      int_datatype = get_datatype_code(pt_file->data_format,type);
      if  ((strcmp(descr,"COMMENT") == 0) 
           ||  (strcmp(descr,"HISTORY") == 0)) {
        fits_read_key_longstr(fptr, descr, &pt_buf, NULL, &stat);
        if (stat) break;
        n = MIN(strlen(pt_buf),nb_val);
        strncpy((char *)val,pt_buf,n);
        *((char *)val + n) = '\0';
        free(pt_buf);
      }
      else {
        fits_read_key(fptr, int_datatype, descr, buffer, NULL, &stat);
        if (stat) break;
        if (strlen(buffer) < nb_val)
          strcpy((char *)val,buffer);
        else {
          strncpy((char *)val,buffer,nb_val);
          *((char *)val + nb_val) = '\0';
        }
      }
      break;
    }
    if (nb_val == 1) {
      int_datatype = get_datatype_code(pt_file->data_format,type);
      fits_read_key(fptr, int_datatype, descr, val, NULL, &stat);
      break;
    }
    switch (type) {
    case CHAR :
      fits_read_keys_str(fptr,descr,1,nb_val,val,&nbread,&stat);
      break;
    case SHORT :
    case LONG :
    case INT :
      fits_read_keys_lng(fptr,descr,1,nb_val,val,&nbread,&stat);
      break;
    case FLOAT :
      fits_read_keys_flt(fptr,descr,1,nb_val,val,&nbread,&stat);
      break;
    case DOUBLE :
      fits_read_keys_dbl(fptr,descr,1,nb_val,val,&nbread,&stat);
      break;
    }
    break;
#endif
  case TIGER_FORMAT :
    stat = 0;
    if (pt_file->external_info == NULL) {
      stat = -1;
      break;
    }
    dsc_items = (Descr_Items *)(pt_file->external_info);
    for (i=0; i<dsc_items->nb_descr
           && strcmp(dsc_items->descr_list[i].descr_name,descr) != 0 ;i++);
    if (i == dsc_items->nb_descr) {
      stat = ERR_NODESC;
      break;
    }
    if ((dsc_items->descr_list[i].data_type == LONG) 
        && (type == INT)) 
      dsc_items->descr_list[i].data_type = type;
    if ((dsc_items->descr_list[i].data_type == INT) 
        && (type == LONG)) 
      dsc_items->descr_list[i].data_type = type;

    nb_val = MIN(nb_val,dsc_items->descr_list[i].nb_values);
    nbread = nb_val;

    if (dsc_items->descr_list[i].data_type != type) {
      if ((dsc_items->descr_list[i].data_type == DOUBLE) 
          && (type == FLOAT)) {
        pt_float = (float*)val;
        for (l=0;l<nb_val;l++)
          pt_float[l] = 
            (float)dsc_items->descr_list[i].descr_value.d_data[l];
      }
      else {
        if ((dsc_items->descr_list[i].data_type == FLOAT) 
            && (type == DOUBLE)) {
          pt_double = (double*)val;
          for (l=0;l<nb_val;l++)
            pt_double[l] = 
              (double)dsc_items->descr_list[i].descr_value.f_data[l];
        }
        else 
          stat = ERR_BAD_DESC;
      }
    } 
    else {
      memcpy(val,dsc_items->descr_list[i].descr_value.c_data,
             nb_val*sizeof_item(type));
      if (type == CHAR)
        *((char *)val+nb_val) = 0;
    }
    break;
  }
  if (stat) {
    sprintf(errtext, "RD_desc: file= %s desc=%s", pt_file->name,descr);
    stat = get_tiger_errcode(pt_file->data_format,stat);
    Handle_Error(errtext, stat);
  }
  if (stat < 0)
    return(stat);
  else
    return(nbread);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                         RD_desc()
!
!.purp   reads values in descriptor, returns number of values read
!.desc
! nbval = RD_desc(anyfile,descr,descr_type,nb_elt,val)
!
! (type) *anyfile;      image, spectrum or table
! char *descr;          name of descriptor (or list of synonym)
! short type;           type of data
! int nb_elt;           number of elements to read
! (type) *val;       array to store values
!.ed
----------------------------------------------------------------------*/

int 
RD_desc(void *anyfile, char *descr, short type, int nb_elt, void *val) 
{
  short ltype = type;
  int nb = nb_elt;
  int status;
  Anyfile *pt_file;
  char *item, *list_of_desc;
  char errtext[132];

  list_of_desc = (char *)malloc((strlen(descr)+1)*sizeof(char));
  strcpy(list_of_desc,descr);

  disable_user_warnings();

  /* Read synomyms up to 1st one present */
  item = strtok(list_of_desc,"|");
  status = Read_one_desc(anyfile, item, ltype, nb, val);
  while ((item != NULL) && (status < 0)) {
    item = strtok(NULL,"|");
    if (item != NULL) {
      status = Read_one_desc(anyfile, item, ltype, nb, val);
    }
  }
  free(list_of_desc);

  restore_user_warnings();

  if (status < 0) {
    pt_file = (Anyfile *)anyfile;
    sprintf(errtext, "RD_desc: file= %s desc=%s", pt_file->name,descr);
    Handle_Error(errtext, ERR_BAD_DESC);
  }
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                         WR_desc()
!
!.purp          writes values in given descriptor
!.desc
! WR_desc(anyfile,descr,type,nb_elt,retval)
!
! (type) *anyfile;      image, spectrum or table
! char *descr;          name of descriptor
! short type;           type of data
! int nb_elt;           number of elements
! (type) *retval;       array of values to store
!.ed
----------------------------------------------------------------------*/

int 
WR_desc(void *anyfile, char *descname, short type, int nb_elt, void *val)
{
  int stat=0, nb_val=nb_elt, i;
  char errtext[132];
  int unit;
  char *descup, *item, *descr;
  char **pt_val;
  Anyfile *pt_file;
  Descr_Items *dsc_items;

#ifdef IRAF
  int one=1, int_type;
  char iraf_unit[lg_unit+1];
  int len_descr, len_unit, len_val;
#endif

#ifdef FITS
  fitsfile *fptr;
  char **comm;
  char keyname[FLEN_KEYWORD+1];
  char card[FLEN_CARD+1];
  char buffer[4097];
#endif

  pt_file = (Anyfile *)anyfile;

  /* If synonyms, take only first one (up to `|') */
  descr = (char *)malloc((strlen(descname)+1)*sizeof(char));
  strcpy(descr,descname);                    /* Work on a copy */
  if ((item = index(descr,'|')) != NULL) *item = '\0'; /* Truncate at first `|' if any */

  if (pt_file->iomode == (int)I_MODE) {
    stat = ERR_ACCESS;
    sprintf(errtext, "WR_desc: file %s is opened Read-Only", pt_file->name);
    Handle_Error(errtext, stat);
    return(stat);
  }

  switch (pt_file->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    switch (type) {
    case CHAR :
      stat = SCDWRC(pt_file->imno,descr,1,(char *)val,1,nb_val,&unit);
      break;
    case SHORT :
    case INT :
    case LONG :
      stat = SCDWRI(pt_file->imno,descr,(int *)val,1L,nb_val,&unit);
      break;
    case FLOAT :
      stat = SCDWRR(pt_file->imno,descr,(float *)val,1L,nb_val,&unit);
      break;
    case DOUBLE :
      stat = SCDWRD(pt_file->imno,descr,(double *)val,1L,nb_val,&unit);
      break;
    } /* switch (type) */
    break;                                   /* case MIDAS_FORMAT */
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len_descr = strlen(descr);
    int_type = 0;
    /* check for tables descriptors */

    if (pt_file->file_type != T_TABLE) {    /* frames */
      strcpy(iraf_unit,"None given");
      len_unit = strlen(iraf_unit);

      switch (type) {
      case CHAR :
        len_val = strlen(val);
        uhdpvt(&(pt_file->imno),descr,&one,&one,val,&stat,
               len_descr,len_val);
        if (stat != 0) {
          uhdavt(&(pt_file->imno),descr,&one,&one,val,
                 iraf_unit,&int_type,&stat,len_descr,len_val,len_unit);
        }
        break;
      case SHORT :
        uhdpvs(&(pt_file->imno),descr,&one,&nb_val,val,&stat,len_descr);
        if (stat != 0) {
          int_type = get_datatype_code(pt_file->data_format,type);
          uhdavs(&(pt_file->imno),descr,&one,&nb_val,val,
                 iraf_unit,&int_type,&stat,len_descr,len_unit);
        }
        break;
      case INT :
        uhdpvi(&(pt_file->imno),descr,&one,&nb_val,val,&stat,len_descr);
        if (stat != 0) {
          int_type = get_datatype_code(pt_file->data_format,type);
          uhdavi(&(pt_file->imno),descr,&one,&nb_val,val,
                 iraf_unit,&int_type,&stat,len_descr,len_unit);
        }
        break;
      case LONG :
        uhdpvl(&(pt_file->imno),descr,&one,&nb_val,val,&stat,len_descr);
        if (stat != 0) {
          int_type = get_datatype_code(pt_file->data_format,type);
          uhdavl(&(pt_file->imno),descr,&one,&nb_val,val,
                 iraf_unit,&int_type,&stat,len_descr,len_unit);
        }
        break;
      case FLOAT :
        uhdpvr(&(pt_file->imno),descr,&one,&nb_val,val,&stat,len_descr);
        if (stat != 0) {
          int_type = get_datatype_code(pt_file->data_format,type);
          uhdavr(&(pt_file->imno),descr,&one,&nb_val,val,
                 iraf_unit,&int_type,&stat,len_descr,len_unit);
        }
        break;
      case DOUBLE :
        uhdpvd(&(pt_file->imno),descr,&one,&nb_val,val,&stat,len_descr);
        if (stat != 0) {
          int_type = get_datatype_code(pt_file->data_format,type);
          uhdavd(&(pt_file->imno),descr,&one,&nb_val,val,
                 iraf_unit,&int_type,&stat,len_descr,len_unit);
        }
        break;
      } /* switch (type) */
    } /* pt_file->file_type != T_TABLE */
    else {                                   /* tables */
      switch (type) {
      case CHAR :
        len_val = strlen(val);
        uthptt(&(pt_file->imno),descr,val,&stat,len_descr,
               len_val);
        if (stat != 0)
          uthadt(&(pt_file->imno),descr,val,&stat,len_descr,
                 len_val);
        break;
      case SHORT :
      case INT :
      case LONG :
        uthpti(&(pt_file->imno),descr,val,&stat,len_descr);
        if (stat != 0) 
          uthadi(&(pt_file->imno),descr,val,&stat,len_descr);
        break;
      case FLOAT :
        uthptr(&(pt_file->imno),descr,val,&stat,len_descr);
        if (stat != 0) 
          uthadr(&(pt_file->imno),descr,val,&stat,len_descr);
        break;
      case DOUBLE :
        uthptd(&(pt_file->imno),descr,val,&stat,len_descr);
        if (stat != 0) 
          uthadd(&(pt_file->imno),descr,val,&stat,len_descr);
        break;
      }
    } /* pt_file->file_type == T_TABLE */
    break;                               /* case IRAF_FORMAT||STSDAS_FORMAT */
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
  case EURO3D_FORMAT :

    fptr = (fitsfile *)pt_file->external_info;
    stat = 0;                                /* check if key already exists */
    strcpy(keyname,descr);
    if ((nb_val == 1) || (type == CHAR)) {
      comm = (char **)malloc(sizeof(char *));	
      comm[0] = (char *)malloc((FLEN_COMMENT+1)*sizeof(char));
      strcpy(comm[0], "none");
      switch (type) {
      case CHAR :
        memset(buffer,0,4097);
        strncpy(buffer,(char *)val,nb_val);
        fits_update_key_str(fptr, keyname, buffer, comm[0], &stat);
        break;
      case SHORT :
      case INT :
      case LONG :
        fits_update_key_lng(fptr, keyname, *((int *)val), comm[0], &stat);
        break;
      case FLOAT :
        fits_update_key_flt(fptr, keyname, *((float *)val), 10, comm[0], &stat);
        break;
      case DOUBLE :
        fits_update_key_dbl(fptr, keyname, *((double *)val), 10, comm[0], &stat);
        break;
      } /* switch(type) */
      free((char *)comm[0]); free((char *)comm);
      break;
    } /* (nb_val == 1) || (type == CHAR) */
    fits_read_record(fptr,1,card,&stat); /* reset to start of header */
    strcat(keyname,"#");
    while (stat == 0)
      fits_delete_key(fptr,keyname,&stat);
    stat = 0;
    comm = (char **)malloc(nb_val*sizeof(char *));	
    for (i=0; i< nb_val; i++) {
      comm[i] = (char *)malloc((FLEN_COMMENT+1)*sizeof(char));
      if (comm[i] != NULL)
        strcpy(comm[i], "none");
    }
    switch (type) {
    case CHAR :
      *pt_val = (char *)malloc((strlen(val)+1)*sizeof(char));
      fits_write_keys_str(fptr, descr, 1, nb_val, pt_val, comm, &stat);
      break;
    case SHORT :
    case INT :
    case LONG :
      fits_write_keys_lng(fptr, descr, 1, nb_val, (long *)val, comm, &stat);
      break;
    case FLOAT :
      fits_write_keys_flt(fptr, descr, 1, nb_val, (float *)val, 10, comm, &stat);
      break;
    case DOUBLE :
      fits_write_keys_dbl(fptr, descr, 1, nb_val, (double *)val, 10, comm, &stat);
      break;
    } /* switch (type) */
    for (i=0; i< nb_val; i++) 
      if (comm[i] != NULL)
        free((char *)comm[i]);
    free((char *)comm);		
    break;          /* case FITS_A_FORMAT || FITS_B_FORMAT || EURO3D_FORMAT */
#endif
  case TIGER_FORMAT :
    stat = 0;

    descup = malloc((strlen(descr)+1)*sizeof(char));
    if (descup == NULL) {
      stat = ERR_ALLOC;
      return(stat);
    }
    strcpy(descup,descr);
    upper_strg(descup);

    if (pt_file->external_info == NULL) { /* add new descriptor */
      alloc_new_desc(pt_file,type,nb_val);
      dsc_items = (Descr_Items *)(pt_file->external_info);
      strcpy(dsc_items->descr_list[0].descr_name,descup);
      dsc_items->nb_descr++;
      i = 0;
    }
    else {
      dsc_items = (Descr_Items *)(pt_file->external_info);
      for (i=0; i< dsc_items->nb_descr 
             && strcmp(dsc_items->descr_list[i].descr_name,descup) != 0; i++);
      if (i == dsc_items->nb_descr) { /* add new descriptor */
        alloc_new_desc(pt_file,type,nb_val);
        strcpy(dsc_items->descr_list[i].descr_name,descup);
        dsc_items->nb_descr++;
      }
    }
    if (dsc_items->descr_list[i].data_type != type)
      stat = -1;
    else {
      if (dsc_items->descr_list[i].nb_values < nb_val) {
        dsc_items->descr_list[i].nb_values = nb_val;
        dsc_items->descr_list[i].descr_value.c_data = 
          realloc(dsc_items->descr_list[i].descr_value.c_data,
                  (nb_val+1)*sizeof_item(type));
      }
      if (type == CHAR) {
        if (strlen((char *)val) > nb_val) {
          strncpy(dsc_items->descr_list[i].descr_value.c_data,(char *)val,
                  nb_val);
          dsc_items->descr_list[i].descr_value.c_data[nb_val] = 0;
        }
        else
          strcpy(dsc_items->descr_list[i].descr_value.c_data,(char *)val);
      }
      else {
        memcpy(dsc_items->descr_list[i].descr_value.c_data,val,
               nb_val*sizeof_item(type));
      }
    }
    free(descup);
    break;                                   /* case TIGER_FORMAT */
  } /* switch (pt_file->data_format) */
  if (stat) {
    sprintf(errtext, "WR_desc: file= %s desc=%s", pt_file->name,descr);
    stat = get_tiger_errcode(pt_file->data_format,stat);
    Handle_Error(errtext, stat);
  }
  free(descr);
  return(stat);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                         WR_history()
!
!.purp      updates the descriptor History (or creates it if necessary)
!.desc
! WR_history(anyfile_out[,anyfile_in])
!
! (type) *anyfile_out;  image, spectrum or table (input)
! (type) *anyfile_in;   image, spectrum or table (output)
!.ed
----------------------------------------------------------------------*/

int 
WR_history(void *anyfile_out, void *anyfile_in) 
{ 
  char  *pt_hist, *pt_tmp;
  char  history[lg_hist+1];
  int   status, i, len;
  char filename[lg_name+1];
  Anyfile *pt_in, *pt_out;

  pt_in  = (Anyfile *)anyfile_in;
  pt_out = (Anyfile *)anyfile_out;
  history[0] = '\0';

  strcpy(filename,pt_out->name);
  if (exist_extension(filename) > 0) {
    return(0);                /* don't write history in each FITS extension */
  }
  /* temporairement pour eviter un core dumped (a corriger) */
  strcpy(history,Cmd_History);
  status = WR_desc(anyfile_out,"HISTORY",CHAR,lg_hist+1,history); 
  return(status);

  if (pt_in != (Anyfile *)0)
    strcpy(history,pt_in->history);
  else 
    strcpy(history,pt_out->history);

  pt_hist = history;			/* if history is too long, truncate it */
  while ((strlen(pt_hist)+strlen(Cmd_History) >lg_hist) && 
         (*pt_hist != '\0'))
    pt_hist = strpbrk(pt_hist,"\n")+1;
  len = strlen(pt_hist);
  if (pt_hist != history)
    for (i=0, pt_tmp=pt_hist; i <= len; pt_tmp++,i++) 
      history[i] = *pt_tmp;

  strcat(history,Cmd_History);
  strcpy(pt_out->history,history);
  status = WR_desc(anyfile_out,"HISTORY",CHAR,lg_hist+1,history); 
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                         delete_desc()
!
!.purp          writes values in given descriptor
!.desc
! delete_desc(anyfile,descr)
!
! (type) *anyfile;      image, spectrum or table
! char *descr;          name of descriptor
!.ed
----------------------------------------------------------------------*/

int 
delete_desc(void *anyfile, char *descr)
{
  int stat=0, i, j;
  char errtext[132];
  char *descup;
  Anyfile *pt_file;
  Descr_Items *dsc_items;

#ifdef IRAF
  char iraf_unit[lg_unit+1];
#endif

#ifdef FITS
  fitsfile *fptr;
  char card[FLEN_CARD+1];
  char keyname[FLEN_KEYWORD+1];
#endif

  pt_file = (Anyfile *)anyfile;

  switch (pt_file->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    stat = SCDDEL(pt_file->imno,descr);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len_descr = strlen(descr);
    int_type = 0;
    /* check for tables descriptors */

    if (pt_file->file_type != T_TABLE) {
      strcpy(iraf_unit,"None given");
      len_unit = strlen(iraf_unit);

      switch (type) { 	/* frames */
      }
    }
    else {
      switch (type) {		/* tables */
      }
    }
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :

    fptr = (fitsfile *)pt_file->external_info;
    stat = 0; /* check if key already exists */
    strcpy(keyname,descr);
    fits_read_record(fptr,1,card,&stat); /* reset to start of header */
    fits_delete_key(fptr,keyname,&stat);
    if (stat) {
      stat = 0;
      if (strlen(keyname) < 8) strcat(keyname,"#");
      fits_read_record(fptr,1,card,&stat); /* reset to start of header */
      while (stat == 0)
        fits_delete_key(fptr,keyname,&stat);
      stat = 0;
    }
    break;
#endif
  case TIGER_FORMAT :
    stat = 0;

    descup = malloc((strlen(descr)+1)*sizeof(char));
    if (descup == NULL) {
      stat = ERR_ALLOC;
      return(stat);
    }
    strcpy(descup,descr);
    upper_strg(descup);

    if (pt_file->external_info == NULL) break;
    dsc_items = (Descr_Items *)(pt_file->external_info);
    for (i=0; i< dsc_items->nb_descr 
           && strcmp(dsc_items->descr_list[i].descr_name,descup) != 0; i++);
    if (i >= dsc_items->nb_descr) break;
    for (j=dsc_items->nb_descr-1;j>i;j--)
      memcpy(dsc_items->descr_list+j-1,dsc_items->descr_list+j,
             sizeof(Descr_Items *));
    dsc_items->nb_descr--;
    break;
  }
  if (stat) {
    sprintf(errtext, "delete_desc: file= %s desc=%s", pt_file->name,descr);
    stat = get_tiger_errcode(pt_file->data_format,stat);
    Handle_Error(errtext, stat);
  }
  return(stat);

}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     get_all_desc()
!
!.purp             returns number of descriptors and their names 
!.desc
! get_all_desc(anyfile_in, descr_list)
!
! (type) *anyfile_in;   image, spectrum or table
! char **descr_list;    list of descriptors     
!.ed
----------------------------------------------------------------------*/

int get_all_desc(void *anyfile_in, char ***descr_list)
{
  Anyfile *pt_file_in;
  Descr_Items *dsc_items;
  int status, nb_desc, fno = 1;
  int info;
  char **pt_list;
  char desc_name[lg_ident+1];

#ifdef IRAF
  char type[2], pattern[2];
  int key_list;
  char bool;
#endif

#ifdef FITS
  fitsfile *fptr_in;
  char *pt_char;
  int nkeys, keypos, j, i;
  char card[FLEN_CARD+1];  /* standard string lengths defined in fitsioc.h */
#endif

  pt_file_in = (Anyfile *)anyfile_in;

  disable_user_warnings();

  switch (pt_file_in->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    nb_desc = 1; /* first non standard descriptor */
    while (SCDINF(pt_file_in->imno,nb_desc,fno,desc_name,lg_label,&info) == 0) {
      first_blk(desc_name);
      if (strlen(desc_name) == 0) break;
      nb_desc++;
    }
    alloc2d(descr_list,nb_desc,16,CHAR);
    pt_list = *descr_list;
    nb_desc = 1; /* first non standard descriptor */
    while (SCDINF(pt_file_in->imno,nb_desc,fno,desc_name,lg_label,&info) == 0) {
      first_blk(desc_name);
      if (strlen(desc_name) == 0) break;
      if (strlen(desc_name) > 16) desc_name[16] = 0;
      strcpy(pt_list[nb_desc-1],desc_name);
      nb_desc++;
    }
    nb_desc--;
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    nb_desc = 1; /* first non standard descriptor */
    strcpy(pattern,"*");
    bool = 0;
    /*
      udhokl(&(pt_file_in->imno),pattern,&bool,&key_list,&status,strlen(pattern));
      while (! status) {
      uhdgnk(&key_list,desc_name,&status);
      if (status) break;
      nb_desc++;
      }
      uhdckl(&key_list,&status);
    */
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
  case EURO3D_FORMAT :
	
    fptr_in = (fitsfile *)pt_file_in->external_info;

    status = 0;
    /* get no. of keywords */
    if (fits_get_hdrpos(fptr_in, &nkeys, &keypos, &status)) {
      status = ERR_READ;
      break;
    }
    alloc2d(descr_list,nkeys,FLEN_KEYWORD+1,CHAR);
    pt_list = *descr_list;
    for (i = 1, j = 0; i <= nkeys; i++)  {
      status = 0;
      if (fits_read_record(fptr_in, i, card, &status)) 
        status = ERR_READ;
      else {
        strncpy(pt_list[j],card,FLEN_KEYWORD);
        pt_char = strchr(pt_list[j],'=');
        if (pt_char == NULL)
          continue;
        else
          *pt_char = 0;
        last_char(pt_list[j]);	/* remove leading spaces */
        if (strlen(pt_list[j]) > 0)
          j++;
      }	
    }
    nb_desc = j;
    break;
#endif
  case TIGER_FORMAT :
    status = 0;
    if (pt_file_in->external_info == NULL) {
      break;
    }
    dsc_items = (Descr_Items *)(pt_file_in->external_info);
    alloc2d(descr_list,dsc_items->nb_descr,16,CHAR);
    pt_list = *descr_list;
    for (nb_desc=0; nb_desc<dsc_items->nb_descr;nb_desc++) {
      strncpy(pt_list[nb_desc],
              dsc_items->descr_list[nb_desc].descr_name,16);
      if (strlen(pt_list[nb_desc]) > 16) pt_list[nb_desc][16] = 0;
    }
    break;
  }
  restore_user_warnings();

  return(nb_desc);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     CP_non_std_desc()
!
!.purp   copies all but standard descriptors from first file to second one
!.desc
! CP_non_std_desc(anyfile_in,anyfile_out)
!
! (type) *anyfile_in;   image, spectrum or table
! (type) *anyfile_out;  image, spectrum or table
!.ed
----------------------------------------------------------------------*/

int CP_non_std_desc(void *anyfile_in, void *anyfile_out)
{
  Anyfile *pt_file_in, *pt_file_out;
  Descr_Items *dsc_items;
  int status, nb_desc, fno = 1, nb_desc_tot;
  int nb_elt, nbytes;
  short data_type;
  int info;
  char desc_name[lg_label+1], type[2];
  void *value;

#ifdef IRAF
  int key_list;
  char pattern[2];
  char bool;
#endif
#ifdef FITS
  fitsfile *fptr_in;
  fitsfile *fptr_out;
  int nkeys, keypos, j;
  int toFits;
  char card[FLEN_CARD+1];  /* standard string lengths defined in fitsioc.h */
#endif

  pt_file_in = (Anyfile *)anyfile_in;
  pt_file_out = (Anyfile *)anyfile_out;

  disable_user_warnings();

  switch (pt_file_in->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :

    /* Access to the total number of descriptors */
    SCDINF(pt_file_in->imno,nb_desc,99,desc_name,lg_label,&nb_desc_tot);

    /* Since there are 8 std descriptors, we start at the 9th */
    for (nb_desc=9; nb_desc <= nb_desc_tot;nb_desc++) {

      SCDINF(pt_file_in->imno,nb_desc,fno,desc_name,lg_label,&info);      
      first_blk(desc_name);
      status = SCDFND(pt_file_in->imno,desc_name,type,&nb_elt,&nbytes);
      if (status) break;
      switch (type[0]) {
      case 'C' : data_type = CHAR;
        break;
      case 'I' : data_type = INT;
        break;
      case 'R' : data_type = FLOAT;
        break;
      case 'D' : data_type = DOUBLE;
        break;
      default : status = ERR_BAD_TYPE;
        break;
      }
      if (status) break;
      if (data_type != CHAR) 
        value = (void *)malloc(nb_elt*nbytes);
      else
        value = (void *)malloc((nb_elt+1)*nbytes);
      RD_desc(pt_file_in,desc_name,data_type,nb_elt,value);
      if (fits_non_std_desc(desc_name))
        WR_desc(pt_file_out,desc_name,data_type,nb_elt,value);
      free(value);
    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    nb_desc = 1; /* first non standard descriptor */
    strcpy(pattern,"*");
    bool = 0;
    /*
      udhokl(&(pt_file_in->imno),pattern,&bool,&key_list,&status,strlen(pattern));
      while (! status) {
      uhdgnk(&key_list,desc_name,&status);
      if (status) break;
      *type = decode_datatype(IRAF_FORMAT,(short)datatype);
      uhdgtp(&pt_file_in->imno),desc_name,&int_type,&status);
      if (status) break;
      switch (int_type) {
      case 'C' : data_type = CHAR;
      break;
      case 'I' : data_type = INT;
      break;
      case 'R' : data_type = FLOAT;
      break;
      case 'D' : data_type = DOUBLE;
      break;
      default : status = ERR_BAD_TYPE;
      break;
      } 
      printf("[%d] desc_name=%s type=%c nb_elt=%d nbytes=%d\n",
      nb_desc,desc_name,type[0],nb_elt,nbytes);

      if (data_type != CHAR) 
      value = (void *)malloc(nb_elt*nbytes);
      else
      value = (void *)malloc((nb_elt+1)*nbytes);
      RD_desc(pt_file_in,desc_name,data_type,nb_elt,value);
      if (fits_non_std_desc(desc_name))
      WR_desc(pt_file_out,desc_name,data_type,nb_elt,value);
      free(value);
      nb_desc++;
      }
      uhdckl(&key_list,&status);case EURO3D_FORMAT :
    */
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
  case EURO3D_FORMAT :
  
    fptr_in = (fitsfile *)pt_file_in->external_info;

    if ((pt_file_out->data_format == FITS_A_FORMAT) ||
        (pt_file_out->data_format == FITS_B_FORMAT) ||
        (pt_file_out->data_format ==  EURO3D_FORMAT)) {
      toFits = 1;
      fptr_out = (fitsfile *)pt_file_out->external_info;
    }
    else
      toFits = 0;


    status = 0;
    if (TK) reset_print_progress();
		
    /* get no. of keywords */
    if (fits_get_hdrpos(fptr_in, &nkeys, &keypos, &status)) {
      status = ERR_READ;
      break;
    }
    for (j = 1; j <= nkeys; j++) {
      status = 0;
      if (fits_read_record(fptr_in, j, card, &status)) 
        status = ERR_READ;
      else {
        if (fits_non_std_desc(card)) { /* Don't bother w/ standard descriptor */
          if (toFits) {                /* Handle Fits to Fits copy directly */
		/* new code to avoid duplicating cards */
            if (!strncmp(card,"COMMENT",7) ||!strncmp(card,"HISTORY",7) ) 
              fits_write_record(fptr_out, card, &status); 
            else {
              char keyname[lg_name+1], value[lg_name+1], comment[lg_name+1], 
                card[lg_name+1];
              fits_read_keyn(fptr_in, j, keyname,value,comment,&status);
              fits_read_card(fptr_in,keyname,card,&status);
              fits_update_card(fptr_out, keyname, card, &status);
            }
                /* merge values fits_write_record(fptr_out, card, &status);  */
            continue;
          }
          if (!strncmp(card,"COMMENT",7) ||!strncmp(card,"HISTORY",7) ) 
            continue; 
          last_char_before(card,'=');
          
          if ((nb_elt = get_descr_type(pt_file_in,card, &data_type))<0) {
            if (DEBUG) print_error("Cannot identify type of descriptor %s from %s",
                        card,(fptr_in->Fptr)->filename);
            continue;
          }
          nbytes = sizeof_item(data_type);
          if (data_type != CHAR) value = (void *)malloc(nb_elt*nbytes);
          else                   value = (void *)malloc((nb_elt+1)*nbytes);
          if (TK) print_progress("Copying user descriptors",100.*j/nkeys,1);
          if (RD_desc(pt_file_in,card,data_type,nb_elt,value) > 0) {
            WR_desc(pt_file_out,card,data_type,nb_elt,value);
	 }
          else {
            if (DEBUG) print_error("Cannot read descriptor %s from %s",
                        card,(fptr_in->Fptr)->filename);
            continue;
          }
          free((char *)value);
        } /* fits_non_std_desc(card) */
      }
    }
    if (TK) print_progress("Copying user descriptors",100,1);
    break;
#endif
  case TIGER_FORMAT :
    status = 0;
    if (pt_file_in->external_info == NULL) {
      break;
    }
    dsc_items = (Descr_Items *)(pt_file_in->external_info);
    for (nb_desc=0; nb_desc<dsc_items->nb_descr;nb_desc++) {
      strcpy(desc_name,dsc_items->descr_list[nb_desc].descr_name);
      data_type = dsc_items->descr_list[nb_desc].data_type;
      nb_elt = dsc_items->descr_list[nb_desc].nb_values;
      nbytes = sizeof_item(data_type);
      if (data_type != CHAR) 
        value = (void *)malloc(nb_elt*nbytes);
      else
        value = (void *)malloc((nb_elt+1)*nbytes);
      RD_desc(pt_file_in,desc_name,data_type,nb_elt,value);
      WR_desc(pt_file_out,desc_name,data_type,nb_elt,value);
      free(value);
    }
    break;
  }
  restore_user_warnings();

  return(0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.blk                   Routines for SPECTRA i/o 
!                                      
!.func                        create_spec()
!
!.purp        creates a new 1D frame according to specifications
!.desc
! create_spec(spectrum,name,npix,start,step,datatype,ident,unit)
!
! SPECTRUM *spectrum;   spectrum structure
! char *name;           spectrum filename
! int npix;             number of pixels
! double start;         start coordinate of spectrum
! double step;          step value for spectrum
! short datatype;       type of storage 
! char *ident;          identifier
! char *unit;           units
!.ed
-------------------------------------------------------------------- */

int 
create_spec(SPECTRUM *spectrum, char *name, int npix, double start, double step, 
            short datatype, char *ident, char *unit)
{
  char errtext[132], filename[lg_name+1];
  int iomode, int_datatype;
  int one_dim=1;
  int status,SCIPUT();
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int bitpix, hdupos;
  int one = 1;
  char *extname;
  char file[lg_name+1];
#endif

  strcpy(filename,name);
  first_blk(filename);
  strcpy(spectrum->name, filename);
  append_ima_extension(spectrum->name,OutputIO.basic_io);
  strcpy(filename,spectrum->name);

  if (ASK)
    confirme_erase(filename);

  spectrum->ident[0] = '\0';
  spectrum->history[0] = '\0';
  strcpy(spectrum->cunit,"None given");
  spectrum->start = start;
  spectrum->step = step;
  spectrum->npts = npix;
  spectrum->end = start + (spectrum->npts-1)*step;
  spectrum->iomode = (int)O_MODE;
  spectrum->data_type = datatype;
  spectrum->file_type = T_IMA1D;
  spectrum->data_format = OutputIO.basic_io;
  spectrum->external_info = NULL;
  spectrum->nwcs = 0;
  spectrum->wcs = NULL;
  	
  if (ident != NULL) strcpy(spectrum->ident,ident);
  if (unit != NULL) {
    memcpy(spectrum->cunit,unit,lg_unit);
    spectrum->cunit[lg_unit] = '\0';
  }
  iomode = get_iomode_code(OutputIO.basic_io,spectrum->iomode);
  int_datatype = get_datatype_code(OutputIO.basic_io,datatype);

  switch(OutputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :	
    status = SCIPUT(filename, int_datatype, 
                    iomode, F_IMA_TYPE, one_dim, &(spectrum->npts), 
                    &(spectrum->start), 
                    &(spectrum->step),spectrum->ident, spectrum->cunit, 
                    &(spectrum->data), &(spectrum->imno));
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimdel(filename,&status,len);
    uimcre(filename,&int_datatype,&one_dim,&(spectrum->npts),
           &(spectrum->imno),&status,len);
    if (status) break;
    status = WR_desc(spectrum,"IDENT",CHAR,strlen(spectrum->ident),
                     spectrum->ident);
    if (status) break;
    status = alloc_spec_mem(spectrum, datatype);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    bitpix = fits_bitpix(datatype);
    if ((hdupos = exist(filename)) > 0)
      delete_spec(spectrum);
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname != NULL) {
      /* extension to a file; first check if the file exists, if not creates it */
      extname[0] = '\0';
      extname++;
      extname[strlen(extname)-1] = '\0';
      if (!exist(file)) {
        fits_create_file(&fptr,file,&status);
        fits_insert_img(fptr,bitpix,one_dim,(long *)&(spectrum->npts),&status);
      }
      else {
        fits_open_file(&fptr,file,iomode,&status);
        if (hdupos > 1) 
          fits_movabs_hdu(fptr,hdupos-1,NULL,&status);
        else
          fits_movabs_hdu(fptr,1,NULL,&status);
        fits_insert_img(fptr,bitpix,one_dim,(long *)&(spectrum->npts),&status);
      }
    }
    else {
      fits_create_file(&fptr,filename,&status);
      fits_create_img(fptr,bitpix,one_dim,(long *)&(spectrum->npts),&status);
    }
    spectrum->external_info = (void *)fptr;
    fits_write_key(fptr, TINT, "CRPIX1", &one, "reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CRVAL1", &(spectrum->start), 
                   "coordinate at reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CDELT1", &(spectrum->step), 
                   "coordinate increment per pixel", &status);

    if ((extname != NULL) && (!is_numeric(extname))) {
      fits_write_key(fptr, TSTRING, "EXTNAME", extname, "Extension name", &status);
    }
    if (!status) {
      status = alloc_spec_mem(spectrum, datatype);
      WR_desc(spectrum,"IDENT",CHAR,lg_ident,spectrum->ident);
    }
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"create_spec: spec %s",filename);
    status = get_tiger_errcode(spectrum->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    spectrum->wstart = spectrum->start;      
    spectrum->wend = spectrum->end;
    spectrum->iwstart = 0;      
    spectrum->iwend = spectrum->npts - 1;
    spectrum->min = 0;
    spectrum->max = 0;
    spectrum->wmin = 0;
    spectrum->wmax = 0;

    WR_desc(spectrum,"COMMENT",CHAR,8,"        ");
  }
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                            header_spec()
!
!.purp   	updates the image structure items (does not map data !)
!.desc
! header_spec(spectrum,name,mode)	
!
! SPECTRUM *spectrum;   spectrum structure
! char *name;           spectrum name
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */
int 
header_spec(SPECTRUM *spectrum, char *name,char *mode)
{
  char errtext[132], filename[lg_name+1];
  int status, nbaxes, info[5], iomode, int_datatype;
  float cuts[4];
#ifdef IRAF
  int one_dim=1, len;
#endif
#ifdef FITS
  fitsfile *fptr;
  double pixref;
#endif
	
  memset(spectrum->ident,' ',lg_ident);
  spectrum->ident[lg_ident] = '\0';
  memset(spectrum->cunit,' ',lg_unit);
  spectrum->cunit[lg_unit] = '\0';
  memset(spectrum->history,' ',lg_hist);
  spectrum->history[lg_hist] = '\0';
  spectrum->file_type = T_IMA1D;
  spectrum->data_format = InputIO.basic_io;
  spectrum->external_info = NULL;
  spectrum->data.d_data = NULL;
  spectrum->min = 0;
  spectrum->max = 0;
  spectrum->nwcs = 0;
  spectrum->wcs = NULL;

  strcpy(filename,name);
  first_blk(filename);
  strcpy(spectrum->name,filename);
  append_ima_extension(spectrum->name,InputIO.basic_io);

  strcpy(filename,spectrum->name);

  if (! exist(filename)) {
    status = ERR_OPEN;
    sprintf(errtext,"header_spec: spec %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' : 
    if (mode[1] == 'O')
      spectrum->iomode = (int)IO_MODE;
    else
      spectrum->iomode = (int)I_MODE;
    break;
  case 'O' : spectrum->iomode = (int)O_MODE;
    break;
  default  : spectrum->iomode = (int)I_MODE;
    break;
  }
	
  iomode = get_iomode_code(InputIO.basic_io,spectrum->iomode);

  switch (InputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status =  SCFINF(filename,2,info);  
    /* returns info on file storage */
    if (status == 0) {
      status = SCFOPN(filename, info[1], 0, F_IMA_TYPE,
                      &(spectrum->imno));
      nbaxes = RD_desc(spectrum,"NPIX",INT,2,&(spectrum->npts));
      if (nbaxes != 1)  {
        /* We open an image as spectrum, too bad ! */
        status = ERR_OPEN; 
        break;
      }
      RD_desc(spectrum,"START",DOUBLE,1,&(spectrum->start));
      RD_desc(spectrum,"STEP",DOUBLE,1,&(spectrum->step));
      RD_desc(spectrum,"IDENT",CHAR,lg_ident,spectrum->ident);
      RD_desc(spectrum,"CUNIT",CHAR,lg_unit,spectrum->cunit);

				
    }
    break;
#endif

#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimopn(filename,&iomode,&(spectrum->imno),&status,len);
    if (status != 0)
      break;
    uimgid(&(spectrum->imno),&int_datatype,&one_dim,&(spectrum->npts),
           &status);
    spectrum->data_type= decode_datatype(InputIO.basic_io,(short)(int_datatype));
    if (status != 0)
      break;
    disable_user_warnings();
    RD_desc(spectrum,"IDENT",CHAR,lg_ident,spectrum->ident);
    restore_user_warnings();
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    if (fits_open_file(&fptr,filename,iomode,&status)) {
      status = ERR_ACCESS; break;
    }
    spectrum->external_info = (void *)fptr;

    if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) {
      status = ERR_READ; break;
    }
    if (nbaxes != 1) {
      status = ERR_IMA_HEAD; break;
    }
    int_datatype = (fptr->Fptr->tableptr)->tdatatype;
    spectrum->data_type = decode_datatype(InputIO.basic_io,(short)int_datatype);
    if (spectrum->data_type == SHORT) {
      if (fptr->Fptr->tableptr[1].tscale == 1 && fptr->Fptr->tableptr[1].tzero == 32768)
        /* unsigned short !!! */
        spectrum->data_type = LONG;
    }
    if (fits_read_key(fptr, TINT, "NAXIS1", 
                      &(spectrum->npts), NULL, &status)) {
      status = ERR_READ; break;
    }
    pixref = 1.0;
    fits_read_key(fptr, TDOUBLE, "CRPIX1", &pixref, NULL, &status);
    if (status) { status = 0; pixref = 1; }
    fits_read_key(fptr, TDOUBLE, "CRVAL1", &(spectrum->start), NULL, &status);
    if (status) { status = 0; spectrum->start = (double)1; }
    fits_read_key(fptr, TDOUBLE, "CDELT1", &(spectrum->step), NULL, &status);
    if (status) { status = 0; spectrum->step = (double)1; }
    spectrum->start -= (pixref-1)*spectrum->step;

    break;
#endif
  }

  if (status) {
    sprintf(errtext,"header_spec: spec %s",filename);
    status = get_tiger_errcode(spectrum->data_format,status);
    Handle_Error(errtext,status);
  }
  else {	         
    disable_user_warnings();
    status = RD_desc(spectrum,"LHCUTS",FLOAT,4,cuts);
    RD_desc(spectrum,"HISTORY",CHAR,lg_hist,spectrum->history);
    restore_user_warnings();

    spectrum->end = ((spectrum->npts-1)*spectrum->step + spectrum->start);      
    spectrum->wstart = spectrum->start;      
    spectrum->wend = spectrum->end;
    spectrum->iwstart = 0;      
    spectrum->iwend = spectrum->npts - 1;
    if (status > 0) {
      spectrum->min = cuts[2];
      spectrum->max = cuts[3];
    }
    status = 0;
    spectrum->wmin = spectrum->min;
    spectrum->wmax = spectrum->max;
    /* parse wcs if contained in file */
    status = parse_wcs(spectrum);
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                            open_spec()
!
!.purp       opens a 1D frame and updates the spectrum structure items
!.desc
! open_spec(spectrum,name,mode)	
!
! SPECTRUM *spectrum;   spectrum structure
! char *name;           spectrum name
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */
int 
open_spec(SPECTRUM *spectrum, char *name,char *mode)
{
  char errtext[132], filename[lg_name+1];
  int status, nbaxes, info[5], iomode, int_datatype;
  float cuts[4];
#ifdef IRAF
  int len, one_dim=1;
#endif
#ifdef FITS
  fitsfile *fptr;
  int bitpix;
  int nbread;
  int group = 0;
  int npix;
  double pixref;
#endif
	
  memset(spectrum->ident,' ',lg_ident);
  spectrum->ident[lg_ident] = '\0';
  memset(spectrum->cunit,' ',lg_unit);
  spectrum->cunit[lg_unit] = '\0';
  memset(spectrum->history,' ',lg_hist);
  spectrum->history[lg_hist] = '\0';
  spectrum->file_type = T_IMA1D;
  spectrum->data_format = InputIO.basic_io;
  spectrum->external_info = NULL;

  strcpy(filename,name);
  first_blk(filename); 
  strcpy(spectrum->name, filename);
  append_ima_extension(spectrum->name,InputIO.basic_io);

  strcpy(filename,spectrum->name);

  if (! exist(filename)) {
    status = ERR_OPEN;
    sprintf(errtext,"open_spec: spec %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' : 
    if (mode[1] == 'O')
      spectrum->iomode = (int)IO_MODE;
    else
      spectrum->iomode = (int)I_MODE;
    break;
  case 'O' : spectrum->iomode = (int)O_MODE;
    break;
  default  : spectrum->iomode = (int)I_MODE;
    break;
  }
	
  iomode = get_iomode_code(InputIO.basic_io,spectrum->iomode);

  switch (InputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status =  SCFINF(filename,2,info);  
    /* returns info on file storage */
    if (status == 0) {
      status = SCIGET(filename, info[1], iomode, F_IMA_TYPE, 
                      1, &nbaxes, &(spectrum->npts), &(spectrum->start), 
                      &(spectrum->step), spectrum->ident, spectrum->cunit, 
                      (char **)(&(spectrum->data)), &(spectrum->imno));
      spectrum->data_type = info[1];
      spectrum->data_type = decode_datatype(InputIO.basic_io,
                                            spectrum->data_type);

      if (nbaxes!=1) /* We open an image like a spectrum, and that's not good */
        status = ERR_OPEN; 
				
    }
    break;
#endif

#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimopn(filename,&iomode,&(spectrum->imno),&status,len);
    if (status != 0)
      break;
    uimgid(&(spectrum->imno),&int_datatype,&one_dim,&(spectrum->npts),
           &status);
    spectrum->data_type= decode_datatype(InputIO.basic_io,(short)(int_datatype));
    if (status != 0)
      break;
    alloc_spec_mem(spectrum, datatype);
    switch(spectrum->data_type) {
    case SHORT :
      uigs1s(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.s_data,&status);
      break;
    case INT :
    case LONG :
      uigs1l(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.l_data,&status);
      break;
    case FLOAT :
      uigs1r(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.f_data,&status);
      break;
    case DOUBLE :
      uigs1d(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.d_data,&status);
      break;
    }
    disable_user_warnings();
    RD_desc(spectrum,"IDENT",CHAR,lg_ident,spectrum->ident);
    restore_user_warnings();
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    if (fits_open_file(&fptr,filename,iomode,&status)) {
      status = ERR_ACCESS; break;
    }
    spectrum->external_info = (void *)fptr;

    if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) {
      status = ERR_READ; break;
    }
    if (nbaxes != 1) {
      status = ERR_IMA_HEAD; break;
    }
    if (fits_read_key(fptr, TINT,"BITPIX", &bitpix,NULL, &status)) {
      status = ERR_READ; break;
    }
    int_datatype = (fptr->Fptr->tableptr)->tdatatype;
    spectrum->data_type = decode_datatype(InputIO.basic_io,(short)int_datatype);
    if (spectrum->data_type == SHORT) {
      if (fptr->Fptr->tableptr[1].tscale == 1 && fptr->Fptr->tableptr[1].tzero == 32768)
        /* unsigned short !!! */
        spectrum->data_type = LONG;
    }
    if (fits_read_key(fptr, TINT, "NAXIS1", 
                      &(spectrum->npts), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (status == 0) {
      pixref = 1;
      fits_read_key(fptr, TDOUBLE, "CRPIX1", &pixref, NULL, &status);
      if (status) { status = 0; pixref = 1.0; }
      fits_read_key(fptr, TDOUBLE, "CRVAL1", &(spectrum->start), NULL, &status);
      if (status) { status = 0; spectrum->start = (double)1; }
      fits_read_key(fptr, TDOUBLE, "CDELT1", &(spectrum->step), NULL, &status);
      if (status) { status = 0; spectrum->step = (double)1; }
      spectrum->start -= (pixref -1)*spectrum->step;
    }
    else
      break;

    if (alloc_spec_mem(spectrum, spectrum->data_type) < 0) {
      fits_close_file(fptr,&status);
      status = ERR_ALLOC;
      break;
    }

    npix = spectrum->npts;
    switch (spectrum->data_type) {
    case SHORT :
      if (fits_read_img_sht(fptr,group,1L,npix,(short)0,
                            spectrum->data.s_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case LONG :
    case INT :
      if (fits_read_img_lng(fptr,group,1L,npix,(int)0,
                            spectrum->data.l_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case FLOAT :
      if (fits_read_img_flt(fptr,group,1L,npix,(float)0,
                            spectrum->data.f_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case DOUBLE :
      if (fits_read_img_dbl(fptr,group,1L,npix,(double)0,
                            spectrum->data.d_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    }

    break;
#endif
  }

  if (status) {
    sprintf(errtext,"open_spec: spec %s",filename);
    status = get_tiger_errcode(spectrum->data_format,status);
    Handle_Error(errtext,status);
  }
  else {	         
    disable_user_warnings();
    status = RD_desc(spectrum,"LHCUTS",FLOAT,4,cuts);
    RD_desc(spectrum,"HISTORY",CHAR,lg_hist,spectrum->history);
    restore_user_warnings();

    spectrum->end = ((spectrum->npts-1)*spectrum->step + spectrum->start);      
    spectrum->wstart = spectrum->start;      
    spectrum->wend = spectrum->end;
    spectrum->iwstart = 0;      
    spectrum->iwend = spectrum->npts - 1;
	
    if (status <= 0) {
      spec_minmax(spectrum);
    }
    else {
      spectrum->min = cuts[2];
      spectrum->max = cuts[3];
    }
    status = 0;
    spectrum->wmin = spectrum->min;
    spectrum->wmax = spectrum->max;
    /* parse wcs if contained in file */
    status = parse_wcs(spectrum);
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        close_spec()
!
!.purp                closes a currently active 1D frame 
!.desc
! close_spec(spectrum)	
!
! SPECTRUM *spectrum;   spectrum structure
!.ed
-------------------------------------------------------------------- */
int 
close_spec(SPECTRUM *spectrum)			/* close active spectrum */
{
  char	errtext[132], filename[lg_name+1];
  int	stat, int_datatype;
  float cuts[4];
#ifdef IRAF
  int one=1;
#endif
#ifdef FITS
  fitsfile *fptr;
#endif

  strcpy(filename,spectrum->name);

  if (spectrum->iomode == (int)I_MODE) {
    switch (spectrum->data_format) {
#ifdef MIDAS
    case MIDAS_FORMAT :
      stat = SCFCLO(spectrum->imno);
      break;
#endif
#ifdef IRAF
    case IRAF_FORMAT :
    case STSDAS_FORMAT :
      uimclo(&(spectrum->imno),&stat);
      break;
#endif
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :

      stat =0;
      fptr = (fitsfile *)spectrum->external_info;
      fits_close_file(fptr,&stat);
      free_spec_mem(spectrum);
      spectrum->external_info = NULL;
      break;
#endif
    }
    if (stat) {
      sprintf(errtext,"close_spec: spec %s",filename);
      stat = get_tiger_errcode(spectrum->data_format,stat);
      Handle_Error(errtext,stat);
    }
    return(stat);
  }

  if (spectrum->data.d_data != NULL) {
    set_subspec(spectrum,spectrum->start,spectrum->end);
    spec_minmax(spectrum);
    cuts[0]=(float)spectrum->wmin; cuts[2]=(float)spectrum->wmin;
    cuts[1]=(float)spectrum->wmax; cuts[3]=(float)spectrum->wmax;
    stat = WR_desc(spectrum,"LHCUTS",FLOAT,4,cuts);
  }

  WR_history(spectrum, (Anyfile *)0);

  switch (spectrum->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    stat = SCFCLO(spectrum->imno);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    switch(spectrum->data_type) {
    case SHORT :
      uips1s(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.s_data,&stat);
      int_datatype = sizeof(short);
      break;
    case INT :
    case LONG :
      uips1l(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.l_data,&stat);
      int_datatype = sizeof(long);
      break;
    case FLOAT :
      uips1r(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.f_data,&stat);
      int_datatype = sizeof(float);
      break;
    case DOUBLE :
      uips1d(&(spectrum->imno),&one,&(spectrum->npts),
             spectrum->data.d_data,&stat);
      int_datatype = sizeof(double);
      break;
    }
    if (stat == 0) {
      WR_desc(spectrum,"IRAF-MIN",FLOAT,1,cuts);
      WR_desc(spectrum,"IRAF-MAX",FLOAT,1,&(cuts[1]));
      int_datatype *= 8;
      WR_desc(spectrum,"IRAF-BPX",INT,1,&int_datatype);
      uimclo(&(spectrum->imno),&stat);
    }
    free_spec_mem(spectrum);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    stat = 0;
    fptr = (fitsfile *)spectrum->external_info;
    if (spectrum->iomode != (int)I_MODE) {
      if (spectrum->data.d_data != NULL) {
        int_datatype = get_datatype_code(OutputIO.basic_io,spectrum->data_type);
        if (fits_write_img(fptr,int_datatype,1L,
                           spectrum->npts,spectrum->data.s_data,&stat)) {
          stat = ERR_WRIT; 
        }
      }
    }
    if (! stat) {
      fits_close_file(fptr,&stat);
      stat = wcs_free(spectrum);
    } 
    free_spec_mem(spectrum);
    spectrum->external_info = NULL;

    break;
#endif
  }
  if (stat) {
    sprintf(errtext,"close_spec: spec %s",filename);
    stat = get_tiger_errcode(spectrum->data_format,stat);
    Handle_Error(errtext,stat);
  } else {
    if (TK && (spectrum->iomode == O_MODE || spectrum->iomode == IO_MODE))
      {
        printf("@ N {%s}\n",filename);
      }
  }

  return(stat);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_spec()
!
!.purp                    delete a 1D frame
!.desc
! delete_spec(spectrum)	
!
! SPECTRUM *spectrum;   spectrum structure
!.ed
-------------------------------------------------------------------- */
int 
delete_spec(SPECTRUM *spectrum)			/* delete active spectrum */
{
  int status, iomode;
  char errtext[132],filename[lg_name+1];
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  char *extname;
  char file[lg_name+1];
  int hdutype, hdu_num;
#endif
	
  strcpy(filename,spectrum->name);

  switch (spectrum->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFDEL(filename);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimdel(filename,&status,len);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname == NULL) {
      if (spectrum->external_info != NULL) {
        fptr = (fitsfile *)spectrum->external_info;
        fits_delete_file(fptr,&status);
      }
      else
        status = unlink(filename);
    } else {
      extname++;
      extname[strlen(extname)-1] = '\0';
      hdutype = IMAGE_HDU;
      if (spectrum->external_info == NULL) {
        iomode = get_iomode_code(InputIO.basic_io,IO_MODE);
        fits_open_file(&fptr,filename,iomode,&status);
      }
      else
        fptr = (fitsfile *)spectrum->external_info;
      if (!is_numeric(extname)) {
        fits_movabs_hdu(fptr, 1, NULL, &status);  /* reach beginning of file */
        fits_movnam_hdu(fptr,hdutype,extname,0,&status); /* search for extname */
      }
      else {
        sscanf(extname,"%d",&hdu_num);
        fits_movabs_hdu(fptr, hdu_num+1, NULL, &status);
      }
      fits_delete_hdu(fptr,NULL,&status);
      if (spectrum->external_info == NULL) {
        fits_close_file(fptr,&status);
      }
    }
    break;
#endif
  }
  if (status)	{
    sprintf(errtext,"delete_spec: spec %s",spectrum->name);
    status = get_tiger_errcode(spectrum->data_format,status);
    Handle_Error(errtext,status);
  }
  if (TK)
    {		
      printf("@ D {%s}\n",filename);
    }
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        create_spec_mem()
!
!.purp        creates a new spectrum into memory
!.desc
! create_spec_mem(spectrum,npix,start,step,datatype)
!
! SPECTRUM *spectrum;   spectrum structure
! int npix;             number of pixels
! double start;         start coordinate of spectrum
! double step;          step value for spectrum
! short datatype;       type of storage 
!.ed
-------------------------------------------------------------------- */

int 
create_spec_mem(SPECTRUM *spectrum, int npix, double start, double step, short datatype)
{
  char errtext[132];
  int iomode, int_datatype;
  int status;

  spectrum->name[0] = '\0';
  spectrum->ident[0] = '\0';
  spectrum->history[0] = '\0';
  strcpy(spectrum->cunit,"None given");
  spectrum->start = start;
  spectrum->step = step;
  spectrum->npts = npix;
  spectrum->end = start + (spectrum->npts-1)*step;
  spectrum->iomode = (int)O_MODE;
  spectrum->data_type = datatype;
  spectrum->file_type = T_IMA1D;
  spectrum->data_format = OutputIO.basic_io;
  spectrum->external_info = NULL;
  	
  iomode = get_iomode_code(OutputIO.basic_io,spectrum->iomode);
  int_datatype = get_datatype_code(OutputIO.basic_io,datatype);

  status = alloc_spec_mem(spectrum, datatype);

  if (status) {
    sprintf(errtext,"create_spec_mem: ");
    status = get_tiger_errcode(spectrum->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    spectrum->wstart = spectrum->start;      
    spectrum->wend = spectrum->end;
    spectrum->iwstart = 0;      
    spectrum->iwend = spectrum->npts - 1;
    spectrum->min = 0;
    spectrum->max = 0;
    spectrum->wmin = 0;
    spectrum->wmax = 0;
  }
  return(status);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  routines to read/write values into spectrum files are
  given in ../incl/funcdef.h
!     
!.func                        RD_spec()
!
!.purp                  reads a spectrum value
!.desc
! (type) value = RD_spec(spectrum,pixel)
!
! SPECTRUM *spectrum;   spectrum structure
! int pixel;            pixel value to read
!.ed
!     
!.func                        WR_spec()
!
!.purp                  writes a spectrum value
!.desc
! WR_spec(spectrum,pixel,value)
!
! SPECTRUM *spectrum;   spectrum structure
! int pixel;            pixel value to write
! (type) value;         value to write
!.ed
!     
-----------------------------------------------------------------------*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                        RD_qspec()
!
!.purp                  reads a spectrum quality flag
!.desc
! unsigned long value = RD_qspec(spectrum,pixel)
!
! SPECTRUM *spectrum;   spectrum structure
! int pixel;            pixel number to read
!.ed
-----------------------------------------------------------------------*/
unsigned long RD_qspec(SPECTRUM *spectrum, int i) {

  if (spectrum->quality == NULL)
    return 0;
  else
    return spectrum->quality[i];
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                        WR_qspec()
!
!.purp                  writes a spectrum quality flag
!.desc
! WR_qspec(spectrum,pixel,value)
!
! SPECTRUM *spectrum;   spectrum structure
! int pixel;            pixel value to write
! unsigned long value;  value to write
!.ed
-----------------------------------------------------------------------*/
int WR_qspec(SPECTRUM *spectrum, int i, unsigned long pixel_val) {

  if (spectrum->quality == NULL) { /* first allocate memory */
    spectrum->quality = (unsigned long *)calloc(spectrum->npts,sizeof(unsigned long));
    if (spectrum->quality == NULL)
      return(ERR_ALLOC);
		
  }
  spectrum->quality[i] = pixel_val;
  return(0);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.blk                 Routines for bidimensional IMAGES i/o   
!     
!.func                        create_frame()
!
!.purp           creates a new 2D frame according to specifications
!.desc
! create_frame(image,name,npix,start,step,datatype,ident,unit);
!
! IMAGE2D *image;       image structure
! char  *name;          image file name
! int *npix;            number of pixels in each dim.
! double *start;        start coordinates in x and y
! double *step;         step values in x and y dim.
! short datatype;       type of data storage 
! char *ident;          identifier
! char *unit;           units
!.ed
-------------------------------------------------------------------- */

int 
create_frame(IMAGE2D *frame, char *name, int *npix, double *start, double *step, 
             short datatype, char *ident, char *unit)
{
  char errtext[132], filename[lg_name+1];
  int two_dim=2, iomode, int_datatype;
  int status, SCIPUT();
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int bitpix, hdupos;
  int one = 1;
  char *extname;
  char file[lg_name+1];
#endif

  strcpy(filename,name);
  first_blk(filename);
  strcpy(frame->name, filename);
  append_ima_extension(frame->name,OutputIO.basic_io);
	
  strcpy(filename, frame->name);

  if (ASK)
    confirme_erase(filename);

  frame->ident[0] = '\0';
  frame->history[0] = '\0';
  strcpy(frame->cunit,"None given");
  frame->startx = start[0];	frame->starty = start[1];
  frame->stepx = step[0];		frame->stepy = step[1];
  frame->endx = start[0] + (npix[0]-1)*step[0];
  frame->endy = start[1] + (npix[1]-1)*step[1];
  frame->nx = npix[0];		frame->ny = npix[1];
  frame->iomode = (int)O_MODE;
  frame->data_type = datatype;
  frame->file_type = T_IMA2D;
  frame->data_format = OutputIO.basic_io;
  frame->external_info = NULL;
  frame->nwcs = 0;
  frame->wcs = NULL;
  	
  if (ident != NULL) strcpy(frame->ident,ident);
  if (unit != NULL) {
    memcpy(frame->cunit,unit,lg_unit);
    frame->cunit[lg_unit] = '\0';
  }
  	
  iomode = get_iomode_code(OutputIO.basic_io,frame->iomode);
  int_datatype = get_datatype_code(OutputIO.basic_io,datatype);

  switch(OutputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCIPUT(filename, int_datatype, iomode, 
                    F_IMA_TYPE, two_dim, &(frame->nx), &(frame->startx), 
                    &(frame->stepx),frame->ident, frame->cunit, &(frame->data), 
                    &(frame->imno));
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimdel(filename,&status,len);
    uimcre(filename,&int_datatype,&two_dim,&(frame->nx),
           &(frame->imno),&status,len);
    if (status) break;
    status = WR_desc(frame,"IDENT",CHAR,strlen(frame->ident),
                     frame->ident);
    if (status) break;
    status = alloc_frame_mem(frame, datatype);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    bitpix = fits_bitpix(datatype);
    if ((hdupos = exist(filename)) > 0)
      delete_frame(frame);
    strcpy(file,filename);
    extname = strchr(file,'[');
    /* extension to a file; first check if the file exists, if not creates it */
		
    if (extname != NULL) {
      /* extension to a file; first check if the file exists, if not creates it */
      extname[0] = '\0';
      extname++;
      extname[strlen(extname)-1] = '\0';
      if (!exist(file)) {
        fits_create_file(&fptr,file,&status);
        fits_insert_img(fptr,bitpix,two_dim,(long *)&(frame->nx),&status);
      }
      else {
        fits_open_file(&fptr,file,iomode,&status);
        if (hdupos > 1) { /* the extension was deleted */
          fits_movabs_hdu(fptr,hdupos-1,NULL,&status);
          fits_insert_img(fptr,bitpix,two_dim,(long *)&(frame->nx),&status);
        }
        else {
          if (hdupos == 1) {
            status = PREPEND_PRIMARY;
            fits_insert_img(fptr,bitpix,two_dim,(long *)&(frame->nx),&status);
          } else
            fits_create_img(fptr,bitpix,two_dim,(long *)&(frame->nx),&status);
        }
      }
    }
    else {
      fits_create_file(&fptr,filename,&status);
      fits_create_img(fptr,bitpix,two_dim,(long *)&(frame->nx),&status);
    }
    frame->external_info = (void *)fptr;
    fits_write_key(fptr, TINT, "CRPIX1", &one, "reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CRVAL1", &(frame->startx), 
                   "coordinate at reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CDELT1", &(frame->stepx), 
                   "coordinate increment per pixel", &status);
    fits_write_key(fptr, TINT, "CRPIX2", &one, "reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CRVAL2", &(frame->starty), 
                   "coordinate at reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CDELT2", &(frame->stepy), 
                   "coordinate increment per pixel", &status);

    if ((extname != NULL) && (!is_numeric(extname))) {
      fits_write_key(fptr, TSTRING, "EXTNAME", extname, "Extension name", &status);
    }
    if (!status) {
      status = alloc_frame_mem(frame, datatype);
      WR_desc(frame,"IDENT",CHAR,lg_ident,frame->ident);
    }
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"create_frame: frame %s",filename);
    status = get_tiger_errcode(frame->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    frame->min = 0;
    frame->max = 0;
    WR_desc(frame,"COMMENT",CHAR,8,"       ");
  }
  return(status);
} 
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        header_frame()
!
!.purp   	updates the image structure items (does not map data !)
!.desc
! header_frame(frame,name)	
!
! IMAGE2D *frame;       image structure
! char *name;           frame name
!.ed
-------------------------------------------------------------------- */
int 
header_frame(IMAGE2D *frame, char *name, char *mode)		
{
  char errtext[132], filename[lg_name+1];
  int status, nbaxes, iomode, int_datatype;
  float cuts[4];
  int info[5];
#ifdef IRAF
  int two_dim=2;
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int bitpix;
  double pixref;
#endif

  memset(frame->ident,' ',lg_ident);
  frame->ident[lg_ident] = '\0';
  memset(frame->cunit,' ',lg_unit);
  frame->cunit[lg_unit] = '\0';
  memset(frame->history,' ',lg_hist);
  frame->history[lg_hist] = '\0';
  frame->external_info = NULL;
  frame->file_type = T_IMA2D;
  frame->data_format = InputIO.basic_io;
  frame->data.d_data = NULL;
  frame->min = 0;
  frame->max = 0;
  frame->nwcs = 0;
  frame->wcs = NULL;

  strcpy(filename,name);
  first_blk(filename); 
  strcpy(frame->name,filename);
  append_ima_extension(frame->name,InputIO.basic_io);

  strcpy(filename,frame->name);

  if (!exist(filename)) {
    status = ERR_OPEN;
    sprintf(errtext,"header_frame: frame %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' : 
    if (mode[1] == 'O')
      frame->iomode = (int)IO_MODE;
    else
      frame->iomode = (int)I_MODE;
    break;
  case 'O' : frame->iomode = (int)O_MODE;
    break;
  default  : frame->iomode = (int)I_MODE;
    break;
  }
	
  iomode = get_iomode_code(InputIO.basic_io,frame->iomode);

  switch (InputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFINF(filename,2,info);  
    if (status == 0) {
      status = SCFOPN(filename, info[1], 0, F_IMA_TYPE,
                      &(frame->imno));
      nbaxes = RD_desc(frame,"NPIX",INT,2,&(frame->nx));
      if (nbaxes != 2) {
        status = ERR_OPEN; 
        break;
      }

      RD_desc(frame,"START",DOUBLE,2,&(frame->startx));
      RD_desc(frame,"STEP",DOUBLE,2,&(frame->stepx));
      RD_desc(frame,"IDENT",CHAR,lg_ident,frame->ident);
      RD_desc(frame,"CUNIT",CHAR,lg_unit,frame->cunit);
      frame->data_type = info[1];
      frame->data_type = decode_datatype(InputIO.basic_io,frame->data_type);

    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimopn(filename,&iomode,&(frame->imno),&status,len);
    if (status != 0) 
      break;
    uimgid(&(frame->imno),&int_datatype,&two_dim,&(frame->nx),&status);
    frame->data_type = decode_datatype(InputIO.basic_io,(short)(int_datatype));
    disable_user_warnings();
    RD_desc(frame,"IDENT",CHAR,lg_ident,frame->ident);
    restore_user_warnings();
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    if (fits_open_file(&fptr,filename,iomode,&status)) {
      status = ERR_ACCESS; break;
    }
    frame->external_info = (void *)fptr;
    if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) {
      status = ERR_READ; break;
    }
    if (nbaxes != 2) {
      status = ERR_IMA_HEAD; break;
    }
    if (fits_read_key(fptr, TINT,"BITPIX", &bitpix,NULL, &status)) {
      status = ERR_READ; break;
    }
    int_datatype = (fptr->Fptr->tableptr)->tdatatype;
    frame->data_type = decode_datatype(InputIO.basic_io,(short)int_datatype);
    if (frame->data_type == SHORT) {
      if (fptr->Fptr->tableptr[1].tscale == 1 && fptr->Fptr->tableptr[1].tzero == 32768)
        /* unsigned short !!! */
        frame->data_type = LONG;
    }
    if (fits_read_key(fptr, TINT, "NAXIS1",
                      &(frame->nx), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS2",
                      &(frame->ny), NULL, &status)) {
      status = ERR_READ; break;
    }
    pixref = 1;
    fits_read_key(fptr, TDOUBLE, "CRPIX1", &pixref, NULL, &status);
    if (status) { status = 0; pixref = 1; }
    fits_read_key(fptr, TDOUBLE, "CRVAL1", &(frame->startx), NULL, &status);
    if (status) { status = 0; frame->startx = (double)1; }
    fits_read_key(fptr, TDOUBLE, "CDELT1", &(frame->stepx), NULL, &status);
    if (status) { status = 0; frame->stepx = (double)1; }
    frame->startx -= (pixref-1)*frame->stepx;
    pixref = 1;
    fits_read_key(fptr, TDOUBLE, "CRPIX2", &pixref, NULL, &status);
    if (status) { status = 0; pixref = 1; }
    fits_read_key(fptr, TDOUBLE, "CRVAL2", &(frame->starty), NULL, &status);
    if (status) { status = 0; frame->starty = (double)1; }
    fits_read_key(fptr, TDOUBLE, "CDELT2", &(frame->stepy), NULL, &status);
    if (status) { status = 0; frame->stepy = (double)1; }
    frame->starty -= (pixref-1)*frame->stepy;
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"header_frame: frame %s",filename);
    status = get_tiger_errcode(frame->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    disable_user_warnings();
    status = RD_desc(frame,"LHCUTS",FLOAT,4,cuts);
    RD_desc(frame,"HISTORY",CHAR,lg_hist,frame->history);
    restore_user_warnings();

    frame->endx = frame->startx + (frame->nx -1)*frame->stepx;
    frame->endy = frame->starty + (frame->ny -1)*frame->stepy;
    if (status > 0) {
      frame->min = cuts[2];
      frame->max = cuts[3];
    }
    status = 0;
    /* parse wcs if contained in file */
    status = parse_wcs(frame);
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        open_frame()
!
!.purp          opens a 2D frame and updates the image structure items
!.desc
! open_frame(frame,name,mode)	
!
! IMAGE2D *frame;       image structure
! char *name;           frame name
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */
int 
open_frame(IMAGE2D *frame, char *name, char *mode)		
{
  char errtext[132], filename[lg_name+1];
  int status, nbaxes, iomode, int_datatype;
  float cuts[4];
  int info[5];
#ifdef IRAF
  int two_dim=2;
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int nbread;
  int npix;
  int group = 0;
  double pixref;
#endif

  memset(frame->ident,' ',lg_ident);
  frame->ident[lg_ident] = '\0';
  memset(frame->cunit,' ',lg_unit);
  frame->cunit[lg_unit] = '\0';
  memset(frame->history,' ',lg_hist);
  frame->history[lg_hist] = '\0';
  frame->external_info = NULL;
  frame->file_type = T_IMA2D;
  frame->data_format = InputIO.basic_io;

  strcpy(filename,name);
  first_blk(filename); 
  strcpy(frame->name,filename);
  append_ima_extension(frame->name,InputIO.basic_io);

  strcpy(filename,frame->name);

  if (!exist(filename)) { /* check if fil exists */
    status = ERR_OPEN;
    sprintf(errtext,"open_frame: frame %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' : 
    if (mode[1] == 'O')
      frame->iomode = (int)IO_MODE;
    else
      frame->iomode = (int)I_MODE;
    break;
  case 'O' : frame->iomode = (int)O_MODE;
    break;
  default  : frame->iomode = (int)I_MODE;
    break;
  }
	
  iomode = get_iomode_code(InputIO.basic_io,frame->iomode);

  switch (InputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFINF(filename,2,info);  
    if (status == 0) {
      status = SCIGET(filename, info[1], iomode, F_IMA_TYPE, 2, 
                      &nbaxes, &(frame->nx), &(frame->startx), &(frame->stepx),
                      frame->ident, frame->cunit, (char **)(&(frame->data)), 
                      &(frame->imno));
      frame->data_type = info[1];
      frame->data_type = decode_datatype(InputIO.basic_io,frame->data_type);

      if (nbaxes!=2) /* We open a spectrum like an image, and that's not good */
        status = ERR_OPEN; 

    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimopn(filename,&iomode,&(frame->imno),&status,len);
    if (status != 0) 
      break;
    uimgid(&(frame->imno),&int_datatype,&two_dim,&(frame->nx),&status);
    frame->data_type = decode_datatype(InputIO.basic_io,(short)(int_datatype));
    if (status != 0)
      break;
    alloc_frame_mem(frame, datatype);
    switch(frame->data_type) {
    case SHORT :
      uigs2s(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.s_data,&status);
      break;
    case INT :
    case LONG :
      uigs2l(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.l_data,&status);
      break;
    case FLOAT :
      uigs2r(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.f_data,&status);
      break;
    case DOUBLE :
      uigs2d(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.d_data,&status);
      break;
    }
    disable_user_warnings();
    RD_desc(frame,"IDENT",CHAR,lg_ident,frame->ident);
    restore_user_warnings();
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    if (fits_open_file(&fptr,filename,iomode,&status)) {
      status = ERR_ACCESS; break;
    }
    frame->external_info = (void *)fptr;
    if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) {
      status = ERR_READ; break;
    }
    if (nbaxes != 2) {
      status = ERR_IMA_HEAD; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS1",
                      &(frame->nx), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS2",
                      &(frame->ny), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (status == 0) {
      pixref = 1.0;
      fits_read_key(fptr, TDOUBLE, "CRPIX1", &pixref, NULL, &status);
      if (status) { status = 0; pixref = 1; }
      fits_read_key(fptr, TDOUBLE, "CRVAL1", &(frame->startx), NULL, &status);
      if (status) { status = 0; frame->startx = (double)1; }
      fits_read_key(fptr, TDOUBLE, "CDELT1", &(frame->stepx), NULL, &status);
      if (status) { status = 0; frame->stepx = (double)1; }
      frame->startx -= (pixref-1)*frame->stepx;
      pixref = 1.0;
      fits_read_key(fptr, TDOUBLE, "CRPIX2", &pixref, NULL, &status);
      if (status) { status = 0; pixref = 1; }
      fits_read_key(fptr, TDOUBLE, "CRVAL2", &(frame->starty), NULL, &status);
      if (status) { status = 0; frame->starty = (double)1; }
      fits_read_key(fptr, TDOUBLE, "CDELT2", &(frame->stepy), NULL, &status);
      if (status) { status = 0; frame->stepy = (double)1; }
      frame->starty -= (pixref-1)*frame->stepy;
    }
    else
      break;

    int_datatype = (fptr->Fptr->tableptr)->tdatatype;
    frame->data_type = decode_datatype(InputIO.basic_io,(short)int_datatype);
    if (frame->data_type == SHORT) {
      if (fptr->Fptr->tableptr[1].tscale == 1 && fptr->Fptr->tableptr[1].tzero == 32768)
        /* unsigned short !!! */
        frame->data_type = LONG;
    }

    if (alloc_frame_mem(frame, frame->data_type) < 0) {
      fits_close_file(fptr,&status);
      status = ERR_ALLOC;
      break;
    }

    npix = frame->nx*frame->ny;
    switch (frame->data_type) {
    case SHORT :
      if (fits_read_img_sht(fptr,group,1L,npix,(short)0,
                            frame->data.s_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case LONG :
    case INT :
      if (fits_read_img_lng(fptr,group,1L,npix,(int)0,
                            frame->data.l_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case FLOAT :
      if (fits_read_img_flt(fptr,group,1L,npix,(float)0,
                            frame->data.f_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case DOUBLE :
      if (fits_read_img_dbl(fptr,group,1L,npix,(double)0,
                            frame->data.d_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    }
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"open_frame: frame %s",filename);
    status = get_tiger_errcode(frame->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    disable_user_warnings();
    status = RD_desc(frame,"LHCUTS",FLOAT,4,cuts);
    RD_desc(frame,"HISTORY",CHAR,lg_hist,frame->history);
    restore_user_warnings();

    frame->endx = frame->startx + (frame->nx -1)*frame->stepx;
    frame->endy = frame->starty + (frame->ny -1)*frame->stepy;
    if (status <= 0) {
      image_minmax(frame);
    }
    else {
      frame->min = cuts[2];
      frame->max = cuts[3];
    }
    status = 0;
    /* parse wcs if contained in file */
    status = parse_wcs(frame);
  }
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                       close_frame()
!
!.purp             closes a currently active 2D frame 
!.desc
! close_frame(frame)	
!
! IMAGE2D *frame;       image structure
!.ed
-------------------------------------------------------------------- */
int 
close_frame(IMAGE2D *frame)			/* close active frame */
{
  char  errtext[132], filename[lg_name+1];
  int   stat, int_datatype;
  float cuts[4];
#ifdef IRAF
  int one=1;
#endif
#ifdef FITS
  fitsfile *fptr;
  int npix;
#endif

  strcpy(filename,frame->name);

  if (frame->iomode == (int)I_MODE) {
    switch (frame->data_format) {
#ifdef MIDAS
    case MIDAS_FORMAT :
      stat = SCFCLO(frame->imno);
      break;
#endif
#ifdef IRAF
    case IRAF_FORMAT :
    case STSDAS_FORMAT :
      uimclo(&(frame->imno),&stat);
      break;
#endif
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :
      stat =0;
      fptr = (fitsfile *)frame->external_info;
      fits_close_file(fptr,&stat);
      free_frame_mem(frame);
      frame->external_info = NULL;
      break;
#endif
    }
    if (stat) {
      sprintf(errtext,"close_frame: frame %s",filename);
      stat = get_tiger_errcode(frame->data_format,stat);
      Handle_Error(errtext,stat);
    }
    return(stat);
  }

  if (frame->data.d_data != NULL) {
    image_minmax(frame);

    cuts[0]=(float)frame->min; cuts[2]=(float)frame->min;
    cuts[1]=(float)frame->max; cuts[3]=(float)frame->max;
    stat = WR_desc(frame,"LHCUTS",FLOAT,4,cuts);
  }

  WR_history(frame, (Anyfile *)0);

  switch (frame->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    stat = SCFCLO(frame->imno);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    switch(frame->data_type) {
    case SHORT :
      uips2s(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.s_data,&stat);
      break;
    case INT :
    case LONG :
      uips2l(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.l_data,&stat);
      break;
    case FLOAT :
      uips2r(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.f_data,&stat);
      break;
    case DOUBLE :
      uips2d(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
             frame->data.d_data,&stat);
      break;
    }
    if (stat == 0)  
      uimclo(&(frame->imno),&stat);
    free_frame_mem(frame);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    stat = 0;
    fptr = (fitsfile *)frame->external_info;
    if (frame->iomode != (int)I_MODE) {
      if (frame->data.d_data != NULL) {
        int_datatype = get_datatype_code(OutputIO.basic_io,frame->data_type);
        npix = frame->nx*frame->ny;
        if (fits_write_img(fptr,int_datatype,1L,npix,
                           frame->data.s_data,&stat)) {
          stat = ERR_WRIT;
        }
      }
    }
    if (! stat) {
      fits_close_file(fptr,&stat);
      stat = wcs_free(frame);
    }
    free_frame_mem(frame);
    frame->external_info = NULL;
    break;
#endif
  }
  if (stat) {
    sprintf(errtext,"close_frame: frame %s",filename);
    stat = get_tiger_errcode(frame->data_format,stat);
    Handle_Error(errtext,stat);
  } else {
    if (TK && (frame->iomode == O_MODE || frame->iomode == IO_MODE))
      {
        printf("@ N {%s}\n",filename);
      }
  }

  return(stat);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_frame()
!
!.purp                    deletes a 2D frame 
!.desc
! delete_frame(frame)	
!
! IMAGE2D *frame;       image structure
!.ed
-------------------------------------------------------------------- */

int 
delete_frame(IMAGE2D *frame)		
{
  int status;
  char errtext[132],filename[lg_name+1];
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int iomode;
  char *extname;
  char file[lg_name+1];
  int hdutype, hdu_num;
#endif
	
  strcpy(filename,frame->name);

  switch (frame->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFDEL(filename);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimdel(filename,&status);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname == NULL) {
      if (frame->external_info != NULL) {
        fptr = (fitsfile *)frame->external_info;
        fits_delete_file(fptr,&status);
      }
      else
        status = unlink(filename);
    } else {
      extname++;
      extname[strlen(extname)-1] = '\0';
      hdutype = IMAGE_HDU;
      if (frame->external_info == NULL) {
        iomode = get_iomode_code(InputIO.basic_io,IO_MODE);
        fits_open_file(&fptr,filename,iomode,&status);
      }
      else
        fptr = (fitsfile *)frame->external_info;
      if (!is_numeric(extname)) {
        fits_movabs_hdu(fptr, 1, NULL, &status);  /* reach beginning of file */
        fits_movnam_hdu(fptr,hdutype,extname,0,&status); /* search for extname */
      }
      else {
        sscanf(extname,"%d",&hdu_num);
        fits_movabs_hdu(fptr, hdu_num+1, NULL, &status); 
      }
      fits_delete_hdu(fptr,NULL,&status);
      if (frame->external_info == NULL) {
        fits_close_file(fptr,&status);
      }
    }
    break;
#endif
  }
  if (status)	{
    sprintf(errtext,"delete_frame: frame %s",filename);
    status = get_tiger_errcode(frame->data_format,status);
    Handle_Error(errtext,status);
  }
  if (TK)
    {		
      printf("@ D {%s}\n",filename);
    }
  return(status);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  routines to read/write values into image files are
  given in ../incl/funcdef.h
!     
!.func                        RD_frame()
!
!.purp                  reads a frame value
!.desc
! (type) value = RD_frame(image,x_pixel,y_pixel)
!
! IMAGE2D *image;       image structure
! int x_pixel;          pixel to read along x axis
! int y_pixel;          pixel to read along y axis
!.ed
!     
!.func                        WR_frame()
!
!.purp                  writes a frame value
!.desc
! WR_frame(image,pixel_x,pixel_y,value)
!
! IMAGE2D *image;       image structure
! int x_pixel;          pixel to write along x axis
! int y_pixel;          pixel to write along y axis
! (type) value;         value to write
!.ed
-----------------------------------------------------------------------*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                        RD_qframe()
!
!.purp                  reads a frame quality flag
!.desc
! unsigned long value = RD_qframe(image,x_pixel,y_pixel)
!
! IMAGE2D *image;       image structure
! int x_pixel;          pixel to read along x axis
! int y_pixel;          pixel to read along y axis
!.ed
-----------------------------------------------------------------------*/
unsigned long RD_qframe(IMAGE2D *frame, int i, int j) 
{
  if (frame->quality == NULL)
    return 0;
  else
    return frame->quality[i+frame->nx*j];
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!     
!.func                        WR_qframe()
!
!.purp                  writes a frame quality flag
!.desc
! WR_qframe(image,pixel_x,pixel_y,value)
!
! IMAGE2D *image;       image structure
! int x_pixel;          pixel to write along x axis
!.ed
-----------------------------------------------------------------------*/
int WR_qframe(IMAGE2D *frame, int i, int j, unsigned long pixel_val) 
{
  if (frame->quality == NULL) { /* first allocate memory */
    frame->quality = (unsigned long *)calloc(frame->nx*frame->ny,sizeof(unsigned long));
    if (frame->quality == NULL)
      return(ERR_ALLOC);
		
  }
  frame->quality[i+frame->nx*j] = pixel_val;
  return(0);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.blk                 Routines for TABLES i/o  	            
!                                                                    
!.func                    create_table()
!
!.purp     creates a new table-file according to specifications
!.desc
! create_table(table,name,nbrow,nbcol,mode,ident) 
!
! TABLE *table;         table structure
! char *name;           table filename
! int nbrow;            number of rows (-1 if unknown)
! int nbcol;            number of columns (-1 if u.)
! char mode;            mode = N(ew),I(nquiry),W(arning),Q(uiet)
! char *ident;          identifier
!.ed
----------------------------------------------------------------------*/
int 
create_table(TABLE *table, char *name, int nbrow, int nbcol, char mode, char *ident)
{
  int TCTINI();
  char ok[10];
  char errtext[132], filename[lg_name+1]; 
  int iomode, end;
  short already_exist=0;
  int  status, nb_row=-1, nb_col=-1;
#ifdef IRAF
  int param;
#endif

#ifdef FITS
  fitsfile *fptr;
  int hdutype;
  int one = 1;
  char **cttype = NULL;
  char **ctform = NULL;
  char **ctunit = NULL;
  char *extname;
  char file[lg_name+1];
#endif

  strcpy(filename,name);
  first_blk(filename);
  strcpy(table->name,filename);
  append_tbl_extension(table->name,OutputIO.basic_io);
  memset(table->ident,' ',lg_ident);
  table->ident[lg_ident] = '\0';
  table->file_type = T_TABLE;
  table->iomode = (int)O_MODE;
  table->data_format = InputIO.basic_io;
  table->external_info = NULL;
  table->allrow = 0;
  table->row = 0;
  table->col = 0;
  table->sel_row = NULL;
  table->nwcs = 0;
  table->wcs = NULL;
		
  if (nbrow > 0) nb_row = nbrow;
  if (nbcol > 0) nb_col = nbcol;

  strcpy(filename,table->name);

  if (exist(filename)) 
    already_exist = 1;

  if (already_exist) {
    switch(mode) {
      /* Warning	*/
    case 'W' :	
      if (VERBOSE) {
        sprintf(errtext,"Table %s overwritten",name);
        print_warning(errtext);
      }
      break;
      /* Inquire before overwritting */
    case 'I' :
      if (ASK) {
        if (!TK)
          {
            sprintf(errtext,"Do you want to overwrite table %s (y/n)? ",name);
            print_msg(errtext);
            scanf("%s",ok); upper_strg(ok);
            if (ok[0] != 'Y') stop_by_user();
          }
        else
          confirme_erase(filename);
      }
      break;
  					
    case 'N' : 
      sprintf(errtext,"create_table: table %s already exists",name);
      Handle_Error(errtext,ERR_REN_TBL);
      return(ERR_REN_TBL);
      break;

    case 'Q' : 
      break;
      /* Unrecognize option */
  					
    default  : 
      sprintf(errtext,"create_table (%s): unknown option",name);
      Handle_Error(errtext,ERR_BAD_PARAM);
      return(ERR_BAD_PARAM);
      break;		
    }
    delete_table(table);
  }
	
  table->select_flag = FALSE;
  table->file_type = T_TABLE;
  table->data_format = OutputIO.basic_io;

  iomode = get_iomode_code(OutputIO.basic_io,O_MODE);
  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    if (nb_row <= 0) nb_row = tble_nb_row;
    if (nb_col <= 0) nb_col= -1;			/* MIDAS default */
    status= TCTINI(filename,F_TRANS,iomode,nb_col,nb_row,&(table->imno));
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(name);
    uttinn(name,&(table->imno),&status, len);
    if (status != 0) break;
    param = TBMXCL;	/* default maximum nber of columns */
    if (nb_col <= 0) nb_col = 10;
    utppti(&(table->imno),&param,&nb_col,&status);
    if (status != 0) break;
    uttcre(&(table->imno),&status);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    if (table->data_format == FITS_A_FORMAT)
      hdutype = ASCII_TBL;
    else
      hdutype = BINARY_TBL;
    status = 0;
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname == NULL) {
      fits_create_file(&fptr,file,&status);
      fits_create_img(fptr,8,0,(long *)&one, &status);
    }
    else {
      extname[0] = '\0';
      extname++;
      extname[strlen(extname)-1] = '\0';

      if (!exist(file))
        fits_create_file(&fptr,file,&status);
      else 
        fits_open_file(&fptr,file,iomode,&status);
      if (status) {
        status = ERR_OPEN;
        break;
      }

      fits_movabs_hdu(fptr, 1, NULL, &status);
      fits_movnam_hdu(fptr,hdutype,"",0,&status);
      if (status) {
        status = 0;
      }
      else { /* delete current extension */
        fits_delete_hdu(fptr, &hdutype, &status);
      }
      fits_get_num_hdus(fptr,&end,&status);	/* reach end of file */
      fits_movabs_hdu(fptr,end,NULL,&status);

    }
    table->external_info = (void *)fptr;

    nb_row = MAX(0,nb_row);
    if (table->data_format == FITS_A_FORMAT) {

      if (fits_insert_atbl(fptr, 132, nb_row, 0, cttype,
                           (long *)&one,ctform, ctunit, extname, &status)) {
        status = ERR_WRIT; break;
      }
    }
    else {
      if (fits_insert_btbl(fptr, nb_row, 0, cttype,
                           ctform, ctunit, extname, 1, &status)) {
        status = ERR_WRIT; break;
      }
    }

    if (status) {
      status = ERR_WRIT; break;
    }
    else {
      table->allrow = nb_row;
    }
    break;
#endif
  }
  if (status) {
    sprintf(errtext,"create_table: table %s",filename);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(errtext,status);
    return(status);	
  }

  if (ident != NULL) {
    strcpy(table->ident,ident);
    WR_desc(table,"COMMENT",CHAR,8,"        ");
    status = WR_desc(table,"IDENT",CHAR,strlen(ident),table->ident); 
    if (status) {
      sprintf(errtext,"create_table: table %s",filename);
      status = get_tiger_errcode(table->data_format,status);
      Handle_Error(errtext,status);
    }
  } 
  return(status);	
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                    open_table()
!
!.purp     opens an existing table-file and updates structure
!.desc
! open_table(table,name,mode) 
!
! TABLE *table;         table structure
! char  *name;          table name
! char  *mode;          opening mode (I,O,IO)
!.ed
----------------------------------------------------------------------*/
int 
open_table(TABLE *table, char *name, char *mode)
{
  int allcol, allrow, nsort, status, iomode;
  char errtext[132], filename[lg_name+1];
  short already_exist=1;
#ifdef IRAF
  int lens, tbnrow, tbncol;
#endif
#ifdef FITS
  fitsfile *fptr;
  int hdutype;
#endif

  table->select_flag = FALSE;
  memset(table->ident,' ',lg_ident);
  table->ident[lg_ident] = '\0';
  memset(table->history,' ',lg_hist);
  table->history[lg_hist] = '\0';
  table->file_type = T_TABLE;
  table->data_format = InputIO.basic_io;
  table->external_info = NULL;
  table->sel_row = NULL;
  table->nwcs = 0;
  table->wcs = NULL;

  strcpy(filename,name);
  first_blk(filename);
  strcpy(table->name,filename);
  append_tbl_extension(table->name,InputIO.basic_io);

  strcpy(filename,table->name);

  if (!exist(filename)) {
    status = ERR_OPEN;
    sprintf(errtext,"open_table: table %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' : 
    if (mode[1] == 'O') 
      table->iomode = (int)IO_MODE;
    else
      table->iomode = (int)I_MODE;
    break;
  case 'O' : table->iomode = (int)O_MODE;
    break;
  default  : table->iomode = (int)I_MODE;
    break;
  }

  if (!exist(filename)) 
    already_exist = 0;

  if (already_exist == 0) {
    status = ERR_NOTBL;
    sprintf(errtext,"open_table: %s doesn't exist",filename);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(errtext,status);
    return(status);
  }

  iomode = get_iomode_code(InputIO.basic_io,table->iomode);

  switch (InputIO.basic_io) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCTOPN(filename,iomode, &(table->imno));
    if (status == 0) {
      status = TCIGET(table->imno,&(table->col), &(table->row),
                      &nsort, &allcol, &allrow);
    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    tbnrow = TBNROW;
    tbncol = TBNCOL;
    len = strlen(filename);
    uttopn(filename,&iomode,&(table->imno),&status,len);
    if (status == 0) {
      utpgti(&(table->imno),&tbnrow, &(table->row),&status);
      utpgti(&(table->imno),&tbncol, &(table->col),&status);
    }
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    if (fits_open_file(&fptr, filename, iomode, &status)) {
      status = ERR_OPEN;
      break;
    }
    table->external_info = (void *)fptr;
    if (fits_get_hdu_type(fptr, &hdutype, &status)) {
      fits_close_file(fptr,&status);
      status = ERR_BAD_HEAD;
      break;
    }
    switch (hdutype) {
    case ASCII_TBL :
    case BINARY_TBL :
      break;
    default:
      if (fits_movrel_hdu(fptr, 1, &hdutype, &status)) {
        fits_close_file(fptr,&status);
        status = ERR_BAD_HEAD;
      }
      break;
    }

    if (hdutype == ASCII_TBL)
      table->data_format = FITS_A_FORMAT;
    else 
      table->data_format = FITS_B_FORMAT;
    table->col = fptr->Fptr->tfield;
    if (fits_read_key(fptr, TINT, "NAXIS2", 
                      &(table->row), NULL, &status)) {
      fits_close_file(fptr,&status);
      status = ERR_READ; break;
    }
    break;
#endif
  }
  if (status) {
    sprintf(errtext,"open_table: table %s",filename);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    table->allrow = table->row;
    disable_user_warnings();
    RD_desc(table,"IDENT",CHAR,lg_ident,table->ident);
    /* YC: remove HISTORY, as it is not used anymore */
    /* RD_desc(table,"HISTORY",CHAR,lg_hist,table->history); */
    restore_user_warnings();
  }
  return(status);		    
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                    close_table()
!
!.purp             closes a currently active table
!.desc
! close_table(table) 
!
! TABLE *table;         table structure
!.ed
----------------------------------------------------------------------*/

int 
close_table(TABLE *table)
{
  int	status, l, nbl, sel=0,iomode;
  int	*selline;
  char select_mode[133], filename[lg_name+1];

#ifdef FITS
  fitsfile *fptr;
#endif

  strcpy(filename,table->name);

  if (table->iomode != I_MODE) 
    WR_history(table, (Anyfile *)0);

  switch(table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    if (table->select_flag) {

      /* if selected, need to close and reopen the table with
         previous selecction since Midas unselect the whole table 
         when a column is created */

      sel = 1;
      selline = (int *)malloc(table->row*sizeof(int));
      nbl = table->row;
      status = RD_desc(table,"TSELTABL",CHAR,132,select_mode);
      if (status > 0)
        for (l=0; l<nbl; l++) 
          TCSGET(table->imno,l+1,&(selline[l]));
    }
    status = TCTCLO(table->imno);
    if (sel) {
      iomode = get_iomode_code(table->data_format,table->iomode);
      TCTOPN(filename, iomode, &(table->imno));
      for (l=0; l<nbl; l++) 
        TCSPUT(table->imno,l+1,&(selline[l]));
      status = WR_desc(table,"TSELTABL",CHAR,132,select_mode);
      free(selline);
      status = TCTCLO(table->imno);
    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    uttclo(&(table->imno),&status);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *)table->external_info;
    fits_close_file(fptr,&status);
    table->external_info = NULL;
    break;
#endif
  }

  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("Close_table",status);
    return(status);
  } else {
    if (TK && (table->iomode == O_MODE || table->iomode == IO_MODE))
      {		
        printf("@ N {%s}\n",filename);
      }
  }
  if (table->select_flag) {
    if (table->sel_row != NULL) 
      free((char *)table->sel_row);
  }

  return(status);		    
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                    handle_select_flag()
!
!.purp        restricts table read/write access to selected rows
!.desc
! handle_select_flag(table,mode,selection) 
!
! TABLE *table;         table structure
! char  mode;           'W'(arning) or 'Q'(uiet)
! char  *selection;     returns current selection
!                                           as a string (may be NULL)
!.ed
----------------------------------------------------------------------*/

int handle_select_flag(TABLE *table, char mode, char *selection)
{
  int status, select, nselect, i, no_col;
  int *selected;
  char select_mode[133], errtext[132];

  select_mode[0] = 0;

  disable_user_warnings();
  status = RD_desc(table,"TSELTABL",CHAR,132,select_mode);
  restore_user_warnings();

  if ((status > 0) && (select_mode[0] != '-')) {
    first_blk(select_mode);
    if (mode != 'Q') {
      sprintf(errtext,"Selection flag enable %s",
              select_mode);
      print_warning(errtext);
    }
    table->select_flag = 1;  	
    table->sel_row = (int *)calloc(table->row,sizeof(int));
  }
  status = 0;
  /* cross reference for read/write operations */

  if (table->select_flag) {
    switch (table->data_format) {
#ifdef MIDAS
    case MIDAS_FORMAT :
      for (nselect=0,i=0; i< table->row; i++) {
        status = TCSGET(table->imno,i+1,&select);
        if (select == TRUE) {
          table->sel_row[nselect] = i;
          nselect++;
        }
      }
      table->row = nselect;
      break;
#endif
#ifdef IRAF
    case IRAF_FORMAT :
    case STSDAS_FORMAT :
#endif
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :
#endif
#if defined(IRAF) || defined(FITS)
      no_col = get_col_ref(table,"TSELTABL");
      if (no_col < 0) {
        table->select_flag = 0;  	
        free(table->sel_row);
        break;
      }
      selected = (int *)calloc(table->allrow,sizeof(int));
      table->select_flag = 0;  	
      RD_col(table,no_col,selected);
      table->select_flag = 1;  	
      for (nselect=0,i=0; i<table->row; i++) {
        if (selected[i] == TRUE) {
          table->sel_row[nselect] = i;
          nselect++;
        }
      }
      table->row = nselect;
      free((char *)selected);
      break;
#endif
    default :
      return(UNKNOWN);
      break;
    }
  }
  if (selection != NULL)
    strcpy(selection,select_mode);
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                   write_selection()
!
!.purp        Save a selection in the table
!.desc
! write_selection(TABLE *table,int *sel,char *selection)
!
! TABLE *table;         table structure
! int *sel;             selection array
! char  *selection;     new selection
!.ed
----------------------------------------------------------------------*/

int 
write_selection(TABLE *table,int *sel, char *selection)
{
  int status = 0;
  int nselect,no_col,i;

  /* on ecrit le label de la selection */

  /* disable_user_warnings(); */
  status = WR_desc(table,"TSELTABL",CHAR,strlen(selection),selection);
  /* restore_user_warnings(); */

  if (table->sel_row!=NULL)
    free(table->sel_row);

  table->sel_row = (int *)calloc(table->allrow,sizeof(int));

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    for (nselect=0,i=0; i<table->allrow; i++) 
      {
	status = TCSPUT(table->imno,i+1,&(sel[i]));
	if (sel[i] == TRUE) 
	  {
	    table->sel_row[nselect] = i;
	    nselect++;
	  }
      }
    
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
#endif
#if defined(IRAF) || defined(FITS)
    disable_user_warnings();
    no_col = get_col_ref(table,"TSELTABL");
    restore_user_warnings();
    if (no_col < 0) {
      no_col = create_col(table,"TSELTABL",INT,'N',"I1",NULL);
      if (no_col < 0) {
        free(table->sel_row);
        table->sel_row = NULL;
        break;
      }
    }

    for (nselect=0,i=0; i<table->allrow; i++) {
      WR_tbl(table,i,no_col,sel+i);
      if (sel[i] == TRUE) {
        table->sel_row[nselect] = i;
        nselect++;
      }
    }
    break;
#endif
  default :
    return(UNKNOWN);
    break;
  }
  table->row = nselect;
  table->select_flag = 1; 
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                    get_col_ref()
!
!.purp                returns column reference
!.desc
! int col_id = get_col_ref (table,label)			
!
! TABLE *table;         table structure
! char  *label;         column label or #num
!.ed
----------------------------------------------------------------------*/
int 
get_col_ref (TABLE *table, char *label)			
{
  int  status, num, TCLSER();
  char errtext[132], *column_label;

#ifdef IRAF
  int one=1;
#endif

#ifdef FITS
  fitsfile *fptr;
  char key[FLEN_KEYWORD+1];
#endif

  column_label = malloc((strlen(label)+1)*sizeof(char));
  if (column_label == NULL) {
    status = ERR_NOCOL;
    return(status);
  }
  strcpy(column_label,label);

  if (sscanf(column_label,"#%d",&num) == 1) {
    switch (table->data_format) {
#ifdef MIDAS
    case MIDAS_FORMAT :
      break;
#endif
#ifdef IRAF
    case IRAF_FORMAT :
    case STSDAS_FORMAT :
      num = tbcnum(&(table->imno),&num);
      break;
#endif
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :
      status = 0;
      upper_strg(column_label);
      fptr = (fitsfile *)table->external_info;
      fits_get_colnum(fptr,0,column_label,&num,&status);
      if (num > 0) {
        status = get_col_name(table, num, key);
        if (strcmp(column_label,key) != 0) status = -1;
      }
      if (status) {
        num = -1; status = 0;
      };
      break;
#endif
    }
  }
  else {
    switch (table->data_format) {
#ifdef MIDAS
    case MIDAS_FORMAT :
      status = TCCSER(table->imno, label, &num);
      break;
#endif
#ifdef IRAF
    case IRAF_FORMAT :
    case STSDAS_FORMAT :
      utcfnd(&(table->imno),label,&one,&num,&status,strlen(label));
      break;
#endif
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :
      status = 0;
      upper_strg(column_label);
      fptr = (fitsfile *)table->external_info;
      fits_get_colnum(fptr,0,column_label,&num,&status);
      if (num > 0) {
        status = get_col_name(table, num, key);
        if (strcmp(column_label,key) != 0) status = -1;
      }
      /* compatibility with old stuff */
      if (status) {
        if (strcmp(column_label,E3D_COL_ID) == 0) {
          status = 0;
          fits_get_colnum(fptr,0,LAB_COL_NO,&num,&status);
        }
        if (strcmp(column_label,LAB_COL_NO) == 0) {
          status = 0;
          fits_get_colnum(fptr,0,E3D_COL_ID,&num,&status);
        }
        /*
          if (strcmp(column_label,E3D_COL_XPOS) == 0) {
          status = 0;
          fits_get_colnum(fptr,0,"A",&num,&status);
          if (status) {
          status = 0;
          fits_get_colnum(fptr,0,LAB_COL_XLD,&num,&status);
          }
          }
        */
        if (strcmp(column_label,LAB_COL_XLD) == 0) {
          status = 0;
          fits_get_colnum(fptr,0,"A",&num,&status);
        }
        /*
          if (strcmp(column_label,E3D_COL_YPOS) == 0) {
          status = 0;
          fits_get_colnum(fptr,0,"D",&num,&status);
          if (status) {
          status = 0;
          fits_get_colnum(fptr,0,LAB_COL_XLD,&num,&status);
          }
          }
        */
        if (strcmp(column_label,LAB_COL_YLD) == 0) {
          status = 0;
          fits_get_colnum(fptr,0,"D",&num,&status);
        }
      }
      if (status) {
        num = -1; status = 0;
      };
      break;
#endif
    }
  }
  free(column_label);
  if (num < 0) {
    status = ERR_NOCOL;
  }

  if (status) {
    sprintf(errtext,"get_col_ref: table %s column %s", table->name, label);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(errtext,status);
    return(status);
  }
  return(num);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                    get_col_name()
!
!.purp                returns column name
!.desc
! int get_col_name (table,col_id,label)			
!
! TABLE *table;         table structure
! int    col_id;        column id
! char  *label;         column label
!.ed
----------------------------------------------------------------------*/
int get_col_name (TABLE *table, int no_col, char *label)
{
  int  status;
#ifdef IRAF
  int status;
  char unit[lg_unit+1], format[10];
  int nocol;
#endif
#ifdef FITS
  fitsfile *fptr;
  char key[FLEN_KEYWORD+1];
#endif


  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCLGET(table->imno, no_col, label);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    nocol = no_col;
    utcinf(&(table->imno),&nocol,label,unit,format,&dtype,&status,
           132,strlen(unit),5);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *)table->external_info;
    sprintf(key,"TTYPE%d",no_col);
    fits_read_key_str(fptr,key,label,NULL,&status);
    break;
#endif
  }	

  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("get_col_name",status);
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     get_col_info()
!
!.purp        returns informations about a column (type ...)
!.desc
! get_col_info (table, col_id, type, format, unit)
!
! TABLE *table;         table structure
! int  col_id;          column reference
! int  *type;           type (out)
! char *format;         format (out)
! char *unit;           unit (out)
!.ed
----------------------------------------------------------------------*/

int 
get_col_info (TABLE *table, int no_col, int *dtype, char *format, char *unit)
{
  int  status, TCFGET(), len;
  char c;

#ifdef IRAF
  int nocol;
  char colname[132];
#endif

#ifdef FITS
  fitsfile *fptr;
  char key[FLEN_KEYWORD+1];
  int repeat, width;
#endif

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCFGET(table->imno, no_col, format, &len, dtype);
    if (status == 0) 
      status = TCUGET(table->imno, no_col, unit);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    nocol = no_col;
    utcinf(&(table->imno),&nocol,colname,unit,format,dtype,&status,
           132,strlen(unit),5);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *)table->external_info;
    fits_get_coltype(fptr,no_col,dtype, (long *)&repeat, (long *)&width,&status);
    /* variable length column */
    if (*dtype < 0) 
      *dtype *= -1;
    if (format != NULL) {
      sprintf(key,"TFORM%d",no_col);
      fits_read_key_str(fptr,key,format,NULL,&status);
      if ( table->data_format == FITS_B_FORMAT) {
        if (strlen(format) > 1)
          sscanf(format,"%d%c",&width,&c);
        else
          c = format[0];
        switch(c) {
        case 'A' :
          sprintf(format,"A%d",width);
          break;
        case 'I' :
        case 'J' :
          strcpy(format,"I4");
          break;
        case 'E' :
        case 'F' :
          strcpy(format,"F7.4");
          break;
        case 'D' :
          strcpy(format,"F9.6");
          break;
        }
      }
    }
    if (unit != NULL) {
      sprintf(key,"TUNIT%d",no_col);
      fits_read_key_str(fptr,key,unit,NULL,&status);
      if (status) {
        status = 0; strcpy(unit,"none");
      }
    }
    break;
#endif
  }	

  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("get_col_info",status);
  }
  *dtype = decode_datatype(table->data_format,(short)(*dtype));
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     create_col()
!
!.purp        creates a new column according to specifications 
!.desc
! col_id = create_col(table,colname,type,mode,format,unit) 	
!
! TABLE *table;         table structure
! char *colname;        column label 
! short type;           value type in column
! char mode;            mode = N(ew),R(ewrite),O(verwrite)
! char *format;         column format
! char *unit;           units
!.ed
----------------------------------------------------------------------*/

int 
create_col(TABLE *table, char *colname, short type, char mode, char *format, char *user_unit) 	
{
  char *column_label;
  char errtext[132], unit[132], col_format[5];
  int  i, status, len, ttype, num;

#ifdef IRAF
  int one=1;
#endif

#ifdef FITS
  fitsfile *fptr;
  char tform[FLEN_KEYWORD+1];
  char desc[FLEN_KEYWORD+1];
  int width=0;
#endif

  column_label = malloc((strlen(colname)+1)*sizeof(char));
  if (column_label == NULL) {
    status = ERR_NOCOL;
    return(status);
  }
  strcpy(column_label,colname);
  first_blk(column_label);

  if (strlen(column_label) > 16) {
    status = ERR_BAD_COL;
    sprintf(errtext, "create_col: file= %s col=%s", table->name,colname);
    Handle_Error(errtext, status);
    return(status);
  }
  if (user_unit == NULL) 
    strcpy(unit,"None given");
  else
    strcpy(unit,user_unit);

  len = 1;

  switch(type) {

  case CHAR : 
    ttype = get_datatype_code(table->data_format,CHAR);
    strcpy(col_format,format);
    status = sscanf(col_format,"A%d",&len);
    if (status == 1)  {
      status = 0;
      if ((table->data_format == IRAF_FORMAT) 
          || (table->data_format == STSDAS_FORMAT))
        ttype = -len;
    }
    break;
  case SHORT :
  case INT :
  case LONG :
  case FLOAT :
  case DOUBLE :
    status = 0;
    ttype = get_datatype_code(table->data_format,type);
    break;
  default : 
    status = ERR_BAD_TYPE;
    break;
  }
  if (status != 0) {
    sprintf(errtext,"create_col: table %s col %s", table->name, column_label);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(errtext,status);
    return(status);
  } 
  /* does column exist ? */
  disable_user_warnings();
  num = get_col_ref(table,column_label);
  restore_user_warnings();
  if (num > 0) { /* column exists */

    switch(mode) {
      /* Overwrite column */
    case 'O' :
      if (format != NULL) { /* new format */
        switch (table->data_format) {
#ifdef MIDAS
        case MIDAS_FORMAT :	
          status = TCFPUT(table->imno,num,format);
          break;
#endif
#ifdef IRAF
        case IRAF_FORMAT :	
        case STSDAS_FORMAT :	
          utcfmt(&(table->imno),&num,format,&status,strlen(format));
          break;
#endif
#ifdef FITS
        case FITS_A_FORMAT :	
        case FITS_B_FORMAT :
          break;
#endif
        }
        if (status) {	
          sprintf(errtext,"create_col: table %s col %s",
                  table->name, column_label);
          status = get_tiger_errcode(table->data_format,status);
          Handle_Error(errtext,status);
          return(status);
        }
      }			/* new unit */
      if (user_unit != NULL) {
        switch (table->data_format) {
#ifdef MIDAS
        case MIDAS_FORMAT :	
          status = TCUPUT(table->imno,num,unit);
          break;
#endif
#ifdef IRAF
        case IRAF_FORMAT :	
        case STSDAS_FORMAT :	
          utcnit(&(table->imno),&num,unit,&status,strlen(unit));
          break;
#endif
#ifdef FITS
        case FITS_A_FORMAT :	
        case FITS_B_FORMAT :
          break;
#endif
        }
        if (status) {	
          sprintf(errtext,"create_col: table %s col %s",
                  table->name, column_label);
          status = get_tiger_errcode(table->data_format,status);
          Handle_Error(errtext,status);
          return(status);
        }
      }
      free(column_label);
      return(num);
      break;

      /* Rewrite */
    case 'R' :		
      for (i=0; i<table->row; i++)
        WR_null(table,i,num);
      free(column_label);
      return(num);
      break;

      /* Column must be new */
    case 'N' : 
      sprintf(errtext,"create_col: table %s col %s already exists",
              table->name, column_label);
      status = get_tiger_errcode(table->data_format,status);
      Handle_Error(errtext,ERR_REN_TBL);
      return(ERR_REN_TBL);
      break;

      /* Unrecognize option */
    default  : 
      sprintf(errtext,"create_col (table %s col %s): unknown option",
              table->name, column_label);
      Handle_Error(errtext,ERR_BAD_PARAM);
      return(ERR_BAD_PARAM);
      break;		
    }
  } 
  switch (table->data_format) { /* create new column */
#ifdef MIDAS
  case MIDAS_FORMAT :	
    if (type != CHAR) 
      len = 1;
    status = TCCINI(table->imno,ttype,len,format,unit,column_label,&num);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :	
  case STSDAS_FORMAT :	
    utcdef(&(table->imno),column_label,unit,format,&ttype,&one,&num,&status,
           strlen(column_label),strlen(unit),strlen(format));
    break;
#endif
#ifdef FITS

  case FITS_A_FORMAT :	
    status = 0;
    upper_strg(column_label);
    fptr = (fitsfile *)table->external_info;
    num = table->col +1;
    fits_insert_col(fptr,num,column_label,format,&status);
    sprintf(desc,"TUNIT%d",num);
    WR_desc(table,desc,CHAR,lg_unit,unit);
    sprintf(desc,"TDISP%d",num);
    WR_desc(table,desc,CHAR,FLEN_KEYWORD,format);
    break;

  case FITS_B_FORMAT :
    status = 0;
    upper_strg(column_label); 
    fptr = (fitsfile *)table->external_info;
    num = table->col +1;
    switch(type) {
    case CHAR :
	sscanf(format,"A%d",&width);
	sprintf(tform,"%dA%d",width,width);
      break;
    case SHORT :
      strcpy(tform,"I");
      break;
    case LONG :
    case INT :
      strcpy(tform,"J");
      break;
    case FLOAT :
      strcpy(tform,"E");
      break;
    case DOUBLE :
      strcpy(tform,"D");
      break;
    }
    fits_insert_col(fptr,num,column_label,tform,&status);
    if (status == 0) {
    sprintf(desc,"TUNIT%d",num);
    WR_desc(table,desc,CHAR,lg_unit,unit);
    sprintf(desc,"TDISP%d",num);
    WR_desc(table,desc,CHAR,FLEN_KEYWORD,format);
    }
    break;
#endif
  }
  if (num < 0) status = -1;

  free(column_label);
  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("create_col",status);
    return(status);	
  }
  else
    table->col++;
  return(num);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     delete_col()
!
!.purp            deletes the specified column
!.desc
! delete_col (table,col_id)			
!
! TABLE *table;         table structure
! int col_id;           column reference
!.ed
----------------------------------------------------------------------*/

int 
delete_col (TABLE *table,int nocol)			
{
  int  status=0, ncol;
#ifdef IRAF
  int i;
#endif
#ifdef FITS
  fitsfile *fptr;
#endif


  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCCDEL(table->imno, nocol, &ncol);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    for (i=0; i<table->allrow && (!status); i++)
      status = WR_null(table,i,nocol);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *)table->external_info;
    fits_delete_col(fptr,nocol,&status);
    break;
#endif
  }
  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("Delete_col",status);
  }
  else
    table->col--;
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     delete_row()
!
!.purp             deletes the specified row
!.desc
! delete_row (table,norow)			
!
! TABLE *table;         table structure
! int norow;            row number
!.ed
----------------------------------------------------------------------*/

int 
delete_row (TABLE *table,int nrow)			
{
  int  status, true_row=nrow+1;
#ifdef FITS
  fitsfile *fptr;
#endif


  if (table->select_flag) 
    true_row = table->sel_row[nrow]+1;

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCRDEL(table->imno, true_row);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    status = ERR_NOIMPL;
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *)table->external_info;
    fits_delete_rows(fptr,true_row,1L,&status);
    break;
#endif
  }
  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("Delete_row",status);
  }
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     RD_tbl()
!
!.purp        reads a value in table(line,column)
!.desc
! RD_tbl(table,row,col_id,value)
!
! TABLE *table;         table structure
! int row;              row number
! int col_id;           column reference
! (type) *value;        value (out)
!.ed
----------------------------------------------------------------------*/

int 
RD_tbl(TABLE *table, int row, int col, void *value)
{
  int  null=0, status, true_row=row+1, len, dtype, lval;
  char format[20], unit[lg_unit+1], err_text[132];
  char *pt_value = (char *)value;
  int type;

#ifdef IRAF
  int one=1;
#endif
#ifdef FITS
  fitsfile *fptr;
  int int_datatype;
  char **str_val;
#endif

  if (table->select_flag) 
    true_row = table->sel_row[row]+1;

  status = get_col_info(table, col, &dtype, format, unit);	
  if (status) {
    sprintf(err_text,"RD_tbl table %s col #%d row #%d",
            table->name, col,true_row);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(err_text,status);
    return(status);
  }

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :

    switch (dtype) {
	
    case CHAR :
      status = TCFGET(table->imno,col,format,&len,&type);
      if (! status) {
        status = TCERDC(table->imno,true_row,col,pt_value,&null);
        pt_value[len] = '\0';
      }
      break;
    case SHORT :	
      status = TCERDI(table->imno,true_row,col,&lval,&null);
      *((short *)pt_value) = (short)lval;
      break;
    case INT :	
    case LONG :	
      status = TCERDI(table->imno,true_row,col,(int *)pt_value,&null);
      break;
    case FLOAT :
      status = TCERDR(table->imno,true_row,col,(float *)pt_value,&null);
      break;
    case DOUBLE :
      status = TCERDD(table->imno,true_row,col,(double *)pt_value,&null);
      break;
    }
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :

    switch (dtype) {
	
    case CHAR :
      sscanf(format,"A%d",&len);
      utrgtt(&(table->imno),&col,&one,&true_row,pt_value,&null,
             &status,len);
      pt_value[len] = '\0';
      pt_value[last_char(pt_value)] = '\0';
      break;
    case SHORT :
      utrgti(&(table->imno),&col,&one,&true_row,&lval,&null,&status);
      *(short *)pt_value = (short)lval;
      break;
    case INT :	
    case LONG :
      utrgti(&(table->imno),&col,&one,&true_row,&lval,&null,&status);
      *(long *)pt_value = lval;
      break;
    case FLOAT :
      utrgtr(&(table->imno),&col,&one,&true_row,pt_value,&null,&status);
      break;
    case DOUBLE :
      utrgtd(&(table->imno),&col,&one,&true_row,pt_value,&null,&status);
      break;
    }
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *)table->external_info;
    int_datatype = get_datatype_code(FITS_A_FORMAT,dtype);
    if (dtype == CHAR) {
      sscanf(format,"A%d",&len);
      str_val = (char **)malloc(sizeof(char *));
      str_val[0] = pt_value;
      fits_read_col(fptr,int_datatype,col,true_row,1L,1L,	
                    NULL, str_val, &null, &status);
      free((char *)str_val);
      pt_value[len] = '\0';
    }
    else {
      fits_read_col(fptr,int_datatype,col,true_row,1L,1L,	
                    NULL, pt_value, &null, &status);
    }
    break;
#endif
  }
  if (status) {
    sprintf(err_text,"RD_tbl table %s col #%d row #%d", 
            table->name, col,true_row);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(err_text,status);
  }
  if (null != 0) 
    status = ERR_NODATA;
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     RD_col()
!
!.purp        reads all a column values in table
!.desc
! RD_col(table,col_id,value)
!
! TABLE *table;         table structure
! int col_id;           column reference
! (type) *value;        array of values (out)
!.ed
----------------------------------------------------------------------*/

int 
RD_col(TABLE *table, int col, void *value)
{
  int  i, null, status, true_row, first_row, last_row, dtype, nocol=col,len,
    nbchars;
  char format[20], unit[lg_unit+1], err_text[132];
  char *pt_value, *pt_tmp;

#ifdef IRAF
  int nbelt;
  unsigned char *nullflag;
#endif
#ifdef FITS
  fitsfile *fptr;
  int int_datatype;
  char **str_val;
#endif

  status = get_col_info(table, col, &dtype, format, unit);	
  if (dtype == CHAR) {
    if (('0' <= *format) && (*format <= '9'))
      sscanf(format,"%dA1",&len);
    else
      sscanf(format,"A%d",&len);
  }
  else
    len= 1;
  if (status) {
    sprintf(err_text,"RD_col table %s col #%d", table->name, col);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(err_text,status);
    return(status);
  }

  if (table->select_flag) {
    first_row = table->sel_row[0]+1;
    last_row = table->sel_row[table->row-1]+1;
  }
  else {
    first_row = 1;
    last_row = table->allrow;
  }
  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCCMAP(table->imno,nocol,&pt_value);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    nullflag = (unsigned char *)malloc(table->allrow*sizeof(unsigned char));
    switch (dtype) {
    case CHAR :
      nbelt = (last_row-first_row+1);
      pt_value = (char *)malloc(nbelt*len*sizeof(char));
      utcgtt(&(table->imno),&nocol,&first_row,&last_row,pt_value,nullflag,
             &status,len);
      if (status != 0) break;
      for (pt_tmp=pt_value, i=0; i<nbelt; i++,pt_tmp+=len) {
        pt_tmp[len-1] = '\0';
        pt_tmp[last_char(pt_tmp)] = '\0';
      }
      break;
    case SHORT :
      pt_value = (char *)malloc((last_row-first_row+1)*sizeof(short));
      utcgts(&(table->imno),&nocol,&first_row,&last_row,pt_value,nullflag,
             &status);
      break;
    case INT :
    case LONG :
      pt_value = (char *)malloc((last_row-first_row+1)*sizeof(int));
      utcgti(&(table->imno),&nocol,&first_row,&last_row,pt_value,nullflag,
             &status);
      break;
    case FLOAT :
      pt_value = (char *)malloc((last_row-first_row+1)*sizeof(float));
      utcgtr(&(table->imno),&nocol,&first_row,&last_row,pt_value,nullflag,
             &status);
      break;
    case DOUBLE :
      pt_value = (char *)malloc((last_row-first_row+1)*sizeof(double));
      utcgtd(&(table->imno),&nocol,&first_row,&last_row,pt_value,nullflag,
             &status);
      break;
    }
    free((char *)nullflag);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    first_row = 1;
    last_row = table->allrow;
    fptr = (fitsfile *)table->external_info;
    int_datatype =
      get_datatype_code(table->data_format,(short)dtype);
    if (dtype == CHAR) {
      pt_value = (char *)malloc(table->allrow*len);
      str_val = (char **)malloc(table->allrow*sizeof(char *));
      for (i=0; i<table->allrow;i++)
        str_val[i] = pt_value + i*len;
      fits_read_col(fptr,int_datatype,col,1L,1L,table->row,	
                    NULL, str_val, &null, &status);
      free((char *)str_val);
    }
    else {
      pt_value = (char *)malloc(table->allrow*sizeof_item((short)dtype));
      fits_read_col(fptr,int_datatype,col,1L,1L,table->allrow,	
                    NULL, (void *)pt_value, &null, &status);
    }
    break;
#endif
  }
  if (status != 0) {
    sprintf(err_text,"RD_col table %s col #%d", table->name, col);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(err_text,status);
    return (status);
  }

  if (!table->select_flag) {
    nbchars = (last_row - first_row +1)*len;
    nbchars *= sizeof_item((short)dtype);
    memcpy((char *)(value),pt_value,nbchars);
  }
  else {
	
    switch (dtype) {	/* handle table selection if any */
    case CHAR :
      for (pt_tmp = (char *)value, i=0; i<table->row; i++) {
        true_row = table->sel_row[i];
        memcpy(pt_tmp,pt_value+(true_row)*len,len);
        pt_tmp += len;
      }
      break;
    case SHORT :
      for (i=0; i<table->row; i++) {
        true_row = table->sel_row[i];
        *((short *)(value)+i) = *((short *)(pt_value)+true_row);
      }
      break;
    case INT :
    case LONG :
      for (i=0; i<table->row; i++) {
        true_row = table->sel_row[i];
        *((int *)(value)+i) = *((int *)(pt_value)+true_row);
      }
      break;
    case FLOAT :
      for (i=0; i<table->row; i++) {
        true_row = table->sel_row[i];
        *((float *)(value)+i) = *((float *)(pt_value)+true_row);
      }
      break;
    case DOUBLE :
      for (i=0; i<table->row; i++) {
        true_row = table->sel_row[i];
        *((double *)(value)+i) = *((double *)(pt_value)+true_row);
      }
      break;
    }
  }
  if (table->data_format != MIDAS_FORMAT)
    free((char *)pt_value);
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     WR_tbl()
!
!.purp            writes a value into table
!.desc
! WR_tbl(table,row,col_id,value)
!
! TABLE *table;         table structure
! int row;              row number 
! int col_id;           column reference
! (type) *value;        value to write
!.ed
----------------------------------------------------------------------*/

int 
WR_tbl(TABLE *table, int row, int col, void *value)
{
  int status, true_row=row+1, dtype, lval;
  char format[20], unit[lg_unit+1], err_text[132], *pt_value;
	
#ifdef IRAF
  int one=1;
#endif
#ifdef FITS
  fitsfile *fptr;
  int int_datatype;
  char **str_val;
#endif

  if (table->iomode == (int)I_MODE) {
    status = ERR_ACCESS;
    sprintf(err_text, "WR_tbl: file %s is opened Read-Only", table->name);
    Handle_Error(err_text, status);
    return(status);
  }
  if (table->select_flag) 
    true_row = table->sel_row[row]+1;

  status = get_col_info(table,col,&dtype,format,unit);
  if (status) {
    sprintf(err_text,"WR_tbl table %s col #%d row #%d", 
            table->name, col,true_row);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(err_text,status);
    return(status);
  }

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :

    switch(dtype) {

    case CHAR :
      pt_value = (char *)value;
      status = TCEWRC(table->imno,true_row,col,pt_value);
      break;
    case SHORT :
      lval = *(short *)value;
      status = TCEWRI(table->imno,true_row,col,&lval);
      break;
    case INT :
    case LONG :
      lval = *(long *)value;
      status = TCEWRI(table->imno,true_row,col,&lval);
      break;
    case FLOAT :
      status = TCEWRR(table->imno,true_row,col,value);
      break;
    case DOUBLE :
      status = TCEWRD(table->imno,true_row,col,value);
      break;
    }
    break;
#endif

#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :

    if (dtype < 0) dtype = CHAR;

    switch(dtype) {

    case CHAR :
      utrptt(&(table->imno),&col,&one,&true_row,value,&status,
             strlen(value));
      break;
    case SHORT :
      lval = *(short *)value;
      utrpti(&(table->imno),&col,&one,&true_row,&lval,&status);
      break;
    case INT :
    case LONG :
      lval = *(long *)value;
      utrpti(&(table->imno),&col,&one,&true_row,&lval,&status);
      break;
    case FLOAT :
      utrptr(&(table->imno),&col,&one,&true_row,value,&status);
      break;
    case DOUBLE :
      utrptd(&(table->imno),&col,&one,&true_row,value,&status);
      break;
    }
    break;
#endif

#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    fptr = (fitsfile *) table->external_info;
    int_datatype = get_datatype_code(table->data_format,dtype);
    if (true_row > table->allrow) {/*need to insert a new line*/
      fits_insert_rows(fptr,true_row-1,1,&status);	
    }
    switch(dtype) {
    case CHAR :
      sscanf(format,"A%d",&lval);
      str_val = (char **)malloc(sizeof(char *));
      if (lval == 1) {
        pt_value = (char *)malloc(2*sizeof(char));
        pt_value[0] = *(char *)value;
        pt_value[1] = '\0';
      }
      else {
        pt_value = (char *)value;
      }
      str_val[0] = pt_value;
      fits_write_col(fptr,int_datatype,col,true_row,1L,1L,str_val,&status);
      free((char *)str_val);
      if (lval == 1)
        free(pt_value);
      break;
    default :
      fits_write_col(fptr,int_datatype,col,true_row,1L,1L,value,&status);
    }
    break;
#endif
  }
  if (status) {
    sprintf(err_text,"WR_tbl table %s col #%d row #%d", 
            table->name, col,true_row);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(err_text,status);
  }
  if (table->allrow < true_row) {
    table->allrow++;
    table->row++;
  }
  return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!                                                                    
!.func                     WR_null()
!
!.purp        write a NULL value into table(line,column)
!.desc
! WR_null(table,row,col_id)
!
! TABLE *table;         table structure
! int row;              row number
! int col_id;           column reference
!.ed
----------------------------------------------------------------------*/

#define NUL_IVAL  2147483647            /* undefined integer */
#define NUL_RVAL  1.701411e38           /* undefined real */
#define NUL_DVAL  1.70141183460469229e38    /* undefined double */
#define NUL_CVAL  '\0'              /* undefined char  */

int 
WR_null(TABLE *table,int row,int col)
{
  int status, true_row=row+1;

#ifdef IRAF
  int one=1;
  int no_col=col;
#endif
	
#ifdef FITS
  char format[20], unit[lg_unit+1];
  int dtype;
  char c_null = NUL_CVAL;
  int i_null = NUL_IVAL;
  float f_null = NUL_RVAL;
  double d_null = NUL_DVAL;
#endif

  if (table->select_flag) 
    true_row = table->sel_row[row-1]+1;

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = TCEDEL(table->imno,true_row,col);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    utrudf(&(table->imno),&no_col,&one,&true_row,&status);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = get_col_info(table,col,&dtype,format,unit);
    switch(dtype) {
    case CHAR :
      status = WR_tbl(table, row, col, &c_null);
      break;
    case SHORT :
    case INT :
    case LONG :
      status = WR_tbl(table, row, col, &i_null);
      break;
    case FLOAT :
      status = WR_tbl(table, row, col, &f_null);
      break;
    case DOUBLE :
      status = WR_tbl(table, row, col, &d_null);
      break;
    }
    break;
#endif
  }
  if (status) {
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error("WR_null",status);
  }
  return(status);		    
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_table()
!
!.purp           delete a currently active table
!.desc
! delete_table(table)	
!
! TABLE *table;         table structure
!.ed
-------------------------------------------------------------------- */
int 
delete_table(TABLE *table)			/* delete active spectrum */
{
  int status;
  char errtext[132], filename[lg_name+1];
	
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int iomode;
  char *extname;
  char file[lg_name+1];
  int hdutype, hdu_num;
#endif

  strcpy(filename,table->name);

  switch (table->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFDEL(filename);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uttdel(filename,&status,len);
    break;
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname == NULL) {
      if (table->external_info != NULL) {
        fptr = (fitsfile *)table->external_info;
        fits_delete_file(fptr,&status);
      }
      else
        status = unlink(filename);
    } else {
      extname++;
      extname[strlen(extname)-1] = '\0';
      hdutype = ANY_HDU;
      if (table->external_info == NULL) {
        iomode = get_iomode_code(InputIO.basic_io,IO_MODE);
        fits_open_file(&fptr,filename,iomode,&status);
      }
      else
        fptr = (fitsfile *)table->external_info;
      if (!is_numeric(extname)) {
        fits_movabs_hdu(fptr, 1, NULL, &status);  /* reach beginning of file */
        fits_movnam_hdu(fptr,hdutype,extname,0,&status); /* search for extname */
      }
      else {
        sscanf(extname,"%d",&hdu_num);
        fits_movabs_hdu(fptr, hdu_num+1, NULL, &status); 
      }
      fits_delete_hdu(fptr,NULL,&status);
      if (table->external_info == NULL) {
        fits_close_file(fptr,&status);
      }
    }
    break;
#endif
  }
		
  if (status)	{
    sprintf(errtext,"delete_table: table %s",filename);
    status = get_tiger_errcode(table->data_format,status);
    Handle_Error(errtext,status);
  }
  if (TK)
    {		
      printf("@ D {%s}\n",filename);
    }
  return(status);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk                 Routines for tridimensional IMAGES i/o
!
!.func                        create_cube()
!
!.purp           creates a new 3D frame according to specifications
!.desc
! create_cube(cube,name,npix,start,step,datatype,ident,unit);
!
! IMAGE3D *cube;        cube structure
! char  *name;          cube file name
! int *npix;            number of pixels in each dim.
! double *start;        start coordinates in x, y and z
! double *step;         step values in x, y  and z dim.
! short datatype;       type of data storage
! char *ident;          identifier
! char *unit;           units
!.ed
-------------------------------------------------------------------- */

int
create_cube(IMAGE3D *cube, char *name, int *npix, double *start, double *step, 
            short datatype, char *ident, char *unit)
{
  char errtext[132], filename[lg_name+1];
  int three_dim=3, iomode, int_datatype;
  int status, SCIPUT();
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int bitpix, hdupos;
  int one = 1;
  char *extname;
  char file[lg_name+1];
#endif

  strcpy(filename,name);
  first_blk(filename);
  strcpy(cube->name,filename);
  append_ima_extension(cube->name,OutputIO.basic_io);
	
  strcpy(filename,cube->name);

  if (ASK)
    confirme_erase(filename);

  cube->ident[0] = '\0';
  cube->history[0] = '\0';
  strcpy(cube->cunit,"None given");
  cube->startx = start[0]; cube->starty = start[1]; cube->startz = start[2];	
  cube->stepx = step[0]; cube->stepy = step[1]; cube->stepz = step[2];
  cube->endx = start[0] + (npix[0]-1)*step[0];
  cube->endy = start[1] + (npix[1]-1)*step[1];
  cube->endz = start[2] + (npix[2]-1)*step[2];
  cube->nx = npix[0];	cube->ny = npix[1];	cube->nz = npix[2];
  cube->iomode = (int)O_MODE;
  cube->data_type = datatype;
  cube->file_type = T_IMA3D;
  cube->data_format = OutputIO.basic_io;
  cube->external_info = NULL;
  cube->nwcs = 0;
  cube->wcs = NULL;
  	
  if (ident != NULL) strcpy(cube->ident,ident);
  if (unit != NULL) {
    memcpy(cube->cunit,unit,lg_unit);
    cube->cunit[lg_unit] = '\0';
  }
  	
  iomode = get_iomode_code(OutputIO.basic_io,cube->iomode);
  int_datatype = get_datatype_code(OutputIO.basic_io,datatype);

  switch(OutputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCIPUT(filename, int_datatype, iomode,
                    F_IMA_TYPE, three_dim, &(cube->nx), &(cube->startx),
                    &(cube->stepx),cube->ident, cube->cunit, &(cube->data),
                    &(cube->imno));
    break;
#endif
#ifdef IRAF
    /*    case IRAF_FORMAT :
          case STSDAS_FORMAT :
          len = strlen(filename);
          uimdel(filename,&status,len);
          uimcre(filename,&int_datatype,&two_dim,&(frame->nx),
          &(frame->imno),&status,len);
          if (status) break;
          status = WR_desc(frame,"IDENT",CHAR,strlen(frame->ident),
          frame->ident);
          if (status) break;
          status = alloc_frame_mem(frame, datatype);
          break; */
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    bitpix = fits_bitpix(datatype);
    if ((hdupos = exist(filename)) > 0)
      delete_cube(cube);
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname != NULL) {
      /* extension to a file; first check if the file exists, if not creates it */
      extname[0] = '\0';
      extname++;
      extname[strlen(extname)-1] = '\0';
      if (!exist(file)) {
        fits_create_file(&fptr,file,&status);
        fits_insert_img(fptr,bitpix,three_dim,(long *)&(cube->nx),&status);
      }
      else {
        fits_open_file(&fptr,file,iomode,&status);
        if (hdupos > 1) 
          fits_movabs_hdu(fptr,hdupos-1,NULL,&status);
        else
          fits_movabs_hdu(fptr,1,NULL,&status);
        fits_insert_img(fptr,bitpix,three_dim,(long *)&(cube->nx),&status);
      }
    }
    else {
      fits_create_file(&fptr,filename,&status);
      fits_create_img(fptr,bitpix,three_dim,(long *)&(cube->nx),&status);
    }
    cube->external_info = (void *)fptr;
    fits_write_key(fptr, TINT, "CRPIX1", &one, "reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CRVAL1", &(cube->startx), 
                   "coordinate at reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CDELT1", &(cube->stepx), 
                   "coordinate increment per pixel", &status);
    fits_write_key(fptr, TINT, "CRPIX2", &one, "reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CRVAL2", &(cube->starty), 
                   "coordinate at reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CDELT2", &(cube->stepy), 
                   "coordinate increment per pixel", &status);
    fits_write_key(fptr, TINT, "CRPIX3", &one, "reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CRVAL3", &(cube->startz), 
                   "coordinate at reference pixel", &status);
    fits_write_key(fptr, TDOUBLE, "CDELT3", &(cube->stepz), 
                   "coordinate increment per pixel", &status);

    if ((extname != NULL) && (!is_numeric(extname))) {
      fits_write_key(fptr, TSTRING, "EXTNAME", extname, "Extension name", &status);
    }
    if (!status) {
      status = alloc_cube_mem(cube, datatype);
      WR_desc(cube,"IDENT",CHAR,lg_ident,cube->ident);
    }
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"create_cube: cube %s",filename);
    status = get_tiger_errcode(cube->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    cube->min = 0;
    cube->max = 0;
    WR_desc(cube,"COMMENT",CHAR,8,"       ");
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        header_cube()
!
!.purp   	updates the cube structure items (does not map data !)
!.desc
! header_cube(cube,name)	
!
! IMAGE3D *cube;        cube structure
! char *name;           cube name
!.ed
-------------------------------------------------------------------- */
int
header_cube(IMAGE3D *cube, char *name, char *mode)		
{
  char errtext[132], filename[lg_name+1];
  int status, nbaxes, iomode, int_datatype;
  float cuts[4];
  int info[5];
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int bitpix;
  double pixref;
#endif

  memset(cube->ident,' ',lg_ident);
  cube->ident[lg_ident] = '\0';
  memset(cube->cunit,' ',lg_unit);
  cube->cunit[lg_unit] = '\0';
  memset(cube->history,' ',lg_hist);
  cube->history[lg_hist] = '\0';
  cube->external_info = NULL;
  cube->file_type = T_IMA3D;
  cube->data_format = InputIO.basic_io;
  cube->data.d_data = NULL;
  cube->min = 0;
  cube->max = 0;
  cube->nwcs = 0;
  cube->wcs = NULL;

  strcpy(filename,name);
  first_blk(filename);
  strcpy(cube->name,filename);
  append_ima_extension(cube->name,InputIO.basic_io);

  strcpy(filename,cube->name);

  if (!exist(filename)) {
    status = ERR_OPEN;
    sprintf(errtext,"header_cube: cube %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' :
    if (mode[1] == 'O')
      cube->iomode = (int)IO_MODE;
    else
      cube->iomode = (int)I_MODE;
    break;
  case 'O' : cube->iomode = (int)O_MODE;
    break;
  default  : cube->iomode = (int)I_MODE;
    break;
  }
	
  iomode = get_iomode_code(InputIO.basic_io,cube->iomode);

  switch (InputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFINF(filename,2,info);
    if (status == 0) {
      status = SCFOPN(filename, info[1], 0, F_IMA_TYPE,
                      &(cube->imno));
      nbaxes = RD_desc(cube,"NPIX",INT,2,&(cube->nx));
      if (nbaxes != 3) {
        /*		 We opened a spectrum or image as a cube, bad work */
        status = ERR_OPEN;
        break;
      }

      RD_desc(cube,"START",DOUBLE,2,&(cube->startx));
      RD_desc(cube,"STEP",DOUBLE,2,&(cube->stepx));
      RD_desc(cube,"IDENT",CHAR,lg_ident,cube->ident);
      RD_desc(cube,"CUNIT",CHAR,lg_unit,cube->cunit);
      cube->data_type = info[1];
      cube->data_type = decode_datatype(InputIO.basic_io,cube->data_type);

    }
    break;
#endif
#ifdef IRAF
    /*    case IRAF_FORMAT :
          case STSDAS_FORMAT :
          len = strlen(filename);
          uimopn(filename,&iomode,&(cube->imno),&status,len);
          if (status != 0)
          break;
          uimgid(&(cube->imno),&int_datatype,&three_dim,&(cube->nx),&status);
          cube->data_type = decode_datatype(InputIO.basic_io,(short)(int_datatype));
          disable_user_warnings();
          RD_desc(cube,"IDENT",CHAR,lg_ident,cube->ident);
          restore_user_warnings();
          break;*/
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    if (fits_open_file(&fptr,filename,iomode,&status)) {
      status = ERR_ACCESS; break;
    }
    cube->external_info = (void *)fptr;
    if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) {
      status = ERR_READ; break;
    }
    if (nbaxes != 3) {
      status = ERR_IMA_HEAD; break;
    }
    if (fits_read_key(fptr, TINT,"BITPIX", &bitpix,NULL, &status)) {
      status = ERR_READ; break;
    }
    int_datatype = (fptr->Fptr->tableptr)->tdatatype;
    cube->data_type = decode_datatype(InputIO.basic_io,(short)int_datatype);
    if (cube->data_type == SHORT) {
      if (fptr->Fptr->tableptr[1].tscale == 1 && fptr->Fptr->tableptr[1].tzero == 32768)
        /* unsigned short !!! */
        cube->data_type = LONG;
    }
    if (fits_read_key(fptr, TINT, "NAXIS1",
                      &(cube->nx), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS2",
                      &(cube->ny), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS3",
                      &(cube->nz), NULL, &status)) {
      status = ERR_READ; break;
    }
    pixref = 1.0;
    fits_read_key(fptr, TDOUBLE, "CRPIX1", &pixref, NULL, &status);
    if (status) { status = 0; pixref = 1; }
    fits_read_key(fptr, TDOUBLE, "CRVAL1", &(cube->startx), NULL, &status);
    if (status) { status = 0; cube->startx = (double)1; }
    fits_read_key(fptr, TDOUBLE, "CDELT1", &(cube->stepx), NULL, &status);
    if (status) { status = 0; cube->stepx = (double)1; }
    cube->startx -= (pixref-1)*cube->stepx;
    pixref = 1.0;
    fits_read_key(fptr, TDOUBLE, "CRPIX2", &pixref, NULL, &status);
    if (status) { status = 0; pixref = 1; }
    fits_read_key(fptr, TDOUBLE, "CRVAL2", &(cube->starty), NULL, &status);
    if (status) { status = 0; cube->starty = (double)1; }
    fits_read_key(fptr, TDOUBLE, "CDELT2", &(cube->stepy), NULL, &status);
    if (status) { status = 0; cube->stepy = (double)1; }
    cube->starty -= (pixref-1)*cube->stepy;
    pixref = 1.0;
    fits_read_key(fptr, TDOUBLE, "CRPIX3", &pixref, NULL, &status);
    if (status) { status = 0; pixref = 1; }
    fits_read_key(fptr, TDOUBLE, "CRVAL3", &(cube->startz), NULL, &status);
    if (status) { status = 0; cube->startz = (double)1; }
    fits_read_key(fptr, TDOUBLE, "CDELT3", &(cube->stepz), NULL, &status);
    if (status) { status = 0; cube->stepz = (double)1; }
    cube->startz -= (pixref-1)*cube->stepz;
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"header_cube: cube %s",filename);
    status = get_tiger_errcode(cube->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    disable_user_warnings();
    status = RD_desc(cube,"LHCUTS",FLOAT,4,cuts);
    RD_desc(cube,"HISTORY",CHAR,lg_hist,cube->history);
    restore_user_warnings();

    cube->endx = cube->startx + (cube->nx -1)*cube->stepx;
    cube->endy = cube->starty + (cube->ny -1)*cube->stepy;
    cube->endz = cube->startz + (cube->nz -1)*cube->stepz;
    if (status > 0) {
      cube->min = cuts[2];
      cube->max = cuts[3];
    }
    status = 0;
    /* parse wcs if contained in file */
    status = parse_wcs(cube);
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        open_cube()
!
!.purp          opens a 3D frame and updates the cube structure items
!.desc
! open_cube(cube,name,mode)	
!
! IMAGE3D *cube;        cube structure
! char *name;           cube name
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */
int
open_cube(IMAGE3D *cube, char *name, char *mode)		
{
  char errtext[132], filename[lg_name+1];
  int status, nbaxes, iomode, int_datatype;
  float cuts[4];
  int info[5];
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  int nbread;
  int npix;
  int group = 0;
  double pixref;
#endif

  memset(cube->ident,' ',lg_ident);
  cube->ident[lg_ident] = '\0';
  memset(cube->cunit,' ',lg_unit);
  cube->cunit[lg_unit] = '\0';
  memset(cube->history,' ',lg_hist);
  cube->history[lg_hist] = '\0';
  cube->external_info = NULL;
  cube->file_type = T_IMA3D;
  cube->data_format = InputIO.basic_io;

  strcpy(filename,name);
  first_blk(filename);
  strcpy(cube->name,filename);
  append_ima_extension(cube->name,InputIO.basic_io);

  strcpy(filename,cube->name);

  if (!exist(filename)) {
    status = ERR_OPEN;
    sprintf(errtext,"open_cube: cube %s",filename);
    Handle_Error(errtext,status);
    return(status);
  }

  switch(mode[0]) {
  case 'I' :
    if (mode[1] == 'O')
      cube->iomode = (int)IO_MODE;
    else
      cube->iomode = (int)I_MODE;
    break;
  case 'O' : cube->iomode = (int)O_MODE;
    break;
  default  : cube->iomode = (int)I_MODE;
    break;
  }
	
  iomode = get_iomode_code(InputIO.basic_io,cube->iomode);

  switch (InputIO.basic_io) {

#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFINF(filename,2,info);
    if (status == 0) {
      status = SCIGET(filename, info[1], iomode, F_IMA_TYPE, 3,
                      &nbaxes, &(cube->nx), &(cube->startx), &(cube->stepx),
                      cube->ident, cube->cunit, (char **)(&(cube->data)),
                      &(cube->imno));
      cube->data_type = info[1];
      cube->data_type = decode_datatype(InputIO.basic_io,cube->data_type);

      if (nbaxes!=3) /* We open a spectrum or an image like a cube, and that's not good */
        status = ERR_OPEN;
    }
    break;
#endif
#ifdef IRAF
    /*    case IRAF_FORMAT :
          case STSDAS_FORMAT :
          len = strlen(filename);
          uimopn(filename,&iomode,&(frame->imno),&status,len);
          if (status != 0)
          break;
          uimgid(&(frame->imno),&int_datatype,&two_dim,&(frame->nx),&status);
          frame->data_type = decode_datatype(InputIO.basic_io,(short)(int_datatype));
          if (status != 0)
          break;
          alloc_frame_mem(frame, datatype);
          switch(frame->data_type) {
          case SHORT :
          uigs2s(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
          frame->data.s_data,&status);
          break;
          case INT :
          case LONG :
          uigs2l(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
          frame->data.l_data,&status);
          break;
          case FLOAT :
          uigs2r(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
          frame->data.f_data,&status);
          break;
          case DOUBLE :
          uigs2d(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
          frame->data.d_data,&status);
          break;
          }
          disable_user_warnings();
          RD_desc(frame,"IDENT",CHAR,lg_ident,frame->ident);
          restore_user_warnings();
          break; */
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status =0;
    if (fits_open_file(&fptr,filename,iomode,&status)) {
      status = ERR_ACCESS; break;
    }
    cube->external_info = (void *)fptr;
    if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) {
      status = ERR_READ; break;
    }
    if (nbaxes != 3) {
      status = ERR_IMA_HEAD; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS1",
                      &(cube->nx), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS2",
                      &(cube->ny), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (fits_read_key(fptr, TINT, "NAXIS3",
                      &(cube->nz), NULL, &status)) {
      status = ERR_READ; break;
    }
    if (status == 0) {
      pixref = 1.0;
      fits_read_key(fptr, TDOUBLE, "CRPIX1", &pixref, NULL, &status);
      if (status) { status = 0; pixref = 1; }
      fits_read_key(fptr, TDOUBLE, "CRVAL1", &(cube->startx), NULL, &status);
      if (status) { status = 0; cube->startx = (double)1; }
      fits_read_key(fptr, TDOUBLE, "CDELT1", &(cube->stepx), NULL, &status);
      if (status) { status = 0; cube->stepx = (double)1; }
      cube->startx -= (pixref-1)*cube->stepx;
      pixref = 1.0;
      fits_read_key(fptr, TDOUBLE, "CRPIX2", &pixref, NULL, &status);
      if (status) { status = 0; pixref = 1; }
      fits_read_key(fptr, TDOUBLE, "CRVAL2", &(cube->starty), NULL, &status);
      if (status) { status = 0; cube->starty = (double)1; }
      fits_read_key(fptr, TDOUBLE, "CDELT2", &(cube->stepy), NULL, &status);
      if (status) { status = 0; cube->stepy = (double)1; }
      cube->starty -= (pixref-1)*cube->stepy;
      pixref = 1.0;
      fits_read_key(fptr, TDOUBLE, "CRPIX3", &pixref, NULL, &status);
      if (status) { status = 0; pixref = 1; }
      fits_read_key(fptr, TDOUBLE, "CRVAL3", &(cube->startz), NULL, &status);
      if (status) { status = 0; cube->startz = (double)1; }
      fits_read_key(fptr, TDOUBLE, "CDELT3", &(cube->stepz), NULL, &status);
      if (status) { status = 0; cube->stepz = (double)1; }
      cube->startz -= (pixref-1)*cube->stepz;
    }
    else
      break;

    int_datatype = (fptr->Fptr->tableptr)->tdatatype;
    cube->data_type = decode_datatype(InputIO.basic_io,(short)int_datatype);
    if (cube->data_type == SHORT) {
      if (fptr->Fptr->tableptr[1].tscale == 1 && fptr->Fptr->tableptr[1].tzero == 32768)
        /* unsigned short !!! */
        cube->data_type = LONG;
    }

    if (alloc_cube_mem(cube, cube->data_type) < 0) {
      fits_close_file(fptr,&status);
      status = ERR_ALLOC;
      break;
    }

    npix = cube->nx*cube->ny*cube->nz;
    switch (cube->data_type) {
    case SHORT :
      if (fits_read_img_sht(fptr,group,1L,npix,(short)0,
                            cube->data.s_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case LONG :
    case INT :
      if (fits_read_img_lng(fptr,group,1L,npix,(int)0,
                            cube->data.l_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case FLOAT :
      if (fits_read_img_flt(fptr,group,1L,npix,(float)0,
                            cube->data.f_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    case DOUBLE :
      if (fits_read_img_dbl(fptr,group,1L,npix,(double)0,
                            cube->data.d_data,&nbread,&status)) {
        status = ERR_READ; break;
      }
      break;
    }
    break;
#endif
  }

  if (status) {
    sprintf(errtext,"open_cube: cube %s",filename);
    status = get_tiger_errcode(cube->data_format,status);
    Handle_Error(errtext,status);
  }
  else {
    disable_user_warnings();
    status = RD_desc(cube,"LHCUTS",FLOAT,4,cuts);
    RD_desc(cube,"HISTORY",CHAR,lg_hist,cube->history);
    restore_user_warnings();

    cube->endx = cube->startx + (cube->nx -1)*cube->stepx;
    cube->endy = cube->starty + (cube->ny -1)*cube->stepy;
    cube->endz = cube->startz + (cube->nz -1)*cube->stepz;
    if (status <= 0) {
      cube_minmax(cube);
    }
    else {
      cube->min = cuts[2];
      cube->max = cuts[3];
    }
    status = 0;
    /* parse wcs if contained in file */
    status = parse_wcs(cube);
  }
  return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                       close_cube()
!
!.purp             closes a currently active 3D cube
!.desc
! close_cube(cube)	
!
! IMAGE3D *cube;        cube structure
!.ed
-------------------------------------------------------------------- */
int
close_cube(IMAGE3D *cube)			/* close active cube */
{
  char   errtext[132], filename[lg_name+1];
  int    stat, int_datatype;
  float  cuts[4];
#ifdef IRAF
  int one=1;
#endif
#ifdef FITS
  fitsfile *fptr;
  int npix;
#endif

  strcpy(filename,cube->name);

  if (cube->iomode == (int)I_MODE) {
    switch (cube->data_format) {
#ifdef MIDAS
    case MIDAS_FORMAT :
      stat = SCFCLO(cube->imno);
      break;
#endif
#ifdef IRAF
      /*		case IRAF_FORMAT :
                        case STSDAS_FORMAT :
                        uimclo(&(frame->imno),&stat);
                        break;  */
#endif
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :
      stat =0;
      fptr = (fitsfile *)cube->external_info;
      fits_close_file(fptr,&stat);
      free_cube_mem(cube);
      cube->external_info = NULL;
      break;
#endif
    }
    if (stat) {
      sprintf(errtext,"close_cube: cube %s",filename);
      stat = get_tiger_errcode(cube->data_format,stat);
      Handle_Error(errtext,stat);
    }
    return(stat);
  }

  if (cube->data.d_data != NULL) {
    cube_minmax(cube);

    cuts[0]=(float)cube->min; cuts[2]=(float)cube->min;
    cuts[1]=(float)cube->max; cuts[3]=(float)cube->max;
    stat = WR_desc(cube,"LHCUTS",FLOAT,4,cuts);
  }

  WR_history(cube, (Anyfile *)0);

  switch (cube->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    stat = SCFCLO(cube->imno);
    break;
#endif
#ifdef IRAF
    /*	case IRAF_FORMAT :
	case STSDAS_FORMAT :
        switch(frame->data_type) {
        case SHORT :
        uips2s(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
        frame->data.s_data,&stat);
        break;
        case INT :
        case LONG :
        uips2l(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
        frame->data.l_data,&stat);
        break;
        case FLOAT :
        uips2r(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
        frame->data.f_data,&stat);
        break;
        case DOUBLE :
        uips2d(&(frame->imno),&one,&(frame->nx),&one,&(frame->ny),
        frame->data.d_data,&stat);
        break;
        }
        if (stat == 0)
        uimclo(&(frame->imno),&stat);
        free_frame_mem(frame);
	break;*/
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    stat = 0;
    fptr = (fitsfile *)cube->external_info;
    if (cube->iomode == (int)O_MODE) {
      int_datatype = get_datatype_code(OutputIO.basic_io,cube->data_type);
      npix = cube->nx*cube->ny*cube->nz;
      if (fits_write_img(fptr,int_datatype,1L,npix,
                         cube->data.s_data,&stat)) {
        stat = ERR_WRIT;
      }
    }
    if (! stat) {
      fits_close_file(fptr,&stat);
      stat = wcs_free(cube);
    }
    free_cube_mem(cube);
    cube->external_info = NULL;
    break;
#endif
  }
  if (stat) {
    sprintf(errtext,"close_cube: cube %s",filename);
    stat = get_tiger_errcode(cube->data_format,stat);
    Handle_Error(errtext,stat);
  } else {
    if (TK && (cube->iomode == O_MODE || cube->iomode == IO_MODE))
      {
        printf("@ N {%s}\n",filename);
      }
  }

  return(stat);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_cube()
!
!.purp                    deletes a 3D cube
!.desc
! delete_cube(cube)	
!
! IMAGE2D *cube;        cube structure
!.ed
-------------------------------------------------------------------- */

int
delete_cube(IMAGE3D *cube)		
{
  int status, iomode;
  char errtext[132],filename[lg_name+1];
#ifdef IRAF
  int len;
#endif
#ifdef FITS
  fitsfile *fptr;
  char *extname;
  char file[lg_name+1];
  int hdutype, hdu_num;
#endif
	
  strcpy(filename,cube->name);

  switch (cube->data_format) {
#ifdef MIDAS
  case MIDAS_FORMAT :
    status = SCFDEL(filename);
    break;
#endif
#ifdef IRAF
  case IRAF_FORMAT :
  case STSDAS_FORMAT :
    len = strlen(filename);
    uimdel(filename,&status);
    break; 
#endif
#ifdef FITS
  case FITS_A_FORMAT :
  case FITS_B_FORMAT :
    status = 0;
    strcpy(file,filename);
    extname = strchr(file,'[');
    if (extname == NULL) {
      if (cube->external_info != NULL) {
        fptr = (fitsfile *)cube->external_info;
        fits_delete_file(fptr,&status);
      }
      else
        status = unlink(filename);
    } else {
      extname++;
      extname[strlen(extname)-1] = '\0';
      hdutype = IMAGE_HDU;
      if (cube->external_info == NULL) {
        iomode = get_iomode_code(InputIO.basic_io,IO_MODE);
        fits_open_file(&fptr,filename,iomode,&status);
      }
      else
        fptr = (fitsfile *)cube->external_info;
      if (!is_numeric(extname)) {
        fits_movabs_hdu(fptr, 1, NULL, &status);  /* reach beginning of file */
        fits_movnam_hdu(fptr,hdutype,extname,0,&status); /* search for extname */
      }
      else {
        sscanf(extname,"%d",&hdu_num);
        fits_movabs_hdu(fptr, hdu_num+1, NULL, &status); 
      }
      fits_delete_hdu(fptr,NULL,&status);
      if (cube->external_info == NULL) {
        fits_close_file(fptr,&status);
      }

    }
    break;
#endif
  }
  if (status)	{
    sprintf(errtext,"delete_cube: cube %s",filename);
    status = get_tiger_errcode(cube->data_format,status);
    Handle_Error(errtext,status);
  }
  if (TK)
    {	
      printf("@ D {%s}\n",filename);
    }
  return(status);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  routines to read/write values into image files are
  given in ../incl/funcdef.h
!     
!.func                        RD_cube()
!
!.purp                  reads a cube value
!.desc
! (type) value = RD_cube(cube,x_pixel,y_pixel,z_value)
!
! IMAGE3D *cube;        cube structure
! int x_pixel;          pixel to read along x axis
! int y_pixel;          pixel to read along y axis
! int z_pixel;          pixel to read along z axis
!.ed
!     
!.func                        WR_cube()
!
!.purp                  writes a cube value
!.desc
! WR_cube(cube,pixel_x,pixel_y,pixel_z,value)
!
! IMAGE2D *cube;        cube structure
! int x_pixel;          pixel to write along x axis
! int y_pixel;          pixel to write along y axis
! int z_pixel;          pixel to write along z axis
! (type) value;         value to write
!.ed
!
-------------------------------------------------------------------- */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                        RD_qcube()
!
!.purp                  reads a cube quality flag
!.desc
! unsigned long value = RD_qcube(cube,x_pixel,y_pixel,z_pixel)
!
! IMAGE3D *cube;        cube structure
! int x_pixel;          pixel to read along x axis
! int y_pixel;          pixel to read along y axis
! int z_pixel;          pixel to read along z axis
!.ed
-----------------------------------------------------------------------*/
unsigned long RD_qcube(IMAGE3D *frame, int i, int j, int k)
{
  if (frame->quality == NULL)
    return 0;
  else
    return frame->quality[i+frame->nx*j+(frame->nx*frame->ny)*k];
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!     
!.func                        WR_qcube()
!
!.purp                  writes a cube quality flag
!.desc
! WR_qcube(cube,pixel_x,pixel_y,pixel_z,value)
! int x_pixel;          pixel to read along x axis
! int y_pixel;          pixel to read along y axis
! int z_pixel;          pixel to read along z axis
! unsigned long value;  value to write
!.ed
-----------------------------------------------------------------------*/
int WR_qcube(IMAGE3D *frame, int i, int j, int k, unsigned long pixel_val) 
{
  if (frame->quality == NULL) { /* first allocate memory */
    frame->quality = (unsigned long *)calloc(frame->nx*frame->ny*frame->nz,sizeof(unsigned long));
    if (frame->quality == NULL)
      return(ERR_ALLOC);		
  }
  frame->quality[i+frame->nx*j+(frame->nx*frame->ny)*k] = pixel_val;
  return(0);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk                 Routines to access any kind of file
!
!.func                            open_anyfile()
!
!.purp       opens any kind of file and returns a new allocated structure
!.desc
! open_anyfile(anyfile,name,mode)	
!
! Anyfile **pointer;    pointer to anyfile structure
! char *name;           filename
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */

int open_anyfile(Anyfile **pointer, char *name, char *mode)
{
  
  int FileType;

  FileType = file_type(name);

  switch (FileType) {
  case T_TIGER: 
    {
      TIGERfile *ptig;

      ptig = (TIGERfile *)malloc(sizeof(TIGERfile));
      open_tiger_frame(ptig,name,mode);
      *pointer = (Anyfile *)ptig;
      break;
    }
  case T_TABLE:
    {
      TABLE *ptab;

      ptab = (TABLE *)malloc(sizeof(TABLE));
      open_table(ptab,name,mode);
      *pointer = (Anyfile *)ptab;
      break;
    }
  case T_IMA1D: 
    { 
      SPECTRUM *pspec;

      pspec = (SPECTRUM *)malloc(sizeof(SPECTRUM));
      header_spec(pspec,name,mode);
      *pointer = (Anyfile *)pspec;
      break;
    }
  case T_IMA2D:
    {
      IMAGE2D *pframe;

      pframe = (IMAGE2D *)malloc(sizeof(IMAGE2D));
      header_frame(pframe,name,mode);
      *pointer = (Anyfile *)pframe;
      break;
    }
  case T_IMA3D:
    {
      IMAGE3D *pframe;

      pframe = (IMAGE3D *)malloc(sizeof(IMAGE3D));
      header_cube(pframe,name,mode);
      *pointer = (Anyfile *)pframe;
      break;
    }
  default: 
    *pointer = NULL;
    return(ERR_BAD_TYPE);
  }
  return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                            close_anyfile()
!
!.purp       close any kind of file and free the structure allocated by open_anyfile
!.desc
! close_anyfile(anyfile)	
!
! Anyfile **pointer;    pointer to anyfile structure
!.ed
-------------------------------------------------------------------- */

int close_anyfile(Anyfile **pointer)
{

  switch ((*pointer)->file_type) {
  case T_TIGER: close_tiger_frame((TIGERfile *)*pointer); break;
  case T_TABLE: close_table((TABLE *)*pointer);           break;
  case T_IMA1D: close_spec((SPECTRUM *)*pointer);         break;
  case T_IMA2D: close_frame((IMAGE2D *)*pointer);         break;
  case T_IMA3D: close_cube((IMAGE3D *)*pointer);          break;
  default: 
    return(ERR_BAD_TYPE);
  }
  free(*pointer);

  return(OK);
}

