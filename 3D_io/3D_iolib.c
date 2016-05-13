/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        3D_iolib.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      3D files i/o 
! COMMENT      
! VERSION      4.0  1994-Jul-22 : creation, AR
! VERSION      4.1  2002-Dec-02 : convert to E3D fits format
! VERSION      4.2  2003-Mar-06 : modify common bounds processing in init_new_E3D_spec
! VERSION      4.3  2003-Apr-08 : change save_E3D_slice so specId in slices are taken into account
! VERSION      4.4  2004-Dec-15 : added put_E3D_frame
! VERSION      4.5  2005-Jan-06 : suppress generic types TBL_TYPE et IMA_TYPE
! VERSION      4.6  2005-Sep-16 : file_type was not set for datacubes
---------------------------------------------------------------------*/

#include <IFU_io.h>
#include <data_io.h>
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

#define find_lens(frame,type_of_spec,specId,i) \
	if (frame->type_of_spec == NULL) *(i) = -1; \
	else { \
	for (*(i)=0; *(i)<frame->nbspec \
		&& frame->type_of_spec[*(i)].specId != specId \
		&& frame->type_of_spec[*(i)].specId != LENS_UNDEF; (*(i))++); \
	if ((frame->type_of_spec[*(i)].specId == LENS_UNDEF) \
		|| (*(i) == frame->nbspec)) \
		*(i) = -1; \
	}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.blk               General scheme (to be read prior to use !)
!     
! The E3D_file structure contains fields describing the whole set of 
! spectra (number of spectra, data type, binning step) and pointers to 
! arrays of spectra headers (signal and noise). Assuming 'frame' is the 
! name of the E3D file, one can retrieve the main parameters, using :
! \
! frame.ident
! frame.cunit
! frame.history
! frame.table_name (i.e. name of the associated table)
! frame.nbspec
! frame.step
! frame.data_type (i.e. short, int, float, double ...)
! \
! frame.spectrum[i].specId
! frame.spectrum[i].npix 
! frame.spectrum[i].start 
! frame.spectrum[i].end 
! frame.spectrum[i].min
! frame.spectrum[i].max
! \
! 'i' beeing the index of the spectrum in the cube (from 0 to nbspec-1).
! \
! The E3D_file may be accessed slice by slice. 
!
-------------------------------------------------------------------- */

#define Tiger_lg_name   80L

#define HEADER_LENGTH	(lg_version+lg_ident+lg_unit+Tiger_lg_name)*sizeof(char) \
				+2*sizeof(int)+2*sizeof(short)+sizeof(double)

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!      
!.blk                 Routines for E3D FRAMES i/o   
!     
!.func                    create_E3D_file()
!
!.purp        creates a new E3D frame according to specifications
!.desc
! create_E3D_file(frame,name,npix,start,step,datatype,ident,unit)
!
! E3D_file *frame;      E3D file structure
! char  *name;          file name
! int    npix;          nb of pixels (if common bounds, else -1)
! double start;         start value for lambda (if common bounds)
! double step;          step value for lambda
! short  datatype;      type of data storage 
! char  *ident;         identifier
! char  *unit;          units
!.ed
-------------------------------------------------------------------- */

