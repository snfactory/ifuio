/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        fits_primary_hd.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      Basic i/o routines
! COMMENT      
! VERSION      1.0  2003-Oct-06 : Creation, AP
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

#ifdef FITS
#include <fitsio.h>
#include <longnam.h>
#endif

extern IO_Format InputIO, OutputIO;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        open_primary_hd()
!
!.purp          opens a primary header (FITS files only !)
!.desc
! open_primary_hd(frame,name,mode)	
!
! Anyfile *frame;       file structure
! char *name;           frame name
! char *mode;           open mode (Input,Ouput,IO)
!.ed
-------------------------------------------------------------------- */
int 
open_primary_hd(Anyfile *frame, char *name, char *mode)		
{
	char errtext[132], filename[lg_name+1];
  	int status, nbaxes, iomode, int_datatype;
  	int one=1;
#ifdef FITS
    fitsfile *fptr;
    int nbread;
    int npix;
    int group = 0;
#endif

	memset(frame->history,' ',lg_hist);
	frame->history[lg_hist] = '\0';
	frame->external_info = 0;
	frame->file_type = T_PRIMHD;
	frame->data_format = InputIO.basic_io;

	strcpy(filename,name);
 	first_blk(filename); 
	strcpy(frame->name,filename);
	append_ima_extension(frame->name,InputIO.basic_io);

	strcpy(filename,frame->name);

	if (!exist(filename)) { /* check if file exists */
		status = ERR_OPEN;
		sprintf(errtext,"open_primary_hd: file %s",filename);
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
	/*
	if (nbaxes > 0)
		status = ERR_BAD_PARAM; */
	break;
#endif
	default :
		status = ERR_BAD_PARAM; break;
	}

  	if (status) {
		sprintf(errtext,"open_primary_hd: file %s",filename);
		status = get_tiger_errcode(frame->data_format,status);
		Handle_Error(errtext,status);
	}
  	return(status);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                       close_primary_hd()
!
!.purp             closes a currently active primary fits header
!.desc
! close_frame(frame)	
!
! Anyfile *frame;       file structure
!.ed
-------------------------------------------------------------------- */
int 
close_primary_hd(Anyfile *frame)		/* close active frame */
{
	char   errtext[132], filename[lg_name+1];
	int stat;
#ifdef FITS
	fitsfile *fptr;
#endif
	strcpy(filename,frame->name);

	switch (frame->data_format) {
#ifdef FITS
    case FITS_A_FORMAT :
    case FITS_B_FORMAT :
	stat = 0;
	fptr = (fitsfile *)frame->external_info;
	fits_close_file(fptr,&stat);
	frame->external_info = NULL;
	break;
#endif
	}
  	if (stat) {
		sprintf(errtext,"close_primary_hd: frame %s",filename);
		stat = get_tiger_errcode(frame->data_format,stat);
		Handle_Error(errtext,stat);
	}
  	return(stat);
}
