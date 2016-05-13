/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        compat.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      3D files i/o (compatibility with old stuff)
! COMMENT      
! VERSION      1.0  2002-Dec-03 AP creation, AP
!              1.1  2005-Sep-16 YC bug correction on interpolate_noise
---------------------------------------------------------------------*/

#include <IFU_io.h>
#include <data_io.h>
/* Ignore values.h if we have STDC_HEADERS */
#ifdef HAVE_VALUES_H
#include <values.h>
#endif

#include <fitsio.h>
#include <longnam.h>

extern void confirme_erase(char *name);
extern int ASK;
extern IO_Format InputIO, OutputIO;

#define CREAT_FILE_PERM 0666
#define OPEN_FILE_PERM 0644
#define DATA_UNDEF -MAXSHORT
#define LENS_UNDEF -MAXINT

#define Tiger_lg_name   80L

#define find_lens(frame,type_of_spec,no_lens,i) \
	for (*(i)=0; *(i)<frame->nbspec \
		&& frame->type_of_spec[*(i)].specId != no_lens \
		&& frame->type_of_spec[*(i)].specId != LENS_UNDEF; (*(i))++); \
	if ((frame->type_of_spec[*(i)].specId == LENS_UNDEF) \
		|| (*(i) == frame->nbspec)) \
		*(i) = -1

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.blk                 Routines for 3D FRAMES i/o   
!     
!.func                    set_tiger_group()
!
!.purp        	set a tiger group description in a E3D file
!.desc
! set_tiger_group(frame)
!
! TIGERfile *frame;     Tiger file structure
!.ed
-------------------------------------------------------------------- */

