/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT 	(c) 1997 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT	        convert.c
! LANGUAGE 	C
!
! AUTHOR	Jean-louis Villecroze
!		
! KEYWORDS	
! PURPOSE 	
! COMMENT      
! VERSION 	1.0 07-01-1998 : Creation JLV
!               1.1            : AP added datacube conversion
!               1.2 12-05-2004 : AP noise conversion fix in datacubes
!               1.3 24-05-2004 : YC test if noise spectrum is non-NULL, minor tweaks
_________________________________________________________________*/

#include <stdlib.h>
#include <IFU_io.h>
#include <data_io.h>

extern IO_Format InputIO, OutputIO;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk               Routines for file conversions
!
!.func                     copy_col()
!
!.purp           copy a column from a table to another table
!.desc
! copy_col(src, dst, col)
!
! TABLE *src;          Input table
! TABLE *dst;          Output table
! int col;             column no
!.ed
--------------------------------------------------------------------*/

void copy_col(TABLE *src,TABLE *dst,int col)
{
  char name[20];
  int type;
  char format[20];
  char unit[20];
  int id,i,n;
  
  get_col_name(src,col,name);

  get_col_info(src,col,&type,format,unit);
  
  id = create_col(dst,name,(short)type,'N',format,unit);

  switch(type)
    {
    case CHAR:
      {
	char *value;
	sscanf(format,"A%d",&n);
	value = (char *)malloc((n+1)*sizeof(char));

	for (i=0;i<src->row;i++) {
          RD_tbl(src,i,col,value);
          WR_tbl(dst,i,id,value);
        }
	free(value);
	break;
      }
    case SHORT:
      {
	short value;

	for (i=0;i<src->row;i++) {
          RD_tbl(src,i,col,&value);
          WR_tbl(dst,i,id,&value);
        }
	break;
      }
    case INT:
      {
	int value;
	
	for (i=0;i<src->row;i++) {
          RD_tbl(src,i,col,&value);
          WR_tbl(dst,i,id,&value);
        }
	break;
      }
    case LONG:
      {
	long value;
	
	for (i=0;i<src->row;i++) {
          RD_tbl(src,i,col,&value);
          WR_tbl(dst,i,id,&value);
        }
	break;
      }
    case FLOAT:
      {
	float value;
	
	for (i=0;i<src->row;i++) {
          RD_tbl(src,i,col,&value);
          WR_tbl(dst,i,id,&value);
        }
	break;
      }
    case DOUBLE:
      {
	double value;
	
	for (i=0;i<src->row;i++) {
          RD_tbl(src,i,col,&value);
          WR_tbl(dst,i,id,&value);
        }
	break;
      }
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     convert_spec()
!
!.purp           		convert a spectrum 
!.desc
! convert_spec(src, dst)
!
! char *src;            Input spectrum
! char *dst;            Output spectrum
!.ed
--------------------------------------------------------------------*/

void convert_spec(char *in,char *out)
{
  SPECTRUM src,dst;

  if (open_spec(&src,in,"I")<0) {
    print_error("Cannot open input file %s",in);
    exit_session(ERR_OPEN);
  }
  else
    if (create_spec(&dst,out,src.npts,src.start,src.step,src.data_type,
                    src.ident,src.cunit)<0) {
      close_spec(&src);
      print_error("Cannot create output file %s",out);
      exit_session(ERR_CREAT);
    }
    else {
      int i;

      for (i=0;i<src.npts;i++) WR_spec(&dst,i,RD_spec(&src,i));

      CP_non_std_desc(&src,&dst);
      close_spec(&dst);
      close_spec(&src);
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     convert_table()
!
!.purp           		   convert a table
!.desc
! convert_table(src, dst)
!
! char *src;            Input table
! char *dst;            Output table
!.ed
--------------------------------------------------------------------*/

void convert_table(char *in,char *out)
{
  TABLE src,dst;
  int i;

  if (open_table(&src,in,"I")<0) {
    print_error("Cannot open input file %s",in);
    exit_session(ERR_OPEN);
  }
  else
    handle_select_flag(&src,'Q',NULL);

  if (create_table(&dst,out,src.row,src.col,'W',src.ident)<0) {
    close_table(&src);
    print_error("Cannot create output file %s",out);
    exit_session(ERR_CREAT);
  }
  else {
    reset_print_progress();
    for (i=1;i<=src.col;i++) {
      print_progress("Convert table: ", (int)((100*i)/src.col),1);
      copy_col(&src,&dst,i);
    }
	
    CP_non_std_desc(&src,&dst);
	
    close_table(&dst);
    close_table(&src);
  }
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     convert_image()
!
!.purp           		   convert a 2D image
!.desc
! convert_image(src, dst)
!
! char *src;            Input image
! char *dst;            Output image
!.ed
--------------------------------------------------------------------*/

void convert_image(char *in,char *out)
{
  IMAGE2D src,dst;
  int npix[2];
  double start[2],step[2];

  if (open_frame(&src,in,"I")<0) {
    print_error("Cannot open input file %s",in);
    exit_session(ERR_OPEN);
  }
  else {
    npix[0]=src.nx;
    npix[1]=src.ny;
    start[0]=src.startx;
    start[1]=src.starty;
    step[0]=src.stepx;
    step[1]=src.stepy;

    if (create_frame(&dst,out,npix,start,step,src.data_type,src.ident,src.cunit)<0) {
      close_frame(&src);
      print_error("Cannot create output file %s",out);
      exit_session(ERR_CREAT);
    }
    else {
      int i,j;

      for (i=0;i<src.nx;i++)
        for (j=0;j<src.ny;j++)
          WR_frame(&dst,i,j,RD_frame(&src,i,j));
	  
      CP_non_std_desc(&src,&dst);

      close_frame(&dst);
      close_frame(&src);
    }
  }
  printf("\n");
}

/* convert tiger convention "variance = -1 if bad pixel" to data quality = -1 */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     set_quality()
!
!.purp    	   Convert tiger convention "variance = -1 if bad pixel" to data quality = -1 
!.desc
! set_quality(rms)
!
! SPECTRUM *rms;            Input/output variance spectrum
!.ed
--------------------------------------------------------------------*/

void set_quality(SPECTRUM *rms) {

  int i;

  if (rms == NULL || rms->data_format != TIGER_FORMAT) return;
  for (i=0; i< rms->npts; i++) {
    if (RD_spec(rms,i) == -1) {
      if (rms->quality == NULL)
        rms->quality = (long *)calloc(rms->npts,sizeof(long));
      rms->quality[i] = 1;
    }
  }
  interpolate_noise(rms);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     convert_datacube()
!
!.purp    	   convert a datacube (Euro3D & Tiger formats)
!.desc
! convert_image(src, dst)
!
! char *src;            Input datacube
! char *dst;            Output datacube
!.ed
--------------------------------------------------------------------*/

void convert_datacube(char *in,char *out)
{
  E3D_file src,dst;
  SPECTRUM sig, noise, *rms, *signal;
  SPAXEL *spaxels;
  E3Dspec_desc *ptdesc;
  int i, specId, nbspax, npix=-1;
  double start = -1, end;
  char extname[lg_name+1], tablename[lg_name+1];

  if (open_E3D_file(&src,in,"I")<0) {
    print_error("Cannot open input file %s",in);
    exit_session(ERR_OPEN);
  }
  else {
    if (has_common_bounds(&src)) {
      get_common_param(&src,&npix,&start,&end);
    }
    if (create_E3D_file(&dst,out,npix,start,src.step,src.data_type,src.ident,src.cunit) < 0) {
      close_E3D_file(&src);
      print_error("Cannot create output file %s",out);
      exit_session(ERR_CREAT);
    }
    else {
      signal = NULL;
      rms = NULL;
      ptdesc = NULL;
      if (src.signal != NULL) {
        signal = &sig;
        ptdesc = src.signal;
      }
      if (src.noise != NULL) {
        rms = &noise;
        ptdesc = src.noise;
      }
      else { 
        if (src.signal == NULL) {
          close_E3D_file(&src);
          print_error("Inconsistent input file (no spectra inside)");
          exit_session(ERR_BAD_PARAM);
        }
      }

      reset_print_progress();
      for (i=0;i<src.nbspec;i++) {
        print_progress("Convert datacube: ", (int)((100*i)/src.nbspec),1);
        specId = ptdesc[i].specId;
        get_E3D_spec(&src,signal,rms,specId);
        set_quality(rms);
        put_E3D_spec(&dst,signal,rms,specId);
        nbspax = get_E3D_spaxels(&src,specId,&spaxels);
        put_E3D_spaxels(&dst,specId,nbspax,spaxels);
      }
      if (src.data_format == TIGER_FORMAT) {	/* set groups */
        set_tiger_group(&src);
      }
      put_E3D_groups(&dst,src.ngroups,src.groups);
      print_progress("Convert datacube: ", (int)100,1);
	  
      CP_non_std_desc(&src,&dst);
      close_E3D_file(&src);
      close_E3D_file(&dst);

      printf("\n");

      if (src.data_format == TIGER_FORMAT) {	/* save table in special extension */
        get_science_table_name(in,tablename);
	append_tbl_extension(out, OutputIO.basic_io);
        sprintf(extname,"%s[%s]",out,E3D_TIGER_EXT);
        convert_table(tablename,extname);
      }
      if (dst.data_format == TIGER_FORMAT) {	/* save science extension as table */
        strcpy(tablename,out);
	remove_file_extension(tablename);
        sprintf(extname,"%s[%s]",in,E3D_TIGER_EXT);
        convert_table(extname,tablename);
      }
    }
  }
}