int 
create_E3D_file(E3D_file *frame,char *name,int npix,double start,double step,short datatype,char *ident,char *unit)
{
	char errtext[132], filename[lg_name+1], tablename[lg_name+1];
	int status=0;
	TABLE table;
	fitsfile *fptr;

           /* set everything to zero, except table name (tiger compat) */
	strncpy(tablename,frame->table_name,lg_name);
	tablename[lg_name] = '\0';
	memset((char *)frame,0, sizeof(E3D_file)); 
	strcpy(frame->table_name, tablename);

  	strcpy(frame->version,"v1.0");

	strcpy(filename,name);
  	first_blk(filename);
	append_datacube_extension(filename,OutputIO.datacubes);
	strcpy(frame->name,filename);

	frame->imno = -1;
  	frame->ident[0] = '\0';
  	frame->history[0] = '\0';
  	strcpy(frame->cunit,"");
  	frame->step = step;
	frame->iomode = (int)O_MODE;
	frame->data_type = datatype;
	frame->file_type = -1;
	frame->data_format = OutputIO.datacubes;
	if (npix < 0) 
		frame->common_bounds = 0;
	else {
		frame->common_bounds = 1;
		frame->common_parameters[0] = start;
		frame->common_parameters[1] = start + (double)(npix-1)*step;
		frame->common_parameters[2] = (double)npix;
	}
  	
	/* if units specified, stores them */
  	if (ident != NULL) strcpy(frame->ident,ident);
  	if (unit != NULL) {
  		memcpy(frame->cunit,unit,lg_unit);
  		frame->cunit[lg_unit] = '\0';
  	}
  	
	if(ASK)
		confirme_erase(filename);

	switch (OutputIO.datacubes) {

	case TIGER_FORMAT :
		frame->data_offset = HEADER_LENGTH;
		frame->file_type = T_TIGER;
		frame->crop = 1;

		if ((frame->imno = creat(filename,CREAT_FILE_PERM))<0)  
			status = ERR_CREAT;
		else {		
				/* writing header */
			write(frame->imno,frame->version,lg_version);
			write(frame->imno,frame->ident,lg_ident);
			write(frame->imno,frame->cunit,lg_unit);
			write(frame->imno,frame->table_name,Tiger_lg_name);
			write(frame->imno,&(frame->data_type),sizeof(short));
			write(frame->imno,&(frame->step),sizeof(double));
			write(frame->imno,&(frame->nbspec),sizeof(int));
			write(frame->imno,&(frame->common_bounds),sizeof(short));
			write(frame->imno,&(frame->extra_hd_off),sizeof(int));
			/* close and reopen to get read access */
			close(frame->imno);
  			frame->imno = open(filename,O_RDWR,OPEN_FILE_PERM);
		}
		remove_file_extension(filename);
		append_tbl_extension(filename,OutputIO.basic_io);
		strcpy(frame->table_name,filename);
		break;

	case EURO3D_FORMAT :
		frame->data_offset = 0;
		frame->file_type = T_TIGER;
		frame->crop = 0;
		sprintf(frame->table_name,"%s[%s]",filename,E3D_DATA);
                status =0;

                if (exist(filename))
                    delete_E3D_file(frame);

                if (fits_create_file(&fptr,filename,&status)) {
                    status = ERR_WRIT; break;
                }
                frame->external_info = (void *)fptr;
		frame->imno = 0;

		fits_write_imghdr(fptr, 8, 0, NULL, &status);
                fits_write_key_str(fptr, "EURO3D", "T", "All mandatory EuroE3D extensions present", &status);
		fits_write_key_str(fptr, E3D_ADC, "F", "data corrected from atm dispersion" , &status);
		fits_write_key_str(fptr, E3D_VERS, "1.0", "version number of the EuroE3D format" , &status);
		sprintf(tablename,"%s[%s]",filename,E3D_DATA);
		status = create_table(&table,tablename,-1,9,'Q',NULL);

		create_col(&table,E3D_COL_ID,LONG,'R',"I8","none");
		create_col(&table,E3D_COL_FLAG,LONG,'R',"I1","none");
		create_col(&table,E3D_COL_NSPAX,LONG,'R',"I8","none");
		create_col(&table,E3D_COL_NPIX,LONG,'R',"I8","none");
		create_col(&table,E3D_COL_IDX,LONG,'R',"I8","none");
		create_col(&table,E3D_COL_XPOS,FLOAT,'R',"F8","none");
		create_col(&table,E3D_COL_YPOS,FLOAT,'R',"F8","none");
		create_col(&table,E3D_COL_GRP,LONG,'R',"I8","none");
		create_col(&table,E3D_COL_SPAXID,CHAR,'R',"A1","none");

		close_table(&table);
		fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
		if (status)
                 	status = ERR_WRIT;
		break;

	default :
		status = ERR_FORMAT;
		break;
	}
	
	if (status) {
        	sprintf(errtext,"create_E3D_file: frame %s",filename);
        	Handle_Error(errtext,status);
	} else {
		WR_desc(frame,"COMMENT",CHAR,8,"       ");
	}
  	return(status);
} 

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        open_E3D_file()
!
!.purp          opens a E3D frame and updates the structure items
!.desc
! open_E3D_file(frame,name,mode)	
!
! E3D_file *frame;      E3D file structure
! char *name;           frame name
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */
int 
open_E3D_file(E3D_file *frame, char *name, char *mode)		
{
	char errtext[132], filename[lg_name+1];
	char descr_name[lg_label+1], format[lg_label+1];
	char tbl_ext[lg_name+1];
	short codesize, data_type;
  	int status=0, i, j, colno, colnpix, colidx;
	int colint, colgid, colsh, cols1, cols2, 
	colang, colpang, colpwav, colairm, colpres, coltemp, colhum;
  	int iomode, nbdesc, nb_values, user_desc, idx;
	E3Dspec_desc *current_spec;
	TABLE tblpos, tblgrp;
	long refpix;
	int int_datatype, type=0;
	Descriptor *descr;
	fitsfile *fptr;
	double min, max, start, step;

	memset((char *)frame,0, sizeof(E3D_file)); /* set everything to zero */

	strcpy(filename,name);
 	first_blk(filename); 
  	strcpy(frame->name,filename);
	memset(frame->ident,' ',lg_ident);
	frame->ident[lg_ident] = '\0';
	memset(frame->cunit,' ',lg_unit);
	frame->cunit[lg_unit] = '\0';
	memset(frame->history,' ',lg_hist);
	frame->history[lg_hist] = '\0';
	frame->file_type = -1;
	frame->imno = -1;

	switch(mode[0]) {
		case 'I' : 
			if (mode[1] == 'O') {
				frame->iomode = (int)IO_MODE;
				iomode = O_RDWR;
			}
			else {
				frame->iomode = (int)I_MODE;
				iomode = O_RDONLY;
			}
			break;
		case 'O' : frame->iomode = (int)O_MODE;
				iomode = O_WRONLY;
			break;
		default  : frame->iomode = (int)I_MODE;
				iomode = O_RDONLY;
			break;
	}
	
	append_datacube_extension(frame->name,InputIO.datacubes);

	strcpy(filename,frame->name);

	switch(InputIO.datacubes) {

	case TIGER_FORMAT :
		frame->data_format = TIGER_FORMAT;
		frame->file_type = T_TIGER;
		frame->crop = 1;
	  	frame->imno = open(filename,iomode,OPEN_FILE_PERM);
	  	if (frame->imno <0) {
			status = ERR_OPEN; 
			sprintf(errtext,"open_E3D_file: frame %s",frame->name);
			Handle_Error(errtext,status);
	  		return(status);
		}
		read(frame->imno,frame->version,lg_version);
		frame->version[4] = 0;
	
	  	if (strcmp(frame->version,"v1.0") != 0) {
			close(frame->imno);
			status = ERR_BAD_TYPE; 
			sprintf(errtext,"open_E3D_file: frame %s",frame->name);
			Handle_Error(errtext,status);
	  		return(status);
		};
		read(frame->imno,frame->ident,lg_ident);
		read(frame->imno,frame->cunit,lg_unit);
		strcpy(frame->table_name,get_path(filename));
		read(frame->imno,filename,Tiger_lg_name);
		remove_path(filename);
		strcat(frame->table_name,"/");
		strcat(frame->table_name,filename);
		read(frame->imno,&(frame->data_type),sizeof(short));
		frame->swapbytes = set_bigendian(frame->data_type);
		if (frame->swapbytes) {
			lseek(frame->imno,-sizeof(short),SEEK_CUR);
			read_DOS(frame->imno,&(frame->data_type),sizeof(short));
		}
	    	status = sizeof_item(frame->data_type);
	  	if (status < 0) {
			close(frame->imno);
			status = ERR_BAD_TYPE; 
			sprintf(errtext,"open_E3D_file: frame %s",frame->name);
			Handle_Error(errtext,status);
	  		return(status);
		}
		else
			status = 0;
		if (frame->swapbytes) {
			read_DOS(frame->imno,&(frame->step),sizeof(double));
			read_DOS(frame->imno,&(frame->nbspec),sizeof(int));
		} else {
			read(frame->imno,&(frame->step),sizeof(double));
			read(frame->imno,&(frame->nbspec),sizeof(int));
		}
	  	if (frame->nbspec < 0) {
			close(frame->imno);
			status = ERR_BAD_TYPE; 
			sprintf(errtext,"open_E3D_file: frame %s",frame->name);
			Handle_Error(errtext,status);
	  		return(status);
		}
		if (frame->swapbytes) {
			read_DOS(frame->imno,&(frame->common_bounds),sizeof(short));
			read_DOS(frame->imno,&(frame->extra_hd_off),sizeof(int));
		} else {
			read(frame->imno,&(frame->common_bounds),sizeof(short));
			read(frame->imno,&(frame->extra_hd_off),sizeof(int));
		}
		frame->data_offset = HEADER_LENGTH;
	
		lseek(frame->imno,frame->extra_hd_off,SEEK_SET);
	
		if (frame->nbspec != 0)
			frame->signal = (E3Dspec_desc *)malloc(frame->nbspec*sizeof(E3Dspec_desc));
		current_spec = frame->signal;
		if (frame->swapbytes) {
		    for (i=0; i < frame->nbspec ;i++, current_spec++) {
			read_DOS(frame->imno,&(current_spec->specId),sizeof(int));
			if (current_spec->specId == LENS_UNDEF) break;
			read_DOS(frame->imno,&(current_spec->start),sizeof(double));
			read_DOS(frame->imno,&(current_spec->end),sizeof(double));
			read_DOS(frame->imno,&(current_spec->npix),sizeof(int));
			read_DOS(frame->imno,&min,sizeof(double));
			read_DOS(frame->imno,&max,sizeof(double));
			read_DOS(frame->imno,&(current_spec->data_offset),sizeof(int));
			frame->data_offset += current_spec->npix*sizeof_item(frame->data_type); 
		    }
		} else {
		    for (i=0; i < frame->nbspec ;i++, current_spec++) {
			read(frame->imno,&(current_spec->specId),sizeof(int));
			if (current_spec->specId == LENS_UNDEF) break;
			read(frame->imno,&(current_spec->start),sizeof(double));
			read(frame->imno,&(current_spec->end),sizeof(double));
			read(frame->imno,&(current_spec->npix),sizeof(int));
			read(frame->imno,&min,sizeof(double));
			read(frame->imno,&max,sizeof(double));
			read(frame->imno,&(current_spec->data_offset),sizeof(int));
			frame->data_offset += current_spec->npix*sizeof_item(frame->data_type); 
		    }
		}
		if (i == 0 && frame->signal != NULL) {
			free((char *)frame->signal);
			frame->noise = NULL;
		}
		if (frame->nbspec != 0 && frame->noise == NULL)
			frame->noise = (E3Dspec_desc *)malloc(frame->nbspec*sizeof(E3Dspec_desc));
		current_spec = frame->noise;
		if (frame->swapbytes) {
		    for (i=0; i < frame->nbspec ;i++, current_spec++) {
			read_DOS(frame->imno,&(current_spec->specId),sizeof(int));
			if (current_spec->specId == LENS_UNDEF) break;
			read_DOS(frame->imno,&(current_spec->start),sizeof(double));
			read_DOS(frame->imno,&(current_spec->end),sizeof(double));
			read_DOS(frame->imno,&(current_spec->npix),sizeof(int));
			read_DOS(frame->imno,&min,sizeof(double));
			read_DOS(frame->imno,&max,sizeof(double));
			read_DOS(frame->imno,&(current_spec->data_offset),sizeof(int));
			frame->data_offset += current_spec->npix*sizeof_item(frame->data_type); 
		    }
		} else {
		    for (i=0; i < frame->nbspec ;i++, current_spec++) {
			read(frame->imno,&(current_spec->specId),sizeof(int));
			if (current_spec->specId == LENS_UNDEF) break;
			read(frame->imno,&(current_spec->start),sizeof(double));
			read(frame->imno,&(current_spec->end),sizeof(double));
			read(frame->imno,&(current_spec->npix),sizeof(int));
			read(frame->imno,&min,sizeof(double));
			read(frame->imno,&max,sizeof(double));
			read(frame->imno,&(current_spec->data_offset),sizeof(int));
			frame->data_offset += current_spec->npix*sizeof_item(frame->data_type); 
		    }
		}
		if (i == 0 && frame->noise != NULL) {
			free((char *)frame->noise);
			frame->noise = NULL;
		}
		if (frame->swapbytes)
	    		user_desc = read_DOS(frame->imno,&nbdesc,sizeof(int));
		else
	    		user_desc = read(frame->imno,&nbdesc,sizeof(int));
		if (user_desc > 0) {
					/* read user defined descriptors */
	
	      /* if nbdesc greater 1000 it's probably seems that the file is corrupted 
			so we return an error */
	
	      if (nbdesc>1000)
		{
		  status = ERR_READ;
		  sprintf(errtext,"open_E3D_file: frame %s",frame->name);
		  Handle_Error(errtext,status);
		  return(status);
		}
	      
	      if (frame->swapbytes) {
	      	    for (i=0; i<nbdesc; i++) {
			memset(descr_name,0,lg_label+1);
			read(frame->imno,descr_name,lg_label*sizeof(char));
			read_DOS(frame->imno,&data_type,sizeof(short));
			read_DOS(frame->imno,&nb_values,sizeof(int));
			alloc_new_desc((Anyfile *)(frame),data_type,nb_values);
			descr = ((Descr_Items *)(frame->external_info))->descr_list;
			descr += i;
			strcpy(descr->descr_name,descr_name);
			codesize = (short)sizeof_item(descr->data_type);
			if (data_type != CHAR) 
				for (j=0;j<nb_values;j++)
					read_DOS(frame->imno,&(descr->descr_value.c_data[j*codesize]),codesize);
			else
				read(frame->imno,&(descr->descr_value.c_data[0]),descr->nb_values*codesize);
			
			((Descr_Items *)(frame->external_info))->nb_descr++;
		    }
		} else {
	      	    for (i=0; i<nbdesc; i++) {
			memset(descr_name,0,lg_label+1);
			read(frame->imno,descr_name,lg_label*sizeof(char));
			read(frame->imno,&data_type,sizeof(short));
			read(frame->imno,&nb_values,sizeof(int));
			alloc_new_desc((Anyfile *)(frame),data_type,nb_values);
			descr = ((Descr_Items *)(frame->external_info))->descr_list;
			descr += i;
			strcpy(descr->descr_name,descr_name);
			codesize = (short)sizeof_item(descr->data_type);
			read(frame->imno,&(descr->descr_value.c_data[0]),descr->nb_values*codesize);
			((Descr_Items *)(frame->external_info))->nb_descr++;
		    }
		}
	}
	lseek(frame->imno,frame->data_offset,SEEK_SET);
	break;

	case EURO3D_FORMAT :
		frame->data_format = EURO3D_FORMAT;
		frame->file_type = T_TIGER;
		sprintf(frame->table_name,"%s[%s]",filename,E3D_TIGER_EXT);
                status = 0;
		iomode = get_iomode_code(InputIO.basic_io,frame->iomode);
                if (fits_open_file(&fptr,filename,iomode,&status)) {
                        status = ERR_OPEN; break;
                }
                frame->external_info = (void *)fptr;
		frame->imno = 0;

		fits_read_key_str(fptr, E3D_VERS,frame->version, NULL, &status);
	
	  	if (strncmp(frame->version,"1.0",3) != 0) {
			close(frame->imno);
			status = ERR_BAD_TYPE; 
			break;
		};
		sprintf(frame->table_name,"%s[%s]",filename,E3D_DATA);
		fits_read_key_str(fptr, "OBJECT",frame->ident, NULL, &status);

		sprintf(tbl_ext,"%s[%s]",filename,E3D_DATA);
		status = open_table(&tblpos,tbl_ext,mode);
	  	if (status < 0) {
			close_table(&tblpos);
			status = ERR_TBL_EXT; 
			break;
		}

		handle_select_flag(&tblpos,'W',NULL);

		RD_desc(&tblpos,E3D_KW_UNITS,CHAR,lg_label,frame->cunit);
		RD_desc(&tblpos,E3D_KW_START,DOUBLE,1,&start);
		RD_desc(&tblpos,E3D_KW_STEP,DOUBLE,1,&step);
		RD_desc(&tblpos,E3D_KW_REFPIX,LONG,1,&refpix);

		if (refpix != 1) {
			start += (refpix -1)*step;
		}
		colnpix = get_col_ref(&tblpos,E3D_COL_NPIX);
		colidx = get_col_ref(&tblpos,E3D_COL_IDX);

		colint = get_col_ref(&tblpos,E3D_COL_INT);
	  	if (colint < 0) {
			close_table(&tblpos);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblpos, colint, &int_datatype, NULL, NULL);
        	frame->data_type = (short)int_datatype;

		colno = get_col_ref(&tblpos,E3D_COL_ID);
	  	if (colno < 0) {
			close_table(&tblpos);
			status = ERR_BAD_COL; 
			break;
		}
		frame->step = step;
		frame->nbspec = tblpos.row;

		if (frame->nbspec != 0)
			frame->signal = (E3Dspec_desc *)malloc(frame->nbspec*sizeof(E3Dspec_desc));

		current_spec = frame->signal;

		for (j=0; j < frame->nbspec ;j++, current_spec++) {
			RD_tbl(&tblpos,j,colno,&(current_spec->specId));
			RD_tbl(&tblpos,j,colnpix,&(current_spec->npix));
			RD_tbl(&tblpos,j,colidx,&idx);
			current_spec->start = start + idx*step;;
			current_spec->end = start+(idx+current_spec->npix -1)*step;
			min = MAXFLOAT;
			max = -MAXFLOAT;
			if (tblpos.select_flag)
				current_spec->data_offset = tblpos.sel_row[j];
			else
				current_spec->data_offset = j;
		}

		if (j == 0 && frame->signal != NULL) {
			free((char *)frame->signal);
			frame->signal = 0x0;
		}

		disable_user_warnings();
		colint = get_col_ref(&tblpos,E3D_COL_RMS);
		restore_user_warnings();

	  	if (colint > 0) {

		    if (frame->nbspec != 0)
			frame->noise = (E3Dspec_desc *)
				malloc(frame->nbspec*sizeof(E3Dspec_desc));
		    current_spec = frame->noise;

		    j = 0;
	  	    if (status == 0) {
		    	for (j=0; j < frame->nbspec ;j++, current_spec++) {
			    RD_tbl(&tblpos,j,colno,&(current_spec->specId));
			    RD_tbl(&tblpos,j,colnpix,&(current_spec->npix));
			    RD_tbl(&tblpos,j,colidx,&idx);
			    current_spec->start = start + idx*step;;
			    current_spec->end = start+(idx+current_spec->npix -1)*step;
			    min = MAXFLOAT;
			    max = -MAXFLOAT;
			    if (tblpos.select_flag)
				current_spec->data_offset = tblpos.sel_row[j];
			    else
				current_spec->data_offset = j;
		    	}
		    }
		    else
		       status = 0;

		    if (j == 0 && frame->noise != NULL) {
			free((char *)frame->noise);
			frame->noise = 0x0;
		    }
		}
		/* parse WCS headers */
		close_table(&tblpos);
		status = parse_wcs(&tblpos);
		frame->wcs = tblpos.wcs;
		frame->nwcs = tblpos.nwcs;

		sprintf(tbl_ext,"%s[%s]",filename,E3D_GRP);
		status = open_table(&tblgrp,tbl_ext,mode);
	  	if (status < 0) {
			close_table(&tblgrp);
			status = ERR_TBL_EXT; 
			break;
		}
		frame->ngroups = tblgrp.row;
		if (frame->ngroups > 0) 
			frame->groups = malloc(frame->ngroups*sizeof(GROUP));
		colgid = get_col_ref(&tblgrp,E3D_COL_GRP);
	  	if (colgid < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		colsh = get_col_ref(&tblgrp,E3D_COL_SHAPE);
	  	if (colsh < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		cols1 = get_col_ref(&tblgrp,E3D_COL_SIZE1);
	  	if (cols1 < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, cols1,&type, format, frame->s1_unit);

		colang = get_col_ref(&tblgrp,E3D_COL_ANGLE);
	  	if (colang < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, colang,&type, format, frame->ang_unit);

		cols2 = get_col_ref(&tblgrp,E3D_COL_SIZE2);
	  	if (cols2 < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, cols2,&type, format, frame->s2_unit);

		colpwav = get_col_ref(&tblgrp,E3D_COL_PWAVE);
	  	if (colpwav < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, colpwav,&type, format, frame->pw_unit);

		colairm = get_col_ref(&tblgrp,E3D_COL_AIRM);
	  	if (colairm < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		colpang = get_col_ref(&tblgrp,E3D_COL_PANG);
	  	if (colpang < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, colpang,&type, format, frame->pa_unit);

		colpres = get_col_ref(&tblgrp,E3D_COL_PRES);
	  	if (colpres < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, colpres,&type, format, frame->pr_unit);

		coltemp = get_col_ref(&tblgrp,E3D_COL_TEMP);
	  	if (coltemp < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, coltemp,&type, format, frame->temp_unit);

		colhum = get_col_ref(&tblgrp,E3D_COL_HUM);
	  	if (colhum < 0) {
			close_table(&tblgrp);
			status = ERR_BAD_COL; 
			break;
		}
		get_col_info(&tblgrp, colhum,&type, format, frame->hum_unit);

		for (j=0; j< frame->ngroups; j++) {
			RD_tbl(&tblgrp,j,colgid,&(frame->groups[j].groupId));
			RD_tbl(&tblgrp,j,colsh,&(frame->groups[j].shape));
			RD_tbl(&tblgrp,j,cols1,&(frame->groups[j].size1));
			RD_tbl(&tblgrp,j,colang,&(frame->groups[j].angle));
			RD_tbl(&tblgrp,j,cols2,&(frame->groups[j].size2));
			RD_tbl(&tblgrp,j,colpwav,&(frame->groups[j].poswav));
			RD_tbl(&tblgrp,j,colairm,&(frame->groups[j].airmass));
			RD_tbl(&tblgrp,j,colpang,&(frame->groups[j].parang));
			RD_tbl(&tblgrp,j,colpres,&(frame->groups[j].pressure));
			RD_tbl(&tblgrp,j,coltemp,&(frame->groups[j].temperature));
			RD_tbl(&tblgrp,j,colhum,&(frame->groups[j].rel_humidity));
		}
		close_table(&tblgrp);
		fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);

	        /* checks for common bounds for all spectra */
	        set_common_bounds(frame);
		break;

	default :
		status = ERR_FORMAT;
		break;
    }
    if (frame->common_bounds) {
	memset(frame->common_parameters,0,3*sizeof(double));
	if (frame->signal != NULL)
		current_spec = frame->signal;
	frame->common_parameters[0] = current_spec->start;
	frame->common_parameters[1] = current_spec->end;
	frame->common_parameters[2] = (int)current_spec->npix;
    }
    
    if (!status) {
    	disable_user_warnings();
    	RD_desc(frame,"HISTORY",CHAR,lg_hist,frame->history);
    	restore_user_warnings();
    } else {
	sprintf(errtext,"open_E3D_file: frame %s",frame->name);
	Handle_Error(errtext,status);
    }
    return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                       close_E3D_file()
!
!.purp             closes a currently active E3D frame 
!.desc
! close_E3D_file(frame)	
!
! E3D_file *frame;      E3D file structure
!.ed
-------------------------------------------------------------------- */
int 
close_E3D_file(E3D_file *frame)			/* close active frame */
{
	E3Dspec_desc *current_spec;
	Descr_Items *dsc_items;
	Descriptor *descr;
	char   errtext[132];
	char   tbl_ext[lg_name+1];
	double min=0, max=0;
	int    colgid, colsh, cols1, cols2, colang, colpang, colpwav, colairm, colpres, coltemp, colhum;
	short  codesize;
	int    stat=0, i, j, none=LENS_UNDEF;
	fitsfile *fptr;
	TABLE  tblgrp;

	if (frame->iomode != (int)I_MODE) {

		switch (frame->data_format) {

		case TIGER_FORMAT :

		    /* checks for common bounds for all spectra */
		    set_common_bounds(frame);

		    frame->extra_hd_off = frame->data_offset;

		    lseek(frame->imno,HEADER_LENGTH -2*sizeof(short) -2*sizeof(int)
			-sizeof(double) - Tiger_lg_name, SEEK_SET);

		    remove_path(frame->table_name);
		    write(frame->imno,frame->table_name,Tiger_lg_name);

		    if (frame->swapbytes) {
			write_DOS(frame->imno,&(frame->data_type),sizeof(short));
			write_DOS(frame->imno,&(frame->step),sizeof(double));
			write_DOS(frame->imno,&(frame->nbspec),sizeof(int));
			write_DOS(frame->imno,&(frame->common_bounds),sizeof(short));
			write_DOS(frame->imno,&(frame->extra_hd_off),sizeof(int));
		    } else {
			write(frame->imno,&(frame->data_type),sizeof(short));
			write(frame->imno,&(frame->step),sizeof(double));
			write(frame->imno,&(frame->nbspec),sizeof(int));
			write(frame->imno,&(frame->common_bounds),sizeof(short));
			write(frame->imno,&(frame->extra_hd_off),sizeof(int));
		    }
		    lseek(frame->imno,frame->extra_hd_off,SEEK_SET);
		    if (frame->swapbytes) {
		        if (frame->signal != NULL) {
			    current_spec = frame->signal;
			for (i=0; i < frame->nbspec ;i++, current_spec++) {
				write_DOS(frame->imno,&(current_spec->specId),sizeof(int));
				write_DOS(frame->imno,&(current_spec->start),sizeof(double));
				write_DOS(frame->imno,&(current_spec->end),sizeof(double));
				write_DOS(frame->imno,&(current_spec->npix),sizeof(int));
				write_DOS(frame->imno,&min,sizeof(double));
				write_DOS(frame->imno,&max,sizeof(double));
				write_DOS(frame->imno,&(current_spec->data_offset),sizeof(int));
			}
		    }
		    else
			write_DOS(frame->imno,&none,sizeof(int));
		    } else {
		        if (frame->signal != NULL) {
			    current_spec = frame->signal;
			for (i=0; i < frame->nbspec ;i++, current_spec++) {
				write(frame->imno,&(current_spec->specId),sizeof(int));
				write(frame->imno,&(current_spec->start),sizeof(double));
				write(frame->imno,&(current_spec->end),sizeof(double));
				write(frame->imno,&(current_spec->npix),sizeof(int));
				write(frame->imno,&min,sizeof(double));
				write(frame->imno,&max,sizeof(double));
				write(frame->imno,&(current_spec->data_offset),sizeof(int));
			}
		    }
		    else
			write(frame->imno,&none,sizeof(int));
		    }

		    if (frame->swapbytes) {
		        if (frame->noise != NULL) {
			    current_spec = frame->noise;
			for (i=0; i < frame->nbspec ;i++, current_spec++) {
				write_DOS(frame->imno,&(current_spec->specId),sizeof(int));
				write_DOS(frame->imno,&(current_spec->start),sizeof(double));
				write_DOS(frame->imno,&(current_spec->end),sizeof(double));
				write_DOS(frame->imno,&(current_spec->npix),sizeof(int));
				write_DOS(frame->imno,&min,sizeof(double));
				write_DOS(frame->imno,&max,sizeof(double));
				write_DOS(frame->imno,&(current_spec->data_offset),sizeof(int));
			}
		       }
		       else
			    write_DOS(frame->imno,&none,sizeof(int));
		    } else {
		        if (frame->noise != NULL) {
			    current_spec = frame->noise;
			for (i=0; i < frame->nbspec ;i++, current_spec++) {
				write(frame->imno,&(current_spec->specId),sizeof(int));
				write(frame->imno,&(current_spec->start),sizeof(double));
				write(frame->imno,&(current_spec->end),sizeof(double));
				write(frame->imno,&(current_spec->npix),sizeof(int));
				write(frame->imno,&min,sizeof(double));
				write(frame->imno,&max,sizeof(double));
				write(frame->imno,&(current_spec->data_offset),sizeof(int));
			}
		       }
		       else
			    write(frame->imno,&none,sizeof(int));
		    }

    		    WR_history(frame, (Anyfile *)0);

		    /* save user descriptors */

		    if (frame->external_info != NULL) {
			dsc_items = (Descr_Items *)frame->external_info;
			if (frame->swapbytes) {
			    write_DOS(frame->imno,&(dsc_items->nb_descr),sizeof(int));
			    for (i=0; i< dsc_items->nb_descr; i++) {
				descr = dsc_items->descr_list+i;
				codesize = (short)sizeof_item(descr->data_type);
				write(frame->imno,descr->descr_name,lg_label*sizeof(char));
				write_DOS(frame->imno,&(descr->data_type),sizeof(short));
				write_DOS(frame->imno,&(descr->nb_values),sizeof(int));
				if (descr->data_type != CHAR) 
				    for (j=0;j<descr->nb_values;j++)
					write_DOS(frame->imno,&(descr->descr_value.c_data[j*codesize]),codesize);
				else
				    write(frame->imno,descr->descr_value.c_data,
					codesize*descr->nb_values);
			    }	
			} else {
			    write(frame->imno,&(dsc_items->nb_descr),sizeof(int));
			    for (i=0; i< dsc_items->nb_descr; i++) {
				descr = dsc_items->descr_list+i;
				codesize = (short)sizeof_item(descr->data_type);
				write(frame->imno,descr->descr_name,lg_label*sizeof(char));
				write(frame->imno,&(descr->data_type),sizeof(short));
				write(frame->imno,&(descr->nb_values),sizeof(int));
				write(frame->imno,descr->descr_value.c_data,
					codesize*descr->nb_values);
			    }	
			}
		    }
		    free_all_desc((Anyfile *)frame);
		    break;

		    case EURO3D_FORMAT :

    		    WR_history(frame, (Anyfile *)0);

					/* saving groups */

		    sprintf(tbl_ext,"%s[%s]",frame->name,E3D_GRP);
	            append_tbl_extension(tbl_ext,OutputIO.basic_io);
		    if (exist(tbl_ext)) 
		    	stat = open_table(&tblgrp,tbl_ext,"IO");
		    else
		    	stat = create_table(&tblgrp,tbl_ext,frame->ngroups,10,'Q',NULL);
	  	    if (stat < 0) {
			stat = ERR_TBL_EXT; 
			break;
		    }
		    colgid = create_col(&tblgrp,E3D_COL_GRP,LONG,'R',"I8","none");
	  	    if (colgid < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colsh = create_col(&tblgrp,E3D_COL_SHAPE,CHAR,'R',"A1","none");
	  	    if (colsh < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    cols1 = create_col(&tblgrp,E3D_COL_SIZE1,FLOAT,'R',"F9.4",frame->s1_unit);
	  	    if (cols1 < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colang = create_col(&tblgrp,E3D_COL_ANGLE,FLOAT,'R',"F9.4",frame->ang_unit);
	  	    if (colang < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    cols2 = create_col(&tblgrp,E3D_COL_SIZE2,FLOAT,'R',"F9.4",frame->s2_unit);
	  	    if (cols2 < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colpwav = create_col(&tblgrp,E3D_COL_PWAVE,FLOAT,'R',"F9.4",frame->pw_unit);
	  	    if (colpwav < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colairm = create_col(&tblgrp,E3D_COL_AIRM,FLOAT,'R',"F9.4","");
	  	    if (colairm < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colpang = create_col(&tblgrp,E3D_COL_PANG,FLOAT,'R',"F9.4",frame->pa_unit);
	  	    if (colpang < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colpres = create_col(&tblgrp,E3D_COL_PRES,FLOAT,'R',"F9.4",frame->pr_unit);
	  	    if (colpres < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    coltemp = create_col(&tblgrp,E3D_COL_TEMP,FLOAT,'R',"F9.4",frame->temp_unit);
	  	    if (coltemp < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    colhum = create_col(&tblgrp,E3D_COL_HUM,FLOAT,'R',"F9.4",frame->hum_unit);
	  	    if (colhum < 0) {
			close_table(&tblgrp);
			stat = ERR_BAD_COL; 
			break;
		    }
		    for (j=0; j< frame->ngroups; j++) {
			WR_tbl(&tblgrp,j,colgid,&(frame->groups[j].groupId));
			WR_tbl(&tblgrp,j,colsh,&(frame->groups[j].shape));
			WR_tbl(&tblgrp,j,cols1,&(frame->groups[j].size1));
			WR_tbl(&tblgrp,j,colang,&(frame->groups[j].angle));
			WR_tbl(&tblgrp,j,cols2,&(frame->groups[j].size2));
			WR_tbl(&tblgrp,j,colpwav,&(frame->groups[j].poswav));
			WR_tbl(&tblgrp,j,colairm,&(frame->groups[j].airmass));
			WR_tbl(&tblgrp,j,colpang,&(frame->groups[j].parang));
			WR_tbl(&tblgrp,j,colpres,&(frame->groups[j].pressure));
			WR_tbl(&tblgrp,j,coltemp,&(frame->groups[j].temperature));
			WR_tbl(&tblgrp,j,colhum,&(frame->groups[j].rel_humidity));
		    }
		    stat = close_table(&tblgrp);
		    break;
		}
	}
  	if (stat) {
		sprintf(errtext,"close_E3D_file: frame %s",frame->name);
		Handle_Error(errtext,stat);
		return(stat);
	} 
	if (frame->signal != NULL) free(frame->signal);
	if (frame->noise != NULL) free(frame->noise);

	if(TK && (frame->iomode == O_MODE || frame->iomode == IO_MODE))
	{
		printf("@ N {%s}\n",frame->name);
	}
	switch(frame->data_format) {

		case TIGER_FORMAT :
	        stat = close(frame->imno);
		break;

		case EURO3D_FORMAT :
                fptr = (fitsfile *)frame->external_info;
		stat = 0;
		fits_close_file(fptr,&stat);
		/* free WCS structure */
		stat = wcs_free(frame);
		break;

                default :
		stat = ERR_FORMAT;
		break;
	}
  	if (stat) {
		sprintf(errtext,"close_E3D_file: frame %s",frame->name);
		Handle_Error(errtext,stat);
	} 
	else 
		frame->imno = -1;

  	return(stat);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_E3D_file()
!
!.purp        closes and deletes a currently active E3D frame 
!.desc
! delete_E3D_file(frame)	
!
! E3D_file *frame;      E3D file structure
!.ed
-------------------------------------------------------------------- */

int delete_E3D_file(E3D_file *frame)		
{
	int status;
	char errtext[132];
	char filename[lg_name+1];
	
	append_datacube_extension(frame->name,InputIO.datacubes);

	strcpy(filename,frame->name);

	if (frame->imno >= 0)
		close_E3D_file(frame);

	status = remove(filename);
	if (status)	{
		sprintf(errtext,"delete_E3D_file: frame %s",frame->name);
		Handle_Error(errtext,status);
	}
	if (TK) 
		printf("@ D {%s}\n",frame->name);
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.blk                   Routines for E3D SPECTRA i/o 
!                                      
!.func                            init_new_E3D_spec()
!
!.purp               returns an initialised spectrum structure items
!.desc
! init_new_E3D_spec(frame,spectrum,npix,start)	
!
! E3D_file *frame;      E3D file structure
! SPECTRUM *spectrum;   spectrum structure
! int npix;             nb of pixels 
! double start;         start coordinate of spectrum 
!.ed
-------------------------------------------------------------------- */
int 
init_new_E3D_spec(E3D_file *frame, SPECTRUM *spectrum, int npix, double start)
{
	char errtext[132];
  	int status = 0;
	
 	sprintf(spectrum->name,"%s->s%.4d",frame->name,-1);
	spectrum->imno = -1;
	spectrum->file_type = -1;
	spectrum->data_format = TIGER_FORMAT;
	spectrum->iomode = frame->iomode;
 	strcpy(spectrum->ident,frame->ident);
 	strcpy(spectrum->cunit,frame->cunit);
 	strcpy(spectrum->history,frame->history);
	spectrum->step = frame->step;
	if (npix < 0) {
		if (frame->common_bounds) {
			get_common_param(frame,&(spectrum->npts), &(spectrum->start),&(spectrum->end));
		}
		else
			status = ERR_BAD_PARAM;
	}
	else {
		spectrum->npts = npix;
		spectrum->start = start;
	}
	if (!status) {
		spectrum->end = ((spectrum->npts-1)*spectrum->step + spectrum->start);
		spectrum->wstart = spectrum->start;      
		spectrum->wend = spectrum->end;
		spectrum->iwstart = 0;      
		spectrum->iwend = spectrum->npts - 1;
		spectrum->data_type = frame->data_type;
		spectrum->external_info = NULL;

		status = alloc_spec_mem(spectrum, frame->data_type);
	}
  	if (status) {
		sprintf(errtext,"init_new_E3D_spec: spec %s (%d pixels)",spectrum->name,npix);
		Handle_Error(errtext,status);
	}
  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                            get_E3D_spec()
!
!.purp               returns the spectrum structure items
!.desc
! get_E3D_spec(frame,signal,noise,specId)	
!
! E3D_file *frame;      E3D file structure
! SPECTRUM *signal;     signal spectrum structure (or NULL)
! SPECTRUM *noise;      noise spectrum structure (or NULL)
! int specId;           spectrum ID
!.ed
-------------------------------------------------------------------- */

int extract_E3D_spec(E3D_file *frame, SPECTRUM *spectrum, E3Dspec_desc **type_of_spec, int specId) {

	char errtext[132];
	E3Dspec_desc *current_spec;
	int status, i, colnum;
	fitsfile *fptr;

	sprintf(spectrum->name,"%s->s%.4d",frame->name,specId); 
	spectrum->imno = -1; 
	spectrum->file_type = -1; 
	spectrum->data_format = frame->data_format;
	spectrum->iomode = frame->iomode; 
 	strcpy(spectrum->ident,frame->ident);
 	strcpy(spectrum->cunit,frame->cunit);
 	strcpy(spectrum->history,frame->history);
	spectrum->step = frame->step; 
	spectrum->data_type = frame->data_type;
	spectrum->external_info = NULL; 

	if (*type_of_spec == frame->signal)  {
		find_lens(frame,signal,specId,&i); 
	}	
	else {
		find_lens(frame,noise,specId,&i); 
	}

	if (i < 0 ) 
		status = ERR_NOIMA;
	else { 
		current_spec = *(type_of_spec); 
		current_spec += i;
		spectrum->start = current_spec->start; 
		spectrum->end = current_spec->end; 
		spectrum->npts = current_spec->npix;
		spectrum->wstart = spectrum->start; 
		spectrum->wend = spectrum->end; 
		spectrum->iwstart = 0;
		spectrum->iwend = spectrum->npts - 1; 
		status = alloc_spec_mem(spectrum, frame->data_type); 

		switch (spectrum->data_format) {
		case TIGER_FORMAT :
			lseek(frame->imno,current_spec->data_offset,SEEK_SET);
			if (frame->swapbytes) {
		  	for (i=0;i<spectrum->npts;i++)
		     		read_DOS(frame->imno,(char *)(spectrum->data.s_data)+
				i*sizeof_item(spectrum->data_type), 
		  		sizeof_item(spectrum->data_type)); 
			} 
			else {
		  		read(frame->imno,(char *)(spectrum->data.s_data), 
		  		spectrum->npts*sizeof_item(spectrum->data_type)); 
			} 
			break;

		case EURO3D_FORMAT :
	        	status = 0;
			i = current_spec->data_offset;
			fptr = (fitsfile *)frame->external_info;

			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	
			if (status) { 
				status = 0;
				fits_movabs_hdu(fptr, 1, NULL, &status);
				fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	                }
	
			/* read signal or noise */
			if (*type_of_spec == frame->signal) 
				fits_get_colnum(fptr,0,E3D_COL_INT,&colnum,&status);
			else 
				fits_get_colnum(fptr,0,E3D_COL_RMS,&colnum,&status);
				
			if (status) {
				status = ERR_BAD_COL; break;
			}
			switch(spectrum->data_type) {
			case SHORT :
				fits_read_col_sht(fptr, colnum, i+1, 1, spectrum->npts, 0, spectrum->data.s_data, NULL, &status);
				break;
			case INT :
			case LONG :
				fits_read_col_lng(fptr, colnum, i+1, 1, spectrum->npts, 0, spectrum->data.l_data, NULL, &status);
				break;
			case FLOAT :
				fits_read_col_flt(fptr, colnum, i+1, 1, spectrum->npts, 0, spectrum->data.f_data, NULL, &status);
				break;
			case DOUBLE :
				fits_read_col_dbl(fptr, colnum, i+1, 1, spectrum->npts, 0, spectrum->data.d_data, NULL, &status);
				break;
			}
			if (status) {
				status = ERR_READ; break;
			}
	
			/* read data quality */
			fits_get_colnum(fptr,0,E3D_COL_DQ,&colnum,&status);
			if (status) {
				status = ERR_BAD_COL; break;
			}
			fits_read_col_lng(fptr, colnum, i+1, 1, spectrum->npts, 0, spectrum->quality, NULL, &status);
			if (status) {
				status = ERR_WRIT; break;
			}
	
			break;

                	default :
			status = ERR_FORMAT;
			break;
		}
	} 
  	if (status) {
		sprintf(errtext,"get_E3D_spec: spec %s",spectrum->name);
		Handle_Error(errtext,status); 
	} 
	return(status);
}

int 
get_E3D_spec(E3D_file *frame, SPECTRUM *signal, SPECTRUM *noise, int specId)
{

	int status;

	if ((signal != NULL) && (frame->signal != NULL)) {
		status = extract_E3D_spec(frame,signal,&(frame->signal),specId);
		if (status) return(status);
	} 
	if ((noise != NULL) && (frame->noise != NULL)) {
		status = extract_E3D_spec(frame,noise,&(frame->noise),specId);
	} 

  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        put_E3D_spec()
!
!.purp               update/add a spectrum in a E3D frame 
!             if the spectrum already exists, it is overwritten.
!             The data array of the input spectrum is freed by the routine.
!.desc
! put_E3D_spec(frame,signal,noise,specId)
!
! E3D_file *frame;      E3D file structure
! SPECTRUM *signal;     signal spectrum structure (or NULL)
! SPECTRUM *noise;      noise spectrum structure (or NULL)
! int specId;           spectrum ID
!.ed
-------------------------------------------------------------------- */

int save_E3D_spec(E3D_file *frame, SPECTRUM *spectrum, E3Dspec_desc **type_of_spec, int specId) {

	char errtext[132];
	int status=0, i, len, idx;
	E3Dspec_desc *current_spec;
	fitsfile *fptr;
	int colnum, tformat;
	char format[132], colname[17];
	char *pt_data;

 	sprintf(spectrum->name,"%s->s%.4d",frame->name,specId); 
	set_subspec(spectrum,spectrum->start,spectrum->end);
	spec_minmax(spectrum); 
 
	if (*type_of_spec == NULL) {	/* create first spectrum */ 
		*type_of_spec = (E3Dspec_desc *)malloc(sizeof(E3Dspec_desc));
		(*type_of_spec)[0].specId = LENS_UNDEF; 
		i = 0; 
	} 
	else {  /* find which spectrum to rewrite */
 
		for (i=0; i<frame->nbspec
			&& (*type_of_spec)[i].specId != specId 
			&& (*type_of_spec)[i].specId != LENS_UNDEF; i++); 
	} 
	if ((i == frame->nbspec) || ((*type_of_spec)[i].specId == LENS_UNDEF)) { 
		*type_of_spec = (E3Dspec_desc *)realloc(
				(char *)*type_of_spec, 
				(i+2)*sizeof(E3Dspec_desc)); 
		current_spec = (*type_of_spec+i); 
		current_spec->specId = specId; 
		current_spec->data_offset = frame->data_offset;
		if (i == frame->nbspec) /* create new spectrum */ {
			frame->nbspec++; 
		}
		else 
			(current_spec+1)->specId = LENS_UNDEF;
	} 
	else
		current_spec = *type_of_spec+i; 

	current_spec->start = spectrum->start; 
	current_spec->end = spectrum->end; 
	current_spec->npix = spectrum->npts; 
	 
	switch (frame->data_format) {

	case TIGER_FORMAT :
		lseek(frame->imno,current_spec->data_offset,SEEK_SET); 
		len = spectrum->npts*sizeof_item(spectrum->data_type); 
		if (frame->swapbytes) { 
	  	for (i=0;i<spectrum->npts;i++)
	     		write_DOS(frame->imno,(char *)(spectrum->data.s_data)+
				i*sizeof_item(spectrum->data_type), 
		  		sizeof_item(spectrum->data_type)); 
		} 
		else 
			write(frame->imno,(char *)(spectrum->data.s_data),len); 
		break;

	case EURO3D_FORMAT :
	        status = 0;
		current_spec->data_offset = i;  /* to handle selection */

		fptr = (fitsfile *)frame->external_info;

		fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);

		if (status) { 
			status = 0;
			fits_movabs_hdu(fptr, 1, NULL, &status);
			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
                }

		/* write npix */
		fits_get_colnum(fptr,0,E3D_COL_NPIX,&colnum,&status);
		if (status) {
			status = 0;
			sprintf(format,"J");
			fits_get_num_cols(fptr, &colnum, &status);
			colnum++;
			fits_insert_col(fptr,colnum,E3D_COL_NPIX,format,&status);
		}
		fits_write_col(fptr, TLONG, colnum, i+1, 1, 1, &(spectrum->npts), &status);

		/* write starting pixel */
		fits_get_colnum(fptr,0,E3D_COL_IDX,&colnum,&status);
		if (status) {
			status = 0;
			sprintf(format,"J");
			fits_get_num_cols(fptr, &colnum, &status);
			colnum++;
			fits_insert_col(fptr,colnum,E3D_COL_IDX,format,&status);
		}
		idx = (int)((spectrum->start - (*type_of_spec)[0].start) / spectrum->step);
		fits_write_col(fptr, TLONG, colnum, i+1, 1, 1, &idx, &status);

		/* write signal or noise */
		if (*type_of_spec == frame->signal) 
			strcpy(colname,E3D_COL_INT);
		else 
			strcpy(colname,E3D_COL_RMS);
		switch(spectrum->data_type) {
			case SHORT :
				if (frame->common_bounds)
					sprintf(format,"%dI",(int)frame->common_parameters[2]);
				else
					sprintf(format,"1PI");
				tformat = TSHORT;
				pt_data = (char *)spectrum->data.s_data;
				break;
			case INT :
			case LONG :
				if (frame->common_bounds)
					sprintf(format,"%dJ",(int)frame->common_parameters[2]);
				else
					sprintf(format,"1PJ");
				tformat = TLONG;
				pt_data = (char *)spectrum->data.l_data;
				break;
			case FLOAT :
				if (frame->common_bounds)
					sprintf(format,"%dE",(int)frame->common_parameters[2]);
				else
					sprintf(format,"1PE");
				tformat = TFLOAT;
				pt_data = (char *)spectrum->data.f_data;
				break;
			case DOUBLE :
				if (frame->common_bounds)
					sprintf(format,"%dD",(int)frame->common_parameters[2]);
				else
					sprintf(format,"1PD");
				tformat = TDOUBLE;
				pt_data = (char *)spectrum->data.d_data;
				break;
		}
		fits_get_colnum(fptr,0,colname,&colnum,&status);
		if (status) {
			status = 0;
			fits_get_num_cols(fptr, &colnum, &status);
			colnum++;
			fits_insert_col(fptr,colnum,colname,format,&status);
		}
		fits_write_col(fptr, tformat, colnum, i+1, 1, spectrum->npts, pt_data, &status);
		if (status) {
			status = ERR_WRIT; break;
		}

		/* write data quality */
		fits_get_colnum(fptr,0,E3D_COL_DQ,&colnum,&status);
		if (status) {
			status = 0;
			if (frame->common_bounds)
				sprintf(format,"%dJ",(int)frame->common_parameters[2]);
			else
				sprintf(format,"1PJ");
			fits_get_num_cols(fptr, &colnum, &status);
			colnum++;
			fits_insert_col(fptr,colnum,E3D_COL_DQ,format,&status);
		}
		fits_write_col(fptr, TLONG, colnum, i+1, 1, spectrum->npts, spectrum->quality, &status);
		if (status) {
			status = ERR_WRIT; break;
		}

		if (i == 0) { /* first spectrum, create associated keywords */
			fits_write_key_str(fptr,E3D_KW_UNITS,frame->cunit,"wavelength unit", &status);
			fits_write_key_dbl(fptr,E3D_KW_START,spectrum->start,10,"reference wavelength", &status);
			fits_write_key_dbl(fptr,E3D_KW_STEP,spectrum->step,10,"wavelength increment per pixel", &status);
			fits_write_key_lng(fptr,E3D_KW_REFPIX,(long)1,"reference pixel", &status);
		}
		break;

               	default :
		status = ERR_FORMAT;
		break;
	}
	free_spec_mem(spectrum); 
 
	if (status) { 
		sprintf(errtext,"put_E3D_spec: spec %s",spectrum->name); 
		Handle_Error(errtext,status);
	} 
	else
		if (frame->data_offset == current_spec->data_offset) {
		   frame->data_offset += len; 
		}
	return(status);
}

int 
put_E3D_spec(E3D_file *frame, SPECTRUM *signal, SPECTRUM *noise, int specId)
{
	int status;
	char errtext[132];

	if (frame->iomode == (int)I_MODE) {
		sprintf(errtext,"put_E3D_spec: file %s is write protected",
			frame->name);
		Handle_Error(errtext,ERR_ACCESS); 
		return(ERR_ACCESS); 
	} 

	if (signal != NULL) {
		status = save_E3D_spec(frame,signal,&(frame->signal),specId);
		if (status) return(status);
	} 
	if (noise != NULL) {
		status = save_E3D_spec(frame,noise,&(frame->noise),specId);
	} 

  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      delete_E3D_spec()
!
!.purp             delete a spectrum in a E3D frame
!.desc
! delete_E3D_spec(frame,specId)	
!
! E3D_file *frame;      E3D file structure
! int specId;           spectrum ID
!.ed
-------------------------------------------------------------------- */
int 
delete_E3D_spec(E3D_file *frame, int specId)
{
	int status, i = -1, j = -1;
	char errtext[132];
	E3Dspec_desc *current_spec;
	fitsfile *fptr;
	

	status = ERR_NOIMA;
	if (frame->signal != NULL) {
	    find_lens(frame,signal,specId,&i);
	    if (i >= 0) {
		current_spec = frame->signal+i;
		if (i < (frame->nbspec -1))
			memcpy((char *)current_spec,(char *)(frame->signal+i+1),
				(frame->nbspec-i-1)*sizeof(E3Dspec_desc));
		}
	}
	if (frame->noise != NULL) {
	    find_lens(frame,noise,specId,&j);
	    if (j >= 0) {
		current_spec = frame->noise+j;
		if (j < (frame->nbspec -1))
		memcpy((char *)current_spec,(char *)(frame->noise+j+1),
			(frame->nbspec-j-1)*sizeof(E3Dspec_desc));
	    }
	}
	if (i>= 0 || j>= 0)
		frame->nbspec--;

	switch (frame->data_format) {

		case EURO3D_FORMAT :
	        status = 0;
		fptr = (fitsfile *)frame->external_info;

		fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);

		if (status) { 
			status = 0;
			fits_movabs_hdu(fptr, 1, NULL, &status);
			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
                }

		fits_delete_rows(fptr,(long)(i+1),1L,&status);
		status = get_tiger_errcode(frame->data_format,status);
		break;
	}
	if (status)	{
		sprintf(errtext,"delete_E3D_spec: spec %d in %s",specId,frame->name);
		Handle_Error(errtext,status);
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        get_E3D_spaxels()
!
!.purp   retrieve spaxels relevant to given ID (returns the number of spaxels)
!.desc
! int get_E3D_spaxels(frame, specId, spaxels)
!
! E3D_file *frame;      datacube structure
! int specId;           spectrum ID
! SPAXEL **spaxels;     pointer to array of spaxel description
!.ed
-------------------------------------------------------------------- */

int get_E3D_spaxels(E3D_file *frame, int specId, SPAXEL **spaxels)
{
	char errtext[132];
	int status, i, colnum, nspaxels;
	long *groups;
	char *spaxid, nullval[2]= "";
	float *xpos, *ypos, x, y;
	fitsfile *fptr;
	SPAXEL *cur_spax;

    	if (frame->signal != NULL) {
		find_lens(frame,signal,specId,&i); 
    	}
    	else {
		find_lens(frame,noise,specId,&i); 
   	}

	if (i < 0 ) 
		status = ERR_NOIMA;
	else { 
		switch (frame->data_format) {
		case TIGER_FORMAT :
	        	status = 0;
			nspaxels = 1;
			cur_spax = (SPAXEL *)calloc(1,sizeof(SPAXEL));
			*spaxels = cur_spax;
			cur_spax[0].specId = specId;
			cur_spax[0].spaxelId = 0;
			cur_spax[0].group = 1;
			get_lens_coordinates(frame, specId, &x, &y);
			cur_spax[0].xpos = x;
			cur_spax[0].ypos = y;
			break;

		case EURO3D_FORMAT :
	        	status = 0;
			fptr = (fitsfile *)frame->external_info;

			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	
			if (status) { 
				status = 0;
				fits_movabs_hdu(fptr, 1, NULL, &status);
				fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	                }
			fits_get_colnum(fptr,0,E3D_COL_NSPAX,&colnum,&status);
			if (status) {
				status = ERR_BAD_COL; break;
			}
			fits_read_col_lng(fptr, colnum, i+1, 1, 1, 0, (long *)&nspaxels, NULL, &status);
			if (status) {
				status = ERR_READ; break;
			}
			xpos = (float *)calloc(nspaxels,sizeof(float));
			fits_get_colnum(fptr,0,E3D_COL_XPOS,&colnum,&status);
			if (status) {
				status = ERR_BAD_COL; break;
			}
			fits_read_col_flt(fptr, colnum, i+1, 1, (long)nspaxels, 0, xpos, NULL, &status);
			if (status) {
				status = ERR_READ; break;
			}
			ypos = (float *)calloc(nspaxels,sizeof(float));
			fits_get_colnum(fptr,0,E3D_COL_YPOS,&colnum,&status);
			if (status) {
				status = ERR_BAD_COL; break;
			}
			fits_read_col_flt(fptr, colnum, i+1, 1, (long)nspaxels, 0, ypos, NULL, &status);
			if (status) {
				status = ERR_READ; break;
			}
			groups = (long *)calloc(nspaxels,sizeof(long));
			fits_get_colnum(fptr,0,E3D_COL_GRP,&colnum,&status);
			if (status) {
				status = ERR_BAD_COL; break;
			}
			fits_read_col_lng(fptr, colnum, i+1, 1, (long)nspaxels, 0, (long *)groups, NULL, &status);
			if (status) {
				status = ERR_READ; break;
			}
			spaxid = calloc(nspaxels+1,sizeof(char));
			fits_get_colnum(fptr,0,E3D_COL_SPAXID,&colnum,&status);
			if (status) {
				status = ERR_BAD_COL; break;
			}
			fits_read_col_str(fptr, colnum, i+1, 1, (long)nspaxels, nullval, &spaxid, NULL, &status);
			if (status) {
				status = ERR_READ; break;
			}
			cur_spax = (SPAXEL *)calloc(nspaxels,sizeof(SPAXEL));
			*spaxels = cur_spax;
			for (i=0; i<nspaxels; i++) {
				cur_spax[i].specId = specId;
				cur_spax[i].spaxelId = spaxid[i];
				cur_spax[i].group = groups[i];
				cur_spax[i].xpos = xpos[i];
				cur_spax[i].ypos = ypos[i];
			}
			free(xpos);free(ypos);free(groups);free(spaxid);
			break;

               	default :
			status = ERR_FORMAT;
			break;
		}
	} 
  	if (status) {
		sprintf(errtext,"get_E3D_spaxels: spec %d",specId);
		Handle_Error(errtext,status); 
	} 
	else
		status = nspaxels;
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        put_E3D_spaxels()
!
!.purp               puts the spaxels relevant to given ID
!.desc
! int put_E3D_spaxels(frame, specId, spaxels)
!
! E3D_file *frame;      datacube structure
! int specId;           spectrum ID
! int nbspax;           number of spaxels
! SPAXEL *spaxels;      array of spaxel structures
!.ed
-------------------------------------------------------------------- */

int put_E3D_spaxels(E3D_file *frame, int specId, int nspax, SPAXEL *spaxels)
{
	char errtext[132];
	int status, i, j, colnum;
	long nspaxels = nspax;
	long id = specId;
	long *groups;
	char *spaxid, **pt_spaxid;
	float *xpos, *ypos;
	fitsfile *fptr;
	SPAXEL *cur_spax;

	if (frame->signal != NULL) {
		find_lens(frame,signal,specId,&i); 
	}
	else {
		find_lens(frame,noise,specId,&i); 
	}

	if (i < 0 ) 
		status = ERR_NOIMA;
	else { 
		switch (frame->data_format) {
		case TIGER_FORMAT :
			status = 0;
			xpos = &(spaxels[0].xpos);
			ypos = &(spaxels[0].ypos);
			set_lens_coordinates(frame, specId, xpos, ypos);
			break;

		case EURO3D_FORMAT :
	        	status = 0;
			cur_spax = spaxels;
			xpos = (float *)calloc(nspaxels,sizeof(float));
			ypos = (float *)calloc(nspaxels,sizeof(float));
			groups = (long *)calloc(nspaxels,sizeof(long));
			spaxid = (char *)calloc(nspaxels+1,sizeof(char));
			pt_spaxid = (char **)malloc(1*sizeof(char *));
			pt_spaxid[0] = spaxid;
			for (j=0; j<nspaxels; j++) {
				groups[j] = cur_spax[j].group;
				xpos[j] = cur_spax[j].xpos;
				ypos[j] = cur_spax[j].ypos;
				spaxid[j] = cur_spax[j].spaxelId;
			}
			spaxid[nspaxels] = '\0';
			fptr = (fitsfile *)frame->external_info;

			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	
			if (status) { 
				status = 0;
				fits_movabs_hdu(fptr, 1, NULL, &status);
				fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	                }
			fits_get_colnum(fptr,0,E3D_COL_ID,&colnum,&status);
			fits_write_col(fptr, TLONG, colnum, i+1, 1, 1, &id, &status);
			fits_get_colnum(fptr,0,E3D_COL_NSPAX,&colnum,&status);
			fits_write_col(fptr, TLONG, colnum, i+1, 1, 1, &nspaxels, &status);
			fits_get_colnum(fptr,0,E3D_COL_XPOS,&colnum,&status);
			fits_write_col(fptr, TFLOAT, colnum, i+1, 1, nspaxels, xpos, &status);
			fits_get_colnum(fptr,0,E3D_COL_YPOS,&colnum,&status);
			fits_write_col(fptr, TFLOAT, colnum, i+1, 1, nspaxels, ypos, &status);
			fits_get_colnum(fptr,0,E3D_COL_GRP,&colnum,&status);
			fits_write_col(fptr, TLONG, colnum, i+1, 1, nspaxels, groups, &status);
			fits_get_colnum(fptr,0,E3D_COL_SPAXID,&colnum,&status);
			fits_write_col_str(fptr, colnum, i+1, 1, nspaxels, pt_spaxid, &status);
			free(xpos);free(ypos);free(groups); free(spaxid); free(pt_spaxid);

			if (status) {
				status = ERR_WRIT; break;
			}
			break;

               	default :
			status = ERR_FORMAT;
			break;
		}
	} 
  	if (status) {
		sprintf(errtext,"put_E3D_spaxels: spec %d",specId);
		Handle_Error(errtext,status); 
	} 
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        exist_group_ID()
!
!.purp            check if group exists (returns -1 if not)
!.desc
! exist_group_ID(frame, groupID)
!
! E3D_file *frame;      datacube structure
! int groupID;          group ID
!.ed
-------------------------------------------------------------------- */

int exist_group_ID(E3D_file *frame, char groupID)
{
	int j, found = 0;

	/* check if group is already defined */

	for (j=0; j<frame->ngroups && !found; j++) {
		if (frame->groups[j].groupId == groupID) {
				/* group already exist */
				found = 1;
		}
	}
	if (found > 0)
		return(j);
	else
		return(-1);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        get_E3D_groups()
!
!.purp               retrieve groups relevant to given ID
!                    return the number of groups read, or status (<0) on error
!.desc
! get_E3D_groups(frame, specId, groups)
!
! E3D_file *frame;      datacube structure
! int specId;           spectrum ID
! GROUP **groups;       pointer to array of group description
!.ed
-------------------------------------------------------------------- */

int get_E3D_groups(E3D_file *frame, int specId, GROUP **groups)
{
	char errtext[132];
	int status, i, j, colnum, ngroups;
	char *groups_id;
	char nullval[2]= "";
	fitsfile *fptr;

	if (frame->signal != NULL) {
		find_lens(frame,signal,specId,&i); 
	}
	else {
		find_lens(frame,noise,specId,&i); 
	}

	if (i < 0 ) 
		status = ERR_NOIMA;
	else { 
		switch (frame->data_format) {
		case TIGER_FORMAT :
			ngroups = 1;
			*groups = (GROUP *)calloc(1,sizeof(GROUP));

			if (frame->groups == NULL) 
				set_tiger_group(frame);

			memcpy((*groups),frame->groups,sizeof(GROUP));			
			break;

		case EURO3D_FORMAT :
	        	status = 0;
			/* ngroups = (long)frame->common_parameters[2]; */
			ngroups = frame->ngroups;
			fptr = (fitsfile *)frame->external_info;

			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	
			if (status) { 
				status = 0;
				fits_movabs_hdu(fptr, 1, NULL, &status);
				fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	                }
			fits_get_colnum(fptr,0,E3D_COL_GRP,&colnum,&status);
			groups_id = (char *)calloc(ngroups+1,sizeof(char));
			fits_read_col_str(fptr, colnum, i+1, 1, (long)ngroups, 
				nullval, &groups_id, NULL, &status);
			if (status) {
				status = ERR_READ; break;
			}
			*groups = (GROUP *)calloc(ngroups,sizeof(GROUP));
			for (i=0; i<ngroups; i++) {
			     for(j=0; j<frame->ngroups; j++)
				if (frame->groups[j].groupId == groups_id[i]) {
					memcpy((*groups)+i,frame->groups+j,sizeof(GROUP));			
				}
			}
			free(groups_id);
			break;

               	default :
			status = ERR_FORMAT;
			break;
		}
	} 
  	if (status) {
		sprintf(errtext,"get_E3D_groups: spec %d",specId);
		Handle_Error(errtext,status); 
	} 
	else
		return ngroups;
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        put_E3D_groups()
!
!.purp                   add groups to a E3D frame
!.desc
! put_E3D_groups(frame, ngroups, groups)
!
! E3D_file *frame;      datacube structure
! int ngroups;          number of groups
! GROUP *groups;        description of groups
!.ed
-------------------------------------------------------------------- */

int put_E3D_groups(E3D_file *frame, int ngroups, GROUP *groups)
{
	char errtext[132];
	int status = 0;
	int i, j, found = 0;

		/* add new group(s) in frame structure */

	for (i=0; i<ngroups && !found; i++) {

		found = 0;

		/* check if group not already defined */

		for (j=0; j<frame->ngroups && !found; j++) {
			if (frame->groups[j].groupId == groups[i].groupId) {
								/* groups already exist */
				found = 1;
			}
		}
		if (!found) {
			frame->groups =(GROUP *)realloc(frame->groups,(frame->ngroups+1)*sizeof(GROUP));
			memcpy((char *)(frame->groups+frame->ngroups),(char *)(groups+i),sizeof(GROUP));
			frame->ngroups++;
		}
		else {
			memcpy((char *)(frame->groups+j-1),(char *)(groups+i),sizeof(GROUP));
		}
        }

  	if (status) {
		sprintf(errtext,"put_E3D_group: file %s",frame->name);
		Handle_Error(errtext,status); 
	} 
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        delete_E3D_group()
!
!.purp                   delete a group into a E3D frame
!.desc
! delete_E3D_group(frame, groupID)
!
! E3D_file *frame;      datacube structure
! int groupID;          number of groups
!.ed
-------------------------------------------------------------------- */

int delete_E3D_group(E3D_file *frame, int groupID)
{
	char errtext[132];
	int status = 0;
	int j, found = 0;

	/* check if group is defined */

	for (j=0; j<frame->ngroups && !found; j++) {
		if (frame->groups[j].groupId == groupID) {
							/* group exist */
			found = 1;
		}
	}
	if (found) {	/* delete group */
		for (; j<frame->ngroups -1; j++) {
			memcpy(frame->groups+j,frame->groups+j+1,sizeof(GROUP));
		}
		frame->ngroups--;
	}
	else
		status = ERR_BAD_PARAM;

  	if (status) {
		sprintf(errtext,"delete_E3D_group: file %s, group %d",frame->name, groupID);
		Handle_Error(errtext,status); 
	} 
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                            get_E3D_row()
!
!.purp               returns all informations belonging to the spectrum
!.desc
! get_E3D_row(frame,specId,signal,noise,spaxels,nspax,groups,ngrp)	
!
! E3D_file *frame;      E3D file structure
! int specId;           number of associated lens
! SPECTRUM *signal;     spectrum structure (or NULL)
! SPECTRUM *noise;      spectrum structure (or NULL)
! int *nspax;           on exit, number of spaxels
! SPAXEL **spaxels;     pointer to array of spaxel description
! int *ngrp;            on exit, number of groups
! GROUP **groups;       pointer to array of group description
!.ed
-------------------------------------------------------------------- */

int 
get_E3D_row(E3D_file *frame, int specId, SPECTRUM *signal, SPECTRUM *noise, int *nspax , SPAXEL **spaxels, int *ngroups, GROUP **groups)
{
	int status;

	status = get_E3D_spec(frame,signal,noise,specId);
	if (!status) {
		*nspax = get_E3D_spaxels(frame,specId,spaxels);
		*ngroups = get_E3D_groups(frame,specId,groups);
	}

  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                            put_E3D_row()
!
!.purp                  sets all informations belonging to the spectrum
!.desc
! put_E3D_row(frame,specId,signal,noise,spaxels,nspax,groups,ngrp)	
!
! E3D_file *frame;      E3D file structure
! int specId;           number of associated lens
! SPECTRUM *signal;     spectrum structure (or NULL)
! SPECTRUM *noise;      spectrum structure (or NULL)
! int nspax;            number of spaxels
! SPAXEL *spaxels;      array of spaxel description
! int ngrp;             number of groups
! GROUP *groups;        pointer to array of group description
!.ed
-------------------------------------------------------------------- */

int 
put_E3D_row(E3D_file *frame, int specId, SPECTRUM *signal, SPECTRUM *noise, int nspax, SPAXEL *spaxels, int ngroups, GROUP *groups)
{
	int status;

	status = put_E3D_spec(frame,signal,noise,specId);
	if (!status) {
		status = put_E3D_spaxels(frame,specId,nspax,spaxels);
		status = put_E3D_groups(frame,ngroups,groups);
	}
  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                            get_E3D_frame()
!
!.purp                 retrieve the whole set of spectra as a 2D image
!.desc
! get_E3D_frame(frame,signal,noise)
!
! E3D_file *frame;      E3D file structure
! IMAGE2D *signal;      2D image structure (or NULL)
! IMAGE2D *noise;       2D image structure (or NULL)
!.ed
-------------------------------------------------------------------- */

int 
get_E3D_frame(E3D_file *E3Dframe, IMAGE2D *signal, IMAGE2D *noise)
{
	int status = 0;
	int npix, i, j;
	double start, step, end;
	char errtext[132];
	SPECTRUM spec;
	
	if (!has_common_bounds(E3Dframe)) {
		status = ERR_NOIMPL;
		sprintf(errtext,"get_E3D_frame: not yet implemented for datacube without common bounds");
		Handle_Error(errtext,status);
		return(status);
	}

	switch (E3Dframe->data_format) {

		case TIGER_FORMAT :
			status = ERR_NOIMPL;
			break;

		case EURO3D_FORMAT :

			if ((signal != NULL) && (E3Dframe->signal != NULL)) {
				memset(signal->ident,' ',lg_ident);
				signal->ident[lg_ident] = '\0';
				memset(signal->cunit,' ',lg_unit);
				signal->cunit[lg_unit] = '\0';
				memset(signal->history,' ',lg_hist);
				signal->history[lg_hist] = '\0';
				signal->external_info = 0;
				signal->file_type = T_IMA1D;
				signal->data_format = InputIO.basic_io;
				signal->data_type = E3Dframe->data_type;
				sprintf(signal->name,"%s[%s]",E3Dframe->name,E3D_DATA);

				get_common_param(E3Dframe, &npix, &start, &end);

				signal->nx = npix;
				signal->startx = start;
				signal->stepx = E3Dframe->step;
				signal->endx = end;

				signal->ny = E3Dframe->nbspec;
				signal->stepy = 1.0;
				signal->starty = 1.0;
				signal->endy = E3Dframe->nbspec;

				if (alloc_frame_mem(signal, signal->data_type) < 0) {
	                		status = ERR_ALLOC;
	                		break;
				}
				for (j=0; j<signal->ny; j++) {
					get_E3D_spec(E3Dframe,&spec,NULL,E3Dframe->signal[j].specId);
					for (i=0; i<spec.npts; i++) {
						WR_frame(signal,i,j,RD_spec(&spec,i));
						WR_qframe(signal,i,j,RD_qspec(&spec,i));
					}
					free_spec_mem(&spec);
				}
				image_minmax(signal);
			}
			if ((noise != NULL) && (E3Dframe->noise != NULL)) {
				memset(noise->ident,' ',lg_ident);
				noise->ident[lg_ident] = '\0';
				memset(noise->cunit,' ',lg_unit);
				noise->cunit[lg_unit] = '\0';
				memset(noise->history,' ',lg_hist);
				noise->history[lg_hist] = '\0';
				noise->external_info = 0;
				noise->file_type = T_IMA1D;
				noise->data_format = InputIO.basic_io;
				noise->data_type = E3Dframe->data_type;
				sprintf(noise->name,"%s[%s]",E3Dframe->name,E3D_DATA);

				get_common_param(E3Dframe, &npix, &start, &end);

				noise->nx = npix;
				noise->startx = start;
				noise->stepx = E3Dframe->step;
				noise->endx = end;

				noise->ny = E3Dframe->nbspec;
				noise->starty = 1.0;
				noise->stepy = 1.0;
				noise->endy = E3Dframe->nbspec;

				if (alloc_frame_mem(noise, E3Dframe->data_type) < 0) {
	                		status = ERR_ALLOC;
	                		break;
				}
				for (j=0; j<noise->ny; j++) {
					get_E3D_spec(E3Dframe,NULL,&spec,E3Dframe->noise[j].specId);
					for (i=0; i<spec.npts; i++) {
						WR_frame(noise,i,j,RD_spec(&spec,i));
						WR_qframe(noise,i,j,RD_qspec(&spec,i));
					}
					free_spec_mem(&spec);
				}
				image_minmax(noise);
			}
			break;
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                            put_E3D_frame()
!
!.purp                 save the whole set of spectra as a 2D image
!.desc
! put_E3D_frame(frame,signal,noise)
!
! E3D_file *frame;      E3D file structure
! IMAGE2D *signal;      2D image structure (or NULL)
! IMAGE2D *noise;       2D image structure (or NULL)
!.ed
-------------------------------------------------------------------- */

int 
put_E3D_frame(E3D_file *E3Dframe, IMAGE2D *signal, IMAGE2D *noise)
{
	int status = 0;
	int npix, i, j;
	double start, step, end;
	char errtext[132];
	SPECTRUM spec;
	
	if (!has_common_bounds(E3Dframe)) {
		status = ERR_NOIMPL;
		sprintf(errtext,"put_E3D_frame: not yet implemented for datacube without common bounds");
		Handle_Error(errtext,status);
		return(status);
	}

	switch (E3Dframe->data_format) {

		case TIGER_FORMAT :
			status = ERR_NOIMPL;
			break;

		case EURO3D_FORMAT :

			if ((signal != NULL) && (E3Dframe->signal != NULL)) {

				get_common_param(E3Dframe, &npix, &start, &end);
				if (signal->nx != npix)
					return(ERR_NOIMPL);
				if (signal->ny != E3Dframe->nbspec)
					return(ERR_NOIMPL);
				if (signal->startx != start)
					return(ERR_NOIMPL);
				if (signal->stepx != E3Dframe->step)
					return(ERR_NOIMPL);

				for (j=0; j<signal->ny; j++) {
					init_new_E3D_spec(E3Dframe,&spec,npix,start);
					for (i=0; i<spec.npts; i++) {
						WR_spec(&spec,i,RD_frame(signal,i,j));
						WR_qspec(&spec,i,RD_qframe(signal,i,j));
					}
					put_E3D_spec(E3Dframe,&spec,NULL,E3Dframe->signal[j].specId);
				}
			}
			if ((noise != NULL) && (E3Dframe->noise != NULL)) {

				get_common_param(E3Dframe, &npix, &start, &end);
				if (noise->nx != npix)
					return(ERR_NOIMPL);
				if (noise->ny != E3Dframe->nbspec)
					return(ERR_NOIMPL);
				if (noise->startx != start)
					return(ERR_NOIMPL);
				if (noise->stepx != E3Dframe->step)
					return(ERR_NOIMPL);

				for (j=0; j<noise->ny; j++) {
					init_new_E3D_spec(E3Dframe,&spec,npix,start);
					for (i=0; i<spec.npts; i++) {
						WR_spec(&spec,i,RD_frame(noise,i,j));
						WR_qspec(&spec,i,RD_qframe(noise,i,j));
					}
					put_E3D_spec(E3Dframe,NULL,&spec,E3Dframe->noise[j].specId);
				}
			}
			break;
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk                 Routines to check/retrieve E3D frame features
!
!.func                        has_common_bounds()
!
!.purp   returns true if all spectra have common bounds, false if not
!.desc
! has_common_bounds(frame)	
!
! E3D_file *frame;      E3D file structure
!.ed
-------------------------------------------------------------------- */
int 
has_common_bounds(E3D_file *frame)		
{
	if (frame->common_bounds)
		return(1);
	else
		return(0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        get_common_param()
!
!.purp   gets spectra parameters if there are valid for the whole set
!.desc
! get_common_param(frame,npix,start,end)	
!
! E3D_file *frame;      E3D file structure
! int *npix;            number of pixels
! double *start;        start coordinate
! double *end;          end coordinate
!.ed
-------------------------------------------------------------------- */
int 
get_common_param(E3D_file *frame, int *npix, double *start, double *end)		
{
	int status;

	status = ERR_NODATA;

	if (frame->common_bounds) {
		*npix = (int)(frame->common_parameters[2]);
		*start = frame->common_parameters[0];
		*end = frame->common_parameters[1];
		status = 0;
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        set_common_bounds()
!
!.purp   	set common bounds flag if common bounds
!.desc
! set_common_bounds(frame)	
!
! E3D_file *frame;      E3D file structure
!.ed
-------------------------------------------------------------------- */
int 
set_common_bounds(E3D_file *frame)		
{
	E3Dspec_desc *current_spec;
	double start;
	int    i, npix;

       /* checks for common bounds for all spectra */

       if (frame->nbspec > 0)
              frame->common_bounds = 1;       /* assume yes, and let's see */
       else
              frame->common_bounds = 0;

       if (frame->signal != NULL && frame->nbspec > 0) {
               current_spec = frame->signal;
               start = current_spec->start;
               npix = current_spec->npix;
               current_spec++;
               for (i=1; i < frame->nbspec
                       && (current_spec->start == start
                       && current_spec->npix == npix);
                       i++, current_spec++);
               if (i < frame->nbspec)
                       frame->common_bounds = 0;
       }
       if (frame->noise != NULL && frame->nbspec > 0) {
               current_spec = frame->noise;
               start = current_spec->start;
               npix = current_spec->npix;
               current_spec++;
               for (i=1; i < frame->nbspec
                       && (current_spec->start == start
                       && current_spec->npix == npix);
                       i++, current_spec++);
               if (i < frame->nbspec)
                       frame->common_bounds = 0;
       }

	return(0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        exist_statistical_error()
!
!.purp        checks existence of the cube of statistical errors
!.desc
! exist_statistical_error(frame)	
!
! E3D_file *frame;      E3D file structure
!.ed
-------------------------------------------------------------------- */
int 
exist_statistical_error(E3D_file *frame)		
{
	if (frame->noise != NULL)
		return(TRUE);
	else
		return(FALSE);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.blk                   Routines for spectrum ID access
!
!.func                        exist_spec_ID()
!
!.purp     checks existence of a spectrum ID number in the given E3D frame
!.desc
! exist_spec_ID(frame,specId)	
!
! E3D_file *frame;      E3D file structure
! int specId;           lens number
!.ed
-------------------------------------------------------------------- */
int 
exist_spec_ID(E3D_file *frame, int specId)		
{
	int i;

	if (frame->signal != NULL) {
		find_lens(frame,signal,specId,&i);
	}
	else {
		if (frame->noise != NULL) {
			find_lens(frame,noise,specId,&i);
		}
	}
	if (i >= 0) 
		return(i); /* returns true, number exists */
	else
		return(-1);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        set_ID_and_coordinates()
!
!.purp                    set spectrum ID and coordinates
!.desc
! set_ID_and_coordinates(frame,nb_spec,Id,x,y)	
!
! E3D_file *frame;      E3D file structure
! int nb_spec;          number of spectra
! int *Id;              list of spectrum ID
! float *x;             list of x coordinates
! float *y;             list of y coordinates
!.ed
-------------------------------------------------------------------- */
int set_ID_and_coordinates(E3D_file *frame, int nbl,int *specId, float *xlens, float *ylens)	
{
	int status = 0;
	int one = 1;
	int i, col_no, col_xl, col_yl, col_sp, col_gr;
	char errtext[132];
	TABLE tbl_lens;
	char table_name[lg_name+1];

	sprintf(table_name,"%s[%s]",frame->name,E3D_DATA);
	append_tbl_extension(table_name,InputIO.basic_io);

	if (frame->data_format != EURO3D_FORMAT)
		return(ERR_BAD_PARAM);

	status = open_table(&tbl_lens,table_name,"IO");
	if (status < 0) {
		sprintf(errtext,"set_ID_and_coordinates");
		Handle_Error(errtext,status);
		return(status);
	}
	col_no = get_col_ref(&tbl_lens,E3D_COL_ID);
	col_sp = get_col_ref(&tbl_lens,E3D_COL_NSPAX);
	col_xl = get_col_ref(&tbl_lens,E3D_COL_XPOS);
	col_yl = get_col_ref(&tbl_lens,E3D_COL_YPOS);
	col_gr = get_col_ref(&tbl_lens,E3D_COL_GRP);
	for (i=0;i<nbl;i++) {
		WR_tbl(&tbl_lens,i,col_no,specId+i);
		WR_tbl(&tbl_lens,i,col_sp,&one);
		WR_tbl(&tbl_lens,i,col_xl,xlens+i);
		WR_tbl(&tbl_lens,i,col_yl,ylens+i);
		WR_tbl(&tbl_lens,i,col_gr,&one);
	}
	close_table(&tbl_lens);
	return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        get_ID_and_coordinates()
!
!.purp               get spectra ID and coordinates
!.desc
! get_ID_and_coordinates(frame,nb_spec,Id,x,y,i)	
!
! E3D_file *frame;      E3D file structure
! int *Id;              list of spectrum ID
! float *x;             list of x coordinates
! float *y;             list of y coordinates
!.ed
-------------------------------------------------------------------- */
int get_ID_and_coordinates(E3D_file *frame, int *specId, float *xlens, float *ylens)	
{
	int status = 0;
	int i, col_no, col_xl, col_yl;
	char errtext[132];
	char table_name[lg_name+1];
	TABLE tbl_lens;

	sprintf(table_name,"%s[%s]",frame->name,E3D_DATA);
	append_tbl_extension(table_name,InputIO.basic_io);

	if (frame->data_format != EURO3D_FORMAT)
		return(ERR_BAD_PARAM);

	status = open_table(&tbl_lens,table_name,"I");
	if (status) {
		sprintf(errtext,"get_ID_and_coordinates");
		Handle_Error(errtext,status);
		return(status);
	}
	col_no = get_col_ref(&tbl_lens,E3D_COL_ID);
	col_xl = get_col_ref(&tbl_lens,E3D_COL_XPOS);
	col_yl = get_col_ref(&tbl_lens,E3D_COL_YPOS);
	if ((col_no < 0) || (col_xl < 0) || (col_yl < 0)) {
			status = ERR_NOCOL;
			sprintf(errtext,"get_ID_and_coordinates");
			Handle_Error(errtext,status);
			return(status);
	}
	for (i=0;i<tbl_lens.row;i++) {
		RD_tbl(&tbl_lens,i,col_no,specId+i);
		RD_tbl(&tbl_lens,i,col_xl,xlens+i);
		RD_tbl(&tbl_lens,i,col_yl,ylens+i);
	}
	close_table(&tbl_lens);
	if (status) {
		sprintf(errtext,"get_ID_and_coordinates");
		Handle_Error(errtext,status);
		return(status);
	}
	return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                        get_spectra_ID()
!
!.purp                  get full list of spectrum IDs 
!.desc
! get_spectra_ID(frame,id)	
!
! E3D_file *frame;      E3D file structure
! int *id;              list of lens numbers 
!.ed
-------------------------------------------------------------------- */
int get_spectra_ID(E3D_file *frame, int *specId)	
{
	int status = 0;
	int i;
	char errtext[132];
	E3Dspec_desc *pt_spec_desc;

	pt_spec_desc = NULL;

	if (frame->signal != NULL) {
		pt_spec_desc = frame->signal;
		if (frame->noise != NULL) 
			pt_spec_desc = frame->noise;
	}

	if (pt_spec_desc != NULL) {

		for (i=0; i<frame->nbspec;i++)
			specId[i] = pt_spec_desc[i].specId;
	}
	else 
		status = ERR_BAD_PARAM;
		
	if (status) {
		sprintf(errtext,"get_spectra_ID");
		Handle_Error(errtext,status);
		return(status);
	}
	return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.blk                   Routines for E3D SLICES i/o 
!                                      
        routines to read/write values into spectrum files are
                     given in ../incl/funcdef.h
!
!.func                        RD_slice()
!
!.purp                  reads a slice value
!.desc
! (type) value = RD_slice(slice,index)
!
! SLICE *slice;         slice structure
! int index;            index of value to read
!.ed
!    
!.func                        WR_slice()
!
!.purp                  writes a slice value
!.desc
! WR_slice(slice,index,value)
!
! SLICE *slice;         slice structure
! int index;            index of value to write
! (type) value;         value to write
!.ed
!
!.func                        RD_qslice()
!
!.purp                  reads slice quality flag
!.desc
! (type) value = RD_qslice(slice,index)
!
! SLICE *slice;         slice structure
! int index;            index of value to read
!.ed
!    
!.func                        WR_qslice()
!
!.purp                  writes slice quality flag
!.desc
! WR_qslice(slice,index,value)
!
! SLICE *slice;         slice structure
! int index;            index of value to write
! (type) value;         value to write
!.ed
-------------------------------------------------------------------- */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      alloc_slice_mem()
!
!.purp               allocate memory for slices
!.desc
! alloc_slice_mem(slice, datatype, npts)	
!
! SLICE *slice;         slice structure
! short datatype;       data type
! int npts;             nb of values in slice
!.ed
-------------------------------------------------------------------- */
int alloc_slice_mem(SLICE *slice, short type, int npts) 
{
	int status = 0;

	slice->npts = npts;
	slice->data_type = type;

	slice->specId = (int *)calloc(slice->npts,sizeof(int));
	if (slice->specId == 0) return(ERR_ALLOC);

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
	if (status == 0) {
        	slice->quality = (unsigned long *)calloc(slice->npts,sizeof(unsigned long));
            	if (slice->quality == 0) status = ERR_ALLOC;
	}
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      free_slice_mem()
!
!.purp               free memory for slices
!.desc
! free_slice_mem(slice)	
!
! SLICE *slice;         slice structure
!.ed
-------------------------------------------------------------------- */
int free_slice_mem(SLICE *slice) 
{
	int status = 0;

	free(slice->specId);

    	switch(slice->data_type) {
        case SHORT :
        case INT :
        case LONG :
        case FLOAT :
        case DOUBLE :
	    free((char *)slice->data.s_data);
            break;
    	}
	slice->data.d_data = 0x0;
        free(slice->quality);
	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                      init_new_E3D_slice()
!
!.purp               returns an initialised slice structure items
!.desc
! init_new_E3D_slice(frame,slice,npts)	
!
! E3D_file *frame;      E3D file structure
! SLICE *slice;         slice structure
!.ed
-------------------------------------------------------------------- */
int 
init_new_E3D_slice(E3D_file *frame, SLICE *slice)
{
	char errtext[132], table_name[lg_name+1];
  	int status, i;
	E3Dspec_desc *pt_spec = NULL;
	TABLE tbl_lens;
	
	slice->index = -1;
	slice->npts = frame->nbspec;

	if (frame->signal != NULL)
		pt_spec = frame->signal;
	else {
	    if (frame->noise != NULL)
        	pt_spec = frame->noise;
	}

	if (pt_spec == NULL) {

		strcpy(table_name,frame->table_name);
		append_tbl_extension(table_name,InputIO.basic_io);
		if (open_table(&tbl_lens,table_name,"I") < 0) 
			return(ERR_OPEN);
		slice->npts = tbl_lens.row;
		close_table(&tbl_lens);

		status = alloc_slice_mem(slice, frame->data_type, slice->npts);
		get_lenses_no_from_table(frame,slice->specId);
	}
	else {
		status = alloc_slice_mem(slice, frame->data_type, slice->npts);
		for (i=0;i<slice->npts;i++)
			slice->specId[i] = pt_spec[i].specId;
	}

  	if (status) {
		sprintf(errtext,"init_new_E3D_slice");
		Handle_Error(errtext,status);
	}
  	return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!    
!.func                            get_E3D_slice()
!
!.purp               returns the slice structure items
!.desc
! get_E3D_slice(frame,signal,noise,index)	
!
! E3D_file *frame;      E3D file structure
! SLICE *signal;        slice structure (or NULL)
! SLICE *noise;         slice structure (or NULL)
! int index;            index of slice
!.ed
-------------------------------------------------------------------- */
int extract_E3D_slice(E3D_file *frame, SLICE *slice, E3Dspec_desc **type_of_slice, int index) {

	char errtext[132];
	E3Dspec_desc *current_spec;
	int status, i, colnum;
	long inc[2];
	fitsfile *fptr;
	long npix[2], lpixel[2], fpixel[2];

	current_spec = *type_of_slice;
        status = init_new_E3D_slice(frame,slice); 
	slice->index = index; 

	switch (frame->data_format) {
	case TIGER_FORMAT :
	    for (i=0;i <frame->nbspec; i++, current_spec++) { 
		slice->specId[i] = current_spec->specId; 
		if (index < current_spec->npix) { 
			lseek(frame->imno,(current_spec->data_offset 
				+index*sizeof_item(slice->data_type)),SEEK_SET); 
		if (frame->swapbytes) { 
			read_DOS(frame->imno,(char *)((char *)(slice->data.s_data)+
				i*sizeof_item(slice->data_type)), 
				sizeof_item(slice->data_type)); 
			} else { 
			read(frame->imno,(char *)((char *)(slice->data.s_data)+
				i*sizeof_item(slice->data_type)), 
				sizeof_item(slice->data_type)); 
			} 
		}
		else 
			WR_slice(slice,i,DATA_UNDEF); 
	    } 
	    break;

	case EURO3D_FORMAT :
	    status = 0;
	    fptr = (fitsfile *)frame->external_info;

	    fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	
	    if (status) { 
		status = 0;
		fits_movabs_hdu(fptr, 1, NULL, &status);
		fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
	    }
	
		/* read signal or noise */
	    if (*type_of_slice == frame->signal) 
		fits_get_colnum(fptr,0,E3D_COL_INT,&colnum,&status);
	    else 
		fits_get_colnum(fptr,0,E3D_COL_RMS,&colnum,&status);
	    if (status) {
		status = ERR_BAD_COL; break;
	    }
	    inc[0] = 1;
	    inc[1] = 1;
            npix[0] = (long)(frame->nbspec);
            npix[1] = (long)(frame->common_parameters[2]); 
	    fpixel[0] = index+1; lpixel[0] = index+1;
            switch(slice->data_type) {
		case SHORT :
	    	for (i=0;i <frame->nbspec; i++, current_spec++) { 
	    		fpixel[1] = i+1; lpixel[1] = i+1;
			slice->specId[i] = current_spec->specId; 
			if (index < current_spec->npix) { 
				fits_read_subset_sht(fptr,colnum,1,npix,fpixel,lpixel,inc,0,slice->data.s_data+i,NULL,&status);
			}
			else 
				slice->data.s_data[i] = -MAXSHORT;
	    	}
		break;
		case INT :
		case LONG :
	    	for (i=0;i <frame->nbspec; i++, current_spec++) { 
	    		fpixel[1] = i+1; lpixel[1] = i+1;
			slice->specId[i] = current_spec->specId; 
			if (index < current_spec->npix) { 
				fits_read_subset_lng(fptr,colnum,1,npix,fpixel,lpixel,inc,0,slice->data.l_data+i,NULL,&status);
			}
			else 
				slice->data.l_data[i] = -MAXLONG;
	    	}
		break;
		case FLOAT :
	    	for (i=0;i <frame->nbspec; i++, current_spec++) { 
	    		fpixel[1] = i+1; lpixel[1] = i+1;
			slice->specId[i] = current_spec->specId; 
			if (index < current_spec->npix) { 
				fits_read_subset_flt(fptr,colnum,1,npix,fpixel,lpixel,inc,0,slice->data.f_data+i,NULL,&status);
			}
			else 
				slice->data.f_data[i] = -MAXFLOAT;
	    	}
		break;
		case DOUBLE :
	    	for (i=0;i <frame->nbspec; i++, current_spec++) { 
	    		fpixel[1] = i+1; lpixel[1] = i+1;
			slice->specId[i] = current_spec->specId; 
			if (index < current_spec->npix) { 
				fits_read_subset_dbl(fptr,colnum,1,npix,fpixel,lpixel,inc,0,slice->data.d_data+i,NULL,&status);
			}
			else 
				slice->data.d_data[i] = -MAXDOUBLE;
	    	}
		break;
            }		
	    break;

            default :
	    status = ERR_FORMAT;
	    break;
	}
  	if (status) {
		sprintf(errtext,"get_E3D_slice: slice %d",slice->index); 
		Handle_Error(errtext,status); 
	} 
	return(status);
}

int 
get_E3D_slice(E3D_file *frame, SLICE *signal, SLICE *noise, int index)
{
	int status=0;
	char errtext[132];

	if (! has_common_bounds(frame)) {
		sprintf(errtext,"get_E3D_slice : Unconsistent data. No common bounds");
		Handle_Error(errtext,ERR_IMA_BOUND); 
		return(ERR_IMA_BOUND); 
	}

	if ((signal != NULL) && (frame->signal != NULL)) {
		status = extract_E3D_slice(frame,signal,&(frame->signal),index);
		if (status) return(status);
	} 
	if ((noise != NULL) && (frame->noise != NULL)) {
		status = extract_E3D_slice(frame,noise,&(frame->noise),index);
	} 

  	return(status);
}

#define extract_E3D_slice_noalloc(type_of_slice) \
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

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        put_E3D_slice()
!
!.purp               update/add a slice in a E3D frame 
!.desc
! put_E3D_slice(frame,signal,noise,index)
!
! E3D_file *frame;      E3D file structure
! SLICE *signal;        slice structure (or NULL)
! SLICE *noise;         slice structure (or NULL)
! int index;            index of slice
!.ed
-------------------------------------------------------------------- */
int save_E3D_slice(E3D_file *frame, SLICE *slice, E3Dspec_desc **type_of_slice, int index) {

	char errtext[132];
	int status=0, len, specId;
	E3Dspec_desc *current_spec;
	fitsfile *fptr;
	int coli, coldq, tformat, i, j;
	char format[132], colname[17];
	double dval;
	SPECTRUM spec;

	slice->index = index;
	if (frame->nbspec == 0 || *type_of_slice == NULL) {	/* create first slice */ 
		len = (int)(frame->common_parameters[2]); 
		for (i=0; i<slice->npts;i++) { 
			specId = slice->specId[i];
			init_new_E3D_spec(frame,&spec, len,
				frame->common_parameters[0]);
			for (j=0; j<len; j++) 
				WR_spec(&spec,j,DATA_UNDEF); 
			if (*type_of_slice == frame->signal) 
				put_E3D_spec(frame,&spec,NULL,specId); 
			else 
				put_E3D_spec(frame,NULL,&spec,specId);
		} 
	}; 

	switch (frame->data_format) {
	case TIGER_FORMAT :
	    for (i=0; i<frame->nbspec;i++) {
		specId = slice->specId[i]; 
		if (*type_of_slice == frame->signal) {
			find_lens(frame,signal,specId,&j); 
		}
		else {
			find_lens(frame,noise,specId,&j); 
		}
		if (j<0) { 
			status = ERR_NODATA; 
			break; 
		} 
		current_spec = *type_of_slice+j;
		dval = (double)RD_slice(slice,i);
		lseek(frame->imno,(current_spec->data_offset
			+index*sizeof_item(slice->data_type)),SEEK_SET); 
		if (frame->swapbytes) { 
			write_DOS(frame->imno,(char *)((char *)(slice->data.s_data)+
				i*sizeof_item(slice->data_type)), 
				sizeof_item(slice->data_type)); 
		} else { 
			write(frame->imno,(char *)((char *)(slice->data.s_data)+
				i*sizeof_item(slice->data_type)), 
				sizeof_item(slice->data_type)); 
		} 
	     } 
	     break;

	case EURO3D_FORMAT :
	        status = 0;
		fptr = (fitsfile *)frame->external_info;

		fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);

		if (status) { 
			status = 0;
			fits_movabs_hdu(fptr, 1, NULL, &status);
			fits_movnam_hdu(fptr,BINARY_TBL,E3D_DATA,0,&status);
                }

		if (*type_of_slice == frame->signal) 
			strcpy(colname,E3D_COL_INT);
		else 
			strcpy(colname,E3D_COL_RMS);

		switch(slice->data_type) {
			case SHORT :
				sprintf(format,"%dI",len);
				tformat = TSHORT;
				break;
			case INT :
			case LONG :
				sprintf(format,"%dJ",len);
				tformat = TLONG;
				break;
			case FLOAT :
				sprintf(format,"%dE",len);
				tformat = TFLOAT;
				break;
			case DOUBLE :
				sprintf(format,"%dD",len);
				tformat = TDOUBLE;
				break;
		}
		fits_get_colnum(fptr,0,colname,&coli,&status);
		if (status) {
			status = 0;
			fits_get_num_cols(fptr, &coli, &status);
			coli++;
			fits_insert_col(fptr,coli,colname,format,&status);
			if (status) {
				status = ERR_WRIT;
				break;
			}
		}
		fits_get_colnum(fptr,0,E3D_COL_DQ,&coldq,&status);
		if (status) {
			status = 0;
			sprintf(format,"%dJ",len);
			fits_get_num_cols(fptr, &coldq, &status);
			coldq++;
			fits_insert_col(fptr,coldq,E3D_COL_DQ,format,&status);
			if (status) {
				status = ERR_WRIT;
				break;
			}
		}
	        for (i=0; i<frame->nbspec;i++) {
			specId = slice->specId[i]; 
			if (*type_of_slice == frame->signal) {
				find_lens(frame,signal,specId,&j); 
			}
			else {
				find_lens(frame,noise,specId,&j); 
			}
			if (j<0 || j>= frame->nbspec) { 
				status = ERR_NODATA; 
				break; 
			} 
			current_spec = *type_of_slice+j;

			/* write signal or noise */
		
            		switch(slice->data_type) {
			case SHORT :
				fits_write_col_sht(fptr,coli,i+1,index+1,1,slice->data.s_data+i,&status);
				break;
			case INT :
			case LONG :
				fits_write_col_lng(fptr,coli,i+1,index+1,1,slice->data.l_data+i,&status);
				break;
			case FLOAT :
				fits_write_col_flt(fptr,coli,i+1,index+1,1,slice->data.f_data+i,&status);
				break;
			case DOUBLE :
				fits_write_col_dbl(fptr,coli,i+1,index+1,1,slice->data.d_data+i,&status);
				break;
			}
			if (status) {
				status = ERR_WRIT;
				break;
			}
			fits_write_col_lng(fptr,coldq,i+1,index+1,1,slice->quality+i,&status);
	    	}
		break;

                default :
	        status = ERR_FORMAT;
	        break;
	}
	free_slice_mem(slice);
 
	if (status) {
		sprintf(errtext,"put_E3D_slice: slice %d",slice->index);
		Handle_Error(errtext,status); 
	}
	return(status);
}

int 
put_E3D_slice(E3D_file *frame, SLICE *signal, SLICE *noise, int index)
{
	char errtext[132];
	int status=0;

	if (frame->iomode == (int)I_MODE) {
		sprintf(errtext,"put_E3D_slice: file %s is write protected",
			frame->name);
		Handle_Error(errtext,ERR_ACCESS); 
		return(ERR_ACCESS); 
	} 
	if (! has_common_bounds(frame)) {
		sprintf(errtext,"put_E3D_slice : Unconsistent data");
		Handle_Error(errtext,ERR_BAD_PARAM); 
		return(ERR_BAD_PARAM); 
	}

	if (signal != NULL) {
		status = save_E3D_slice(frame,signal,&(frame->signal),index);
		if (status) return(status);
	} 
	if (noise != NULL) {
		status = save_E3D_slice(frame,noise,&(frame->noise),index);
	}

  	return(status);
}