int set_tiger_group(E3D_file *frame)
{
	char instrument[lg_label+1];

	instrument[0] = 0;
	frame->ngroups = 1;
	frame->groups = (GROUP *)malloc(1*sizeof(GROUP));
	frame->groups[0].groupId = 1;      /* group number                    */
	frame->groups[0].shape = SQUARE;   /* default is square               */
	frame->groups[0].size1 = 1;        /* first spaxel size parameter (mm)*/
	frame->groups[0].size2 = 0;        /* second spaxel size parameter    */

	disable_user_warnings();
	RD_desc(frame,"INSTRUME",CHAR,lg_label,instrument);
	upper_strg(instrument);
	restore_user_warnings();

	/* default setting */

        frame->groups[0].shape = SQUARE;   /* shape keyword                   */
        frame->groups[0].size1 = 1;        /* first spaxel size parameter (mm)*/
        frame->groups[0].size2 = 0;        /* second spaxel size parameter    */

	/* check if instrument is already known */

	if (strncmp(instrument,"OASIS",5) == 0) {
		frame->groups[0].shape = HEXAGON;  /* shape keyword                   */
		frame->groups[0].size1 = 1.19;     /* first spaxel size parameter (mm)*/
		frame->groups[0].size2 = 0;        /* second spaxel size parameter    */
	}
	if (strncmp(instrument,"SAURON",6) == 0) {
		frame->groups[0].shape = SQUARE;   /* shape keyword                   */
		frame->groups[0].size1 = 1.308;    /* first spaxel size parameter (mm)*/
		frame->groups[0].size2 = 0;        /* second spaxel size parameter    */
        }
        if (strncmp(instrument,"SNIFS",5) == 0) {
	        frame->groups[0].shape = SQUARE;   /* shape keyword                   */
	        frame->groups[0].size1 = 0.42;     /* first spaxel size parameter (mm)*/
	        frame->groups[0].size2 = 0;        /* second spaxel size parameter    */
	}
	frame->groups[0].angle = 0;                /* orientation of the spaxel       */
	frame->groups[0].poswav = 0;               /* wavelength for ADC              */
	frame->groups[0].airmass = 0;              /* airmass                         */
	frame->groups[0].parang = 0;               /* paralactic angle for ADC        */
	frame->groups[0].pressure = 0;             /* pressure for ADC                */
	frame->groups[0].temperature = 0;          /* temperature for ADC             */
	frame->groups[0].rel_humidity = 0;         /* relative humidity               */
	return(0);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.func                    create_tiger_frame()
!
!.purp        creates a new Tiger frame according to specifications
!.desc
! create_tiger_frame(frame,name,npix,start,step,datatype,
!                                          tbl_name,ident,unit)
!
! TIGERfile *frame;     Tiger file structure
! char  *name;          file name
! int    npix;          nb of pixels (if common bounds, else -1)
! double start;         start value for lambda (if common bounds)
! double step;          step value for lambda
! short  datatype;      type of data storage
! char  *tbl_name;      name of associated table
! char  *ident;         identifier
! char  *unit;          units
!.ed
-------------------------------------------------------------------- */

int
create_tiger_frame(E3D_file *frame,char *name,int npix,double start,double step,short datatype,char *table_name,char *ident,char *unit)
{
	int status;

	switch (InputIO.datacubes) { /* set table_name to be stored in header */

		case TIGER_FORMAT:
		strcpy(frame->table_name,table_name);
		break;

		case EURO3D_FORMAT :
		sprintf(frame->table_name,"%s[%s]",frame->name,E3D_DATA);
		break;
	}

	status = create_E3D_file(frame,name,npix,start,step,datatype,ident,unit);

	switch (frame->data_format) {

		case TIGER_FORMAT:	/* overwrite create_E3D_file default */
		strcpy(frame->table_name,table_name);
		break;
	}

	if (!status) 
		set_tiger_group(frame);

	return(status);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.func                    close_tiger_frame()
!
!.purp        	set a tiger group description in a E3D file
!.desc
! close_tiger_frame(frame)
!
! TIGERfile *frame;     Tiger file structure
!.ed
-------------------------------------------------------------------- */

int close_tiger_frame(E3D_file *frame)
{
	set_tiger_group(frame);
	return(close_E3D_file(frame));
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_tiger_spec()
!
!.purp             delete a spectrum in a E3D frame
!.desc
! delete_tiger_spec(frame,signal,noise,specId)	
!
! E3D_file *frame;      E3D file structure
! SPECTRUM *signal;     signal spectrum structure
! SPECTRUM *noise;      noise spectrum structure
! int specId;           spectrum ID
!.ed
-------------------------------------------------------------------- */
int 
delete_tiger_spec(E3D_file *frame, SPECTRUM *signal, SPECTRUM *noise, int specId)
{
	int status, i=-1, j=-1;
	char errtext[132];
	E3Dspec_desc *current_spec;
	
	status = ERR_NOIMA;
	if (frame->signal != NULL && signal != NULL) {
	    find_lens(frame,signal,specId,&i);
	    if (i >= 0) {
		current_spec = frame->signal+i;
		if (i < (frame->nbspec -1))
			memcpy((char *)current_spec,(char *)(frame->signal+i+1),
				(frame->nbspec-i-1)*sizeof(E3Dspec_desc));
		status = free_spec_mem(signal);
	    }
	}
	if (frame->noise != NULL && noise != NULL) {
	    find_lens(frame,noise,specId,&j);
	    if (j >= 0) {
		current_spec = frame->noise+j;
		if (j < (frame->nbspec -1))
			memcpy((char *)current_spec,(char *)(frame->noise+j+1),
				(frame->nbspec-j-1)*sizeof(E3Dspec_desc));
		status = free_spec_mem(noise);
	    }
	}
	if (i>=0 || j>=0)
		frame->nbspec--;
	if (status)	{
		sprintf(errtext,"delete_tiger_spec: spec %s",signal->name);
		Handle_Error(errtext,status);
	}
	return(status);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                    set_bigendian()
!
!.purp             check the way bytes are stored
!.desc
! int set_bigendian(short type) {
!
! short  type;          units
!.ed
-------------------------------------------------------------------- */

int set_bigendian(short type) {

	union {
		short type;
		char octets[2];
	} bytes;

	bytes.type = type;
	if (bytes.octets[0] == 0) {
		bytes.type = 1;
		if (bytes.octets[0] == 0) 
			return(0);
		else
			return(1);
	} else {
		bytes.type = 1;
		if (bytes.octets[0] == 0) 
			return(1);
		else
			return(0);
	}
	
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        get_coord_table_name()
!
!.purp          returns the table name associated with a 3D frame
!.desc
! get_coord_table_name(ima_name,tab_name)	
!
! char *ima_name;       frame name
! char *tab_name;       table name
!.ed
-------------------------------------------------------------------- */
int 
get_coord_table_name(char *name, char *tab_name)		
{
	char errtext[132], filename[lg_name+1];
  	int status=0;
  	int iomode;
	char buffer[lg_hist+1];
	int imno;

	strcpy(filename,name);
 	first_blk(filename); 
	iomode = O_RDONLY;
	
	append_datacube_extension(filename,InputIO.datacubes);

	switch (InputIO.datacubes) {

	case TIGER_FORMAT :
  	    imno = open(filename,iomode,OPEN_FILE_PERM);
  	    if (imno <0) {
		status = ERR_OPEN; 
		sprintf(errtext,"get_coord_table_name: frame %s",filename);
		Handle_Error(errtext,status);
  		return(status);
	    }
	    read(imno,buffer,lg_version);

  	    if (strncmp(buffer,"v1.0",4) != 0) {
		close(imno);
		status = ERR_BAD_TYPE; 
		sprintf(errtext,"get_coord_table_name: frame %s",filename);
		Handle_Error(errtext,status);
  		return(status);
	    };
	    read(imno,buffer,lg_ident);
	    read(imno,buffer,lg_unit);
	    read(imno,tab_name,lg_name);
	    close(imno);
	    break;

	case EURO3D_FORMAT :
	    status = 0;
	    sprintf(tab_name,"%s[%s]",filename,E3D_DATA);
	    break;
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        get_science_table_name()
!
!.purp          returns the table name associated with a 3D frame
!.desc
! get_science_table_name(ima_name,tab_name)	
!
! char *ima_name;       frame name
! char *tab_name;       table name
!.ed
-------------------------------------------------------------------- */
int 
get_science_table_name(char *name, char *tab_name)		
{
	char errtext[132], filename[lg_name+1];
  	int status=0;
  	int iomode;
	char buffer[lg_hist+1];
	int imno;

	strcpy(filename,name);
 	first_blk(filename); 
	iomode = O_RDONLY;
	
	append_datacube_extension(filename,InputIO.datacubes);

	switch (InputIO.datacubes) {

	case TIGER_FORMAT :
  	    imno = open(filename,iomode,OPEN_FILE_PERM);
  	    if (imno <0) {
		status = ERR_OPEN; 
		sprintf(errtext,"get_science_table_name: frame %s",filename);
		Handle_Error(errtext,status);
  		return(status);
	    }
	    read(imno,buffer,lg_version);

  	    if (strncmp(buffer,"v1.0",4) != 0) {
		close(imno);
		status = ERR_BAD_TYPE; 
		sprintf(errtext,"get_science_table_name: frame %s",filename);
		Handle_Error(errtext,status);
  		return(status);
	    };
	    read(imno,buffer,lg_ident);
	    read(imno,buffer,lg_unit);
	    strcpy(tab_name,get_path(filename));
            read(imno,filename,Tiger_lg_name);
            remove_path(filename);
            strcat(tab_name,"/");
            strcat(tab_name,filename);
	    close(imno);
	    break;

	case EURO3D_FORMAT :
	    status = 0;
	    sprintf(tab_name,"%s[%s]",filename,E3D_TIGER_EXT);
	    break;
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                  get_lenses_no_from_table()
!
!.purp              return lenses number from associated table
!.desc
! nlens = get_lenses_no_from_table(frame,specId)	
!
! E3D_file *frame;      3D file structure
! int *specId;          list of lens numbers 
!.ed
-------------------------------------------------------------------- */
int get_lenses_no_from_table(E3D_file *frame, int *specId)	
{
	int status = 0;
	int i, j, k;
	char errtext[132];
	char table_name[lg_name+1];
	int col_no, nol;
	TABLE tbl_lens;
	E3Dspec_desc *pt_spec = NULL;

	if (frame->signal != NULL) {
		pt_spec = frame->signal;
	}
	else {
		if (frame->noise != NULL) 
			pt_spec = frame->noise;
	}
	strcpy(table_name,frame->table_name);
	append_tbl_extension(table_name,InputIO.basic_io);
	status = open_table(&tbl_lens,table_name,"I");
	if (!status) {
		handle_select_flag(&tbl_lens,'W',NULL);
		col_no = get_col_ref(&tbl_lens,E3D_COL_ID);
		if (col_no < 0) {
			status = ERR_NOCOL;
			sprintf(errtext,"get_lenses_no_from_table");
			Handle_Error(errtext,status);
			return(status);
		}
		if (pt_spec != NULL) {
		    for (i=0, j=0; i<tbl_lens.row;i++) {
			RD_tbl(&tbl_lens,i,col_no,&nol);
			for (k=0; k<frame->nbspec && pt_spec[k].specId != nol; k++);
			if (pt_spec[k].specId == nol) {
				specId[j] = nol;
				j++;
			}
		    }
		}
		else { /* new data cube */

		    for (i=0; i< tbl_lens.row;i++) {
			RD_tbl(&tbl_lens,i,col_no,&nol);
			specId[i] = nol;
		    }
		    j = i;
		}
		close_table(&tbl_lens);
	};
	if (status) {
		sprintf(errtext,"get_lenses_no_from_table");
		Handle_Error(errtext,status);
		return(status);
	}
	return(j);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        get_lenses_coord()
!
!.purp               get spectra ID and coordinates
!.desc
! get_lenses_coord(frame,lab_xcol,lab_ycol,nb_spec,Id,x,y,i)	
!
! E3D_file *frame;      E3D file structure
! char *lab_xcol;       column label for x coord
! char *lab_ycol;       column label for y coord
! int nb_spec;          number of spectra
! int *Id;              list of spectrum ID
! float *x;             list of x coordinates
! float *y;             list of y coordinates
! int *i;               list of table rows
!.ed
-------------------------------------------------------------------- */
int get_lenses_coord(E3D_file *frame, char *lab_xcol, char *lab_ycol, 
		     int *specId, float *xlens, float *ylens, int *il)	
{
	int status = 0;
	int i, col_no, col_xl, col_yl, found=1, k;
	char errtext[132];
	char table_name[lg_name+1];
	E3Dspec_desc *pt_spec = NULL;
	TABLE tbl_lens;

	if (frame->signal != NULL)
	  pt_spec = frame->signal;
	else {
	  if (frame->noise != NULL)
	      pt_spec = frame->noise;
	}
        strcpy(table_name,frame->table_name);
	append_tbl_extension(table_name,InputIO.basic_io);
	if (frame->data_format == EURO3D_FORMAT) {
		if ((strcmp(lab_xcol,E3D_COL_XPOS) != 0)
			&& (strcmp(lab_ycol,E3D_COL_YPOS) != 0)) {
			/* use Tiger science table */
			get_science_table_name(frame->name,table_name);
		}
	}

	status = open_table(&tbl_lens,table_name,"I");
	if (!status) {
		col_no = get_col_ref(&tbl_lens,E3D_COL_ID);
		col_xl = get_col_ref(&tbl_lens,lab_xcol);
		col_yl = get_col_ref(&tbl_lens,lab_ycol);
		if ((col_no < 0) || (col_xl < 0) || (col_yl < 0)) {
			status = ERR_NOCOL;
			sprintf(errtext,"get_lenses_coord");
			Handle_Error(errtext,status);
			return(status);
		}
		if (pt_spec == NULL) {
			RD_col(&tbl_lens,col_no,specId);
			RD_col(&tbl_lens,col_xl,xlens);
			RD_col(&tbl_lens,col_yl,ylens);
			for (i=0; i<tbl_lens.row;i++) 
				il[i] = i;
		}
		else {
			for (i=0; i<frame->nbspec && found ;i++) {
				specId[i] = pt_spec[i].specId;
				k = search_in_col(&tbl_lens,col_no,specId+i);
				if (k < 0) {
					found = 0;
					status = ERR_NODATA;
				}
				else {
					RD_tbl(&tbl_lens,k,col_xl,xlens+i);
					RD_tbl(&tbl_lens,k,col_yl,ylens+i);
					il[i] = k;
				}
			}
		}
		close_table(&tbl_lens);
	};
	if (status) {
		sprintf(errtext,"get_lenses_coord");
		Handle_Error(errtext,status);
		return(status);
	}
	return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        set_lenses_coord()
!
!.purp                    set spectra ID and coordinates in extension also
!.desc
! set_lenses_coord(frame,lab_xcol,lab_ycol,nb_spec,Id,x,y)
!
! E3D_file *frame;      E3D file structure
! char *lab_xcol;       column label for x coord
! char *lab_ycol;       column label for y coord
! int nb_spec;          number of spectra
! int *Id;              list of spectrum ID
! float *x;             list of x coordinates
! float *y;             list of y coordinates
!.ed
-------------------------------------------------------------------- */
int set_lenses_coord(E3D_file *frame,char *lab_xcol,char *lab_ycol,int nbl,int *specId, float *xlens, float *ylens)
{
        int status = 0;
        int i, col_no, col_xl, col_yl;
        char errtext[132];
        TABLE tbl_lens;
        char table_name[lg_name+1];

	set_ID_and_coordinates(frame,nbl,specId,xlens,ylens);

        strcpy(table_name,frame->table_name);
        append_tbl_extension(table_name,InputIO.basic_io);

        if (exist(table_name)) {
                status = open_table(&tbl_lens,table_name,"IO");
        } else {
                status = create_table(&tbl_lens,table_name,nbl,3,'W',NULL);
        }
        if (status < 0) {
                sprintf(errtext,"set_lenses_coord");
                Handle_Error(errtext,status);
                return(status);
        }
        col_no = create_col(&tbl_lens,LAB_COL_NO,LONG,'R',"I4","none");
        col_xl = create_col(&tbl_lens,lab_xcol,FLOAT,'R',"F9.6","pixels");
        col_yl = create_col(&tbl_lens,lab_ycol,FLOAT,'R',"F9.6","pixels");
        for (i=0;i<nbl;i++) {
                WR_tbl(&tbl_lens,i,col_no,specId+i);
                WR_tbl(&tbl_lens,i,col_xl,xlens+i);
                WR_tbl(&tbl_lens,i,col_yl,ylens+i);
        }
        close_table(&tbl_lens);
        return(status);

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        get_lenses_coord_select()
!
!.purp              get associated lenses number and coordinates
!.desc
! get_lenses_coord_select(frame,lab_xcol,lab_ycol,specId,xl,yl,il,sel)	
!
! E3D_file *frame;      3D file structure
! char *lab_xcol;       column label for x coord
! char *lab_ycol;       column label for y coord
! int *specId;          list of lens numbers 
! float *xl;            list of x coordinates
! float *yl;            list of y coordinates
! int *il;              list of table rows
! int *nbsel;           nb of select lenses
!.ed
-------------------------------------------------------------------- */
int get_lenses_coord_select(E3D_file *frame, char *lab_xcol, char *lab_ycol, 
			    int *specId, float *xlens, float *ylens, 
			    int *il, int *nbsel)	
{
	int status = 0;
	int i, col_no, col_xl, col_yl, k, *no, l;
	char errtext[132];
	char table_name[lg_name+1];
	E3Dspec_desc *pt_spec = NULL;
	TABLE tbl_lens;

	if (frame->signal != NULL)
	  pt_spec = frame->signal;
	else {
	  if (frame->noise != NULL)
	    pt_spec = frame->noise;
	}
	strcpy(table_name,frame->table_name);
	status = open_table(&tbl_lens,table_name,"I");
	handle_select_flag(&tbl_lens, 'W', errtext);

	if (!status) {
	  col_no = get_col_ref(&tbl_lens,E3D_COL_ID);
	  col_xl = get_col_ref(&tbl_lens,lab_xcol);
	  col_yl = get_col_ref(&tbl_lens,lab_ycol);
	  if ((col_no < 0) || (col_xl < 0) || (col_yl < 0)) {
	    status = ERR_NOCOL;
	    sprintf(errtext,"get_lenses_coord");
	    Handle_Error(errtext,status);
	    return(status);
	  }
	  if (pt_spec == NULL) {
	    RD_col(&tbl_lens,col_no,specId);
	    RD_col(&tbl_lens,col_xl,xlens);
	    RD_col(&tbl_lens,col_yl,ylens);
	    for (i=0; i<tbl_lens.row;i++) 
	      il[i] = i;
	    *nbsel = tbl_lens.row;
	  }
	  else {
	    no = (int *)malloc(tbl_lens.row*sizeof(int));
	    RD_col(&tbl_lens,col_no,no);
	    *nbsel = 0;
	    for (i=0; i<frame->nbspec; i++) {
	      specId[*nbsel] = pt_spec[i].specId;
	      for (k=-1, l=0; l<tbl_lens.row; l++) {
		if (specId[*nbsel] == no[l]) {
		  k = l;
		  break;
		}
	      }
	      if (k < 0) {
		continue;
	      }
	      else {
		RD_tbl(&tbl_lens,k,col_xl,&(xlens[*nbsel]));
		RD_tbl(&tbl_lens,k,col_yl,&(ylens[*nbsel]));
		il[*nbsel] = k;
		(*nbsel)++;
	      }
	    }
	    free(no);
	  }
	  close_table(&tbl_lens);
	};
	if (status) {
		sprintf(errtext,"get_lenses_coord_select");
		Handle_Error(errtext,status);
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      init_new_tiger_slice()
!
!.purp               returns an initialised slice structure items
!.desc
! init_new_tiger_slice(frame,slice,npts)	
!
! E3D_file *frame;      E3D file structure
! SLICE *slice;         slice structure
! int npts;                  nb of values in slice
!.ed
-------------------------------------------------------------------- */
int 
init_new_tiger_slice(E3D_file *frame, SLICE *slice, int npts)
{
	char errtext[132];
  	int status, i;
	E3Dspec_desc *pt_spec = NULL;
	
	slice->index = -1;
	slice->npts = npts;
	slice->data_type = frame->data_type;

	status = 0;
	slice->specId = (int *)calloc(slice->npts,sizeof(int));
	if (slice->specId == 0) status = ERR_ALLOC;
	if (frame->signal != NULL)
		pt_spec = frame->signal;
	else {
	    if (frame->noise != NULL)
        	pt_spec = frame->noise;
	}
	if (pt_spec == NULL) {
		get_lenses_no_from_table(frame,slice->specId);
	}
	else {
		for (i=0;i<slice->npts;i++)
			slice->specId[i] = pt_spec[i].specId;
	}

	if (status == 0) {
    	switch(slice->data_type) {
        case SHORT :
            slice->data.s_data = (short *)calloc(slice->npts,sizeof(short));
            if (slice->data.s_data == 0) status = ERR_ALLOC;
            break;
        case INT :
        case LONG :
            slice->data.l_data = (long *)calloc(slice->npts,sizeof(long));
            if (slice->data.l_data == 0) status = ERR_ALLOC;
            break;
        case FLOAT :
            slice->data.f_data = (float *)calloc(slice->npts,sizeof(float));
            if (slice->data.f_data == 0) status = ERR_ALLOC;
            break;
        case DOUBLE :
            slice->data.d_data = (double *)calloc(slice->npts,sizeof(double));
            if (slice->data.d_data == 0) status = ERR_ALLOC;
            break;
    	}
        slice->quality = (unsigned long *)calloc(slice->npts,sizeof(unsigned long));
	}
  	if (status) {
		sprintf(errtext,"init_new_tiger_slice");
		Handle_Error(errtext,status);
	}
  	return(status);
}
#define extract_3D_slice_noalloc(type_of_slice) \
        type_of_slice->index = index; \
        current_spec = &(frame->type_of_slice[0]); \
        for (i=0;i <frame->nbspec; i++, current_spec++) { \
                type_of_slice->specId[i] = current_spec->specId; \
                if (index < current_spec->npix) { \
                        lseek(frame->imno,(current_spec->data_offset \
                                +index*sizeof_item(type_of_slice->data_type)),SEEK_SET); \
                        if (frame->swapbytes) {\
                                read_DOS(frame->imno,(char *)((char *)(type_of_slice->data.s_data)+\
                                        i*sizeof_item(type_of_slice->data_type)), \
                                        sizeof_item(type_of_slice->data_type)); \
                        } else { \
                                read(frame->imno,(char *)((char *)(type_of_slice->data.s_data)+\
                                        i*sizeof_item(type_of_slice->data_type)), \
                                        sizeof_item(type_of_slice->data_type)); \
                        } \
                }\
                else \
                        WR_slice(type_of_slice,i,DATA_UNDEF); \
        }

int 
get_3D_slice_noalloc(E3D_file *frame, SLICE *signal, SLICE *noise, 
			int index)
{
	char errtext[132];
  	int i, status=0;
	E3Dspec_desc *current_spec;

	if (! has_common_bounds(frame)) {
		sprintf(errtext,"get_3D_slice : Unconsistent data");
		Handle_Error(errtext,ERR_BAD_PARAM); 
		return(ERR_BAD_PARAM); 
	}

	if (signal != NULL) {
		extract_3D_slice_noalloc(signal);
	} 
	if (noise != NULL) {
		extract_3D_slice_noalloc(noise);
	} 

  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        get_lens_coordinates()
!
!.purp                   get associated coordinates to one lens
!.desc
! get_lens_coordinates(frame,specId,xl,yl)	
!
! E3D_file *frame;      file structure
! int specId;           lens numbers 
! float *xl;            x coordinates
! float *yl;            y coordinates
!.ed
-------------------------------------------------------------------- */
int get_lens_coordinates(E3D_file *frame, int specId, float *xlens, float *ylens) 
{
	int status = 0;
	int i, col_no, col_xl, col_yl, no;
	char errtext[132];
	char table_name[lg_name+1];
	TABLE tbl_lens;

        strcpy(table_name,frame->table_name);
	status = open_table(&tbl_lens,table_name,"I");

	no = specId;

	if (!status) {
	  col_no = get_col_ref(&tbl_lens,LAB_COL_NO);
	  col_xl = get_col_ref(&tbl_lens,LAB_COL_XLD);
	  col_yl = get_col_ref(&tbl_lens,LAB_COL_YLD);

	  if ((col_no < 0) || (col_xl < 0) || (col_yl < 0)) {
	    status = ERR_NOCOL;
	    sprintf(errtext,"get_lens_coordinates");
	    Handle_Error(errtext,status);
	    return(status);
	  }
	  i = search_in_col(&tbl_lens,col_no,&no);
	  RD_tbl(&tbl_lens,i,col_xl,xlens);
	  RD_tbl(&tbl_lens,i,col_yl,ylens);
	  close_table(&tbl_lens);

	} else {
		sprintf(errtext,"get_lens_coordinates");
		Handle_Error(errtext,status);
	}
	return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        set_lens_coordinates()
!
!.purp                   set associated coordinates to one lens
!.desc
! set_lens_coordinates(frame,specId,xl,yl)	
!
! E3D_file *frame;      3D file structure
! int specId;           lens numbers 
! float *xl;            x coordinates
! float *yl;            y coordinates
!.ed
-------------------------------------------------------------------- */
int set_lens_coordinates(E3D_file *frame, int specId, float *xlens, float *ylens) 
{
	int status = 0;
	int i, col_no, col_xl, col_yl, no;
	char errtext[132];
	char table_name[lg_name+1];
	TABLE tbl_lens;

	no = specId;

        strcpy(table_name,frame->table_name);
	append_tbl_extension(table_name,OutputIO.basic_io);

        if (exist(table_name)) {
                status = open_table(&tbl_lens,table_name,"IO");
		if (status) {
		        sprintf(errtext,"set_lens_coordinates");
                	Handle_Error(errtext,status);
                	return(status);
		}
        	col_no = get_col_ref(&tbl_lens,LAB_COL_NO);
		if (col_no >= 0)
	  		i = search_in_col(&tbl_lens,col_no,&no);
		else
			col_no = create_col(&tbl_lens,LAB_COL_NO,LONG,'R',"I4","none");

		if (i < 0) i = tbl_lens.row;
        	col_xl = get_col_ref(&tbl_lens,LAB_COL_XLD);
		if (col_xl < 0)
			col_xl = create_col(&tbl_lens,LAB_COL_XLD,FLOAT,'R',"F9.6","pixels");
        	col_yl = get_col_ref(&tbl_lens,LAB_COL_YLD);
		if (col_yl < 0)
			col_yl = create_col(&tbl_lens,LAB_COL_YLD,FLOAT,'R',"F9.6","pixels");
        } else {
                status = create_table(&tbl_lens,table_name,-1,3,'W',NULL);
		if (status) {
		        sprintf(errtext,"set_lens_coordinates");
                	Handle_Error(errtext,status);
                	return(status);
		}
        	col_no = create_col(&tbl_lens,LAB_COL_NO,LONG,'R',"I4","none");
        	col_xl = create_col(&tbl_lens,LAB_COL_XLD,FLOAT,'R',"F9.6","pixels");
        	col_yl = create_col(&tbl_lens,LAB_COL_YLD,FLOAT,'R',"F9.6","pixels");
		i = 0;
        }

  	if ((col_no < 0) || (col_xl < 0) || (col_yl < 0)) {
    		status = ERR_NOCOL;
    		sprintf(errtext,"set_lens_coordinates");
    		Handle_Error(errtext,status);
    		return(status);
  	}
  	WR_tbl(&tbl_lens,i,col_no,&no);
  	WR_tbl(&tbl_lens,i,col_xl,xlens);
  	WR_tbl(&tbl_lens,i,col_yl,ylens);
  	close_table(&tbl_lens);

	return(status);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.blk                 Routines for 3D maxima files i/o   
!     
!.func                    alloc_3D_max()
!
!.purp            creates/allocates a new 3D maxima frame
!.desc
! alloc_3D_max(maxima,nblines,nbmax_by_line)
!
! Maxima_Set *maxima;   3D maxima file structure
! int nblines;          maximum number of lines to store
! int nbmax_by_line     max. nb. of maxima to store by line
!.ed
-------------------------------------------------------------------- */

int
alloc_3D_max(Maxima_Set *maxima, int nblines, int nbmax_by_line)
{
	int i, j;
	int nby = nblines, max_pts = nbmax_by_line;

	maxima->line = (max_lines *)malloc(nby*sizeof(max_lines));
	if (maxima->line == NULL) {
		Handle_Error("alloc_3D_max",ERR_ALLOC);
		return(ERR_ALLOC);
	}
	for (i=0; i<nby; i++) {
		maxima->line[i].ycoord = -MAXFLOAT;
		maxima->line[i].nb_max=0;
		maxima->line[i].maxima = (max_param *)malloc(max_pts*sizeof(max_param));
		if (maxima->line[i].maxima == NULL) {
			Handle_Error("alloc_3D_max",ERR_ALLOC);
			return(ERR_ALLOC);
		}
		for (j=0; j<max_pts; j++) {
			maxima->line[i].maxima[j].xcoord = -MAXFLOAT;
			maxima->line[i].maxima[j].intens = -MAXFLOAT;
			maxima->line[i].maxima[j].sigma[0] = -MAXFLOAT;
			maxima->line[i].maxima[j].sigma[1] = -MAXFLOAT;
			maxima->line[i].maxima[j].alpha = -MAXFLOAT;
		}
	}
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.func                      load_3D_max()
!
!.purp                 load a 3D maxima frame
!.desc
! load_3D_max(maxima, filename)
!
! Maxima_Set *maxima;   3D maxima file structure
! char *filename;       name of maxima frame
!.ed
-------------------------------------------------------------------- */
int
load_3D_max(Maxima_Set *maxima, char *name)
{
	int fdmax, i, j, ipix;
	char errtext[132], filename[lg_name+1];

	strcpy(filename,name);
	if (strchr(filename,'.') == NULL)
		strcat(filename,".max");
  	fdmax = open(filename,O_RDONLY,OPEN_FILE_PERM);
	if (fdmax < 0) {
		sprintf(errtext,"load_3D_max: frame %s",filename);
		Handle_Error(errtext,ERR_OPEN);
		return(ERR_OPEN);
	}
	read(fdmax,maxima->config_name,21*sizeof(char));
	read(fdmax,&(maxima->enlarger.gamma),sizeof(double));
	read(fdmax,&(maxima->wedges.theta_w),sizeof(double));
	read(fdmax,&(maxima->grism.index_coef),sizeof(int));
	read(fdmax,&(maxima->grism.A),sizeof(double));
	read(fdmax,&(maxima->grism.g_per_mm),sizeof(int));
	read(fdmax,&(maxima->filter.inf_util),sizeof(double));
	read(fdmax,&(maxima->filter.sup_util),sizeof(double));
	read(fdmax,&(maxima->telescope.fratio),sizeof(float));
   	if ((maxima->telescope.fratio < 5) || (maxima->telescope.fratio > 50)) {
		/* old format ! */
    	lseek(fdmax,-sizeof(float),SEEK_CUR);
		read(fdmax,&ipix,sizeof(int));
		maxima->telescope.fratio = (float)ipix;
	}
	read(fdmax,&(maxima->ccd.nx),sizeof(int));
	read(fdmax,&(maxima->ccd.ny),sizeof(int));
	read(fdmax,&(maxima->ccd.pixsize),sizeof(float));
   	if ((maxima->ccd.pixsize < 5) || (maxima->ccd.pixsize > 50)) {
		/* old format ! */
    	lseek(fdmax,-sizeof(float),SEEK_CUR);
		read(fdmax,&ipix,sizeof(int));
		maxima->ccd.pixsize = (float)ipix;
	}
	read(fdmax,&(maxima->nb_ycoords),sizeof(int));
	maxima->line = (max_lines *)malloc(maxima->nb_ycoords*sizeof(max_lines));
	if (maxima->line == NULL) {
		close(fdmax);
		Handle_Error("load_3D_max",ERR_ALLOC);
		return(ERR_ALLOC);
	}
	for (i=0; i<maxima->nb_ycoords; i++) {
		read(fdmax,&(maxima->line[i].ycoord),sizeof(float));
		read(fdmax,&(maxima->line[i].nb_max),sizeof(int));
		if (maxima->line[i].nb_max <= 0) continue;
		maxima->line[i].maxima = 
			(max_param *)malloc(maxima->line[i].nb_max*sizeof(max_param));
		if (maxima->line[i].maxima == NULL) {
			close(fdmax);
			Handle_Error("load_3D_max",ERR_ALLOC);
			return(ERR_ALLOC);
		}
		for (j=0; j<maxima->line[i].nb_max; j++) {
			read(fdmax,&(maxima->line[i].maxima[j].xcoord),sizeof(float));
			read(fdmax,&(maxima->line[i].maxima[j].intens),sizeof(float));
			read(fdmax,&(maxima->line[i].maxima[j].sigma[0]),sizeof(float));
			read(fdmax,&(maxima->line[i].maxima[j].sigma[1]),sizeof(float));
			read(fdmax,&(maxima->line[i].maxima[j].alpha),sizeof(float));
		}
	}
	close(fdmax);
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.func                      save_3D_max()
!
!.purp                  save a 3D maxima frame
!.desc
! save_3D_max(maxima, filename)
!
! Maxima_Set *maxima;   3D maxima file structure
! char *filename;       name of maxima frame
!.ed
-------------------------------------------------------------------- */
int
save_3D_max(Maxima_Set *maxima, char *name)
{
	int fdmax, i, j;
	char errtext[132], filename[lg_name+1];

	strcpy(filename,name);
	if (strchr(filename,'.') == NULL)
		strcat(filename,".max");

	if (ASK && exist(filename))
	  confirme_erase(filename);

	if ((fdmax = creat(filename,CREAT_FILE_PERM))<0) {
		sprintf(errtext,"save_3D_max: frame %s",filename);
		Handle_Error(errtext,ERR_CREAT);
		return(ERR_CREAT);
	}
	write(fdmax,maxima->config_name,21*sizeof(char));
	write(fdmax,&(maxima->enlarger.gamma),sizeof(double));
	write(fdmax,&(maxima->wedges.theta_w),sizeof(double));
	write(fdmax,&(maxima->grism.index_coef),sizeof(int));
	write(fdmax,&(maxima->grism.A),sizeof(double));
	write(fdmax,&(maxima->grism.g_per_mm),sizeof(int));
	write(fdmax,&(maxima->filter.inf_util),sizeof(double));
	write(fdmax,&(maxima->filter.sup_util),sizeof(double));
	write(fdmax,&(maxima->telescope.fratio),sizeof(float));
	write(fdmax,&(maxima->ccd.nx),sizeof(int));
	write(fdmax,&(maxima->ccd.ny),sizeof(int));
	write(fdmax,&(maxima->ccd.pixsize),sizeof(float));
	write(fdmax,&(maxima->nb_ycoords),sizeof(int));
	for (i=0; i<maxima->nb_ycoords; i++) {
		write(fdmax,&(maxima->line[i].ycoord),sizeof(float));
		write(fdmax,&(maxima->line[i].nb_max),sizeof(int));
		for (j=0; j<maxima->line[i].nb_max; j++) {
			write(fdmax,&(maxima->line[i].maxima[j].xcoord),sizeof(float));
			write(fdmax,&(maxima->line[i].maxima[j].intens),sizeof(float));
			write(fdmax,&(maxima->line[i].maxima[j].sigma[0]),sizeof(float));
			write(fdmax,&(maxima->line[i].maxima[j].sigma[1]),sizeof(float));
			write(fdmax,&(maxima->line[i].maxima[j].alpha),sizeof(float));
		}
	}
	close(fdmax);
	if(TK)
	  printf("@ N {%s}\n",filename);

	return(OK);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk               Miscellaneous routines to interpolate noise data 
!
!.func                       interpolate_noise
!
!.purp          interpolate -1 values for noise spectrum 
!.desc
! int interpolate_noise(noise)
!
! SPECTRUM *noise;      noise spectrum
!.ed
----------------------------------------------------------------------------*/

int interpolate_noise(SPECTRUM *noise) {

  int i, i0, status;
  float x1, y1, x2, y2, a, b;

  i = 0;
  x1 = 0;
  status = 0;


  while (i<noise->npts) {
    for (; i<noise->npts && RD_spec(noise,i)!=-1; i++); /* find 1st-1 */
    if (i == noise->npts) return(status);
    if (i == 0) {               /* very 1st-pixel is affected */
      for (i++; i<noise->npts && RD_spec(noise,i)==-1; i++); /* find first value */
      if (i == noise->npts) return(status);
      y1 = RD_spec(noise,i);
      for (i0=0; i0<i; i0++) WR_spec(noise,i0,y1);
    }
    else {
      y1 = (float)RD_spec(noise,i-1);
      x1 = (float)i-1;
      /* find 2nd non--1 value */
      for (i++; i<noise->npts && RD_spec(noise,i)==-1; i++);
      if (i < noise->npts) {
        y2 = (float)RD_spec(noise,i);
        x2 = (float)i;
        a = (y2 - y1)/(x2 -x1);
        b = y1 - a*x1;
        /* linear interpolation */
        for (i0=(int)x1+1; i0<i; i0++) WR_spec(noise,i0,a*i0+b);
      }
      else {
        /* very last pixel is affected (no need to test for i>=0) */
        for (i--; RD_spec(noise,i)==-1; i--);
        y2 = RD_spec(noise,i);
        for (i++; i<noise->npts; i++) WR_spec(noise,i,y2);
      }
    }
  } 
  return(status);
}
