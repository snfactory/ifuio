/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        io_misc.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      general tools for I/O
! COMMENT      
! VERSION      4.0  1992-June-15 : creation AR 
!              4.1  2002-Oct-01  : added quality flags
!              4.2  2005-Jan-05  : file_format improvement (now check several extensions)
!              4.3  2005-Jan-06  : suppress genreic types IMA_TYPE and TBL_TYPE
!              4.4  2005-Jun-20  : fixed bug on keyword length in alloc_new_desc
---------------------------------------------------------------------*/
#include <stdlib.h>
#include <IFU_io.h>

#include <data_io.h>
#ifdef MIDAS
#include "../midas_io/midas_defs.h"
#endif
#ifdef IRAF
#include "../iraf_io/incl/iraf_def.h"
#endif
#ifdef FITS
#include <fitsio.h>
#include <longnam.h>
#endif

IO_Format InputIO, OutputIO;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	Input/Output format for storing data
!
!.func                       decode_format()
!							
!.purp          sets up strcutures describing IO data format
!
!.desc
! int decode_format(format, IOstruct) 
!
! char *format;         data format given by user (i.e. midas, iraf (stsdas), fits) 
! IOFormat *IOstruct;   data format structure
!							
!.ed
--------------------------------------------------------------------*/

int 
decode_format(char *format, IO_Format *IOstruct)
{
	short Format;

	IOstruct->basic_io = FITS_B_FORMAT; /* default format*/
	IOstruct->datacubes = 0;

	lower_strg(format);

	if (strlen(format) == 0) return(OK);

	if (strcmp(format,"midas") == 0)  Format = MIDAS_FORMAT; 
	if (strcmp(format,"iraf") == 0)   Format = IRAF_FORMAT; 
	if (strcmp(format,"stsdas") == 0) Format = STSDAS_FORMAT;
	if (strcmp(format,"fits") == 0)   Format = FITS_B_FORMAT;
	if (strcmp(format,"fits,a") == 0) Format = FITS_A_FORMAT;
	if (strcmp(format,"fits,b") == 0) Format = FITS_B_FORMAT;
	if (strcmp(format,"euro3d") == 0) Format = EURO3D_FORMAT;
	if (strcmp(format,"tiger+fits") == 0) Format = TIGER_FITS_FORMAT;
	if (strcmp(format,"tiger+midas") == 0) Format = TIGER_MIDAS_FORMAT;

#ifdef MIDAS
	if (Format == MIDAS_FORMAT) {
		IOstruct->basic_io = Format;
		return(OK);
	}
	if (Format == TIGER_MIDAS_FORMAT) {
		IOstruct->basic_io = MIDAS_FORMAT;
		IOstruct->datacubes = TIGER_FORMAT;
		return(OK);
	}
#endif
#ifdef IRAF
	if ((Format == IRAF_FORMAT) || (Format == STSDAS_FORMAT)) {
		IOstruct->basic_io = Format;
		return(OK);
	}
#endif
#ifdef FITS
	if ((Format == FITS_A_FORMAT) || (Format == FITS_B_FORMAT)) {
		IOstruct->basic_io = Format;
		return(OK);
	}
	if (Format == TIGER_FITS_FORMAT) {
		IOstruct->basic_io = FITS_B_FORMAT;
		IOstruct->datacubes = TIGER_FORMAT;
		return(OK);
	}
	if (Format == EURO3D_FORMAT) {
		IOstruct->basic_io = FITS_B_FORMAT;
		IOstruct->datacubes = EURO3D_FORMAT;
		return(OK);
	}
#endif
	return(ERR_BAD_PARAM);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func                       set_inputformat()
!							
!.purp          sets up the variables describing input data format
!
!.desc
! int set_inputformat(format) 
!
! char *format;         data format (i.e. midas, iraf (stsdas), fits) 
!							
!.ed
--------------------------------------------------------------------*/

int 
set_inputformat(char *format)
{
	char errtext[132];
	IO_Format IOstruct;

	if (decode_format(format,&IOstruct)) {

		sprintf(errtext,"set_inputformat, %s has not been included into I/O library\n",format);
		Handle_Error(errtext,ERR_BAD_TYPE);
		return(ERR_BAD_TYPE);
	}
	InputIO = IOstruct;
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       set_outputformat()
!							
!.purp          sets up the variables describing output data format
!				                                                      
!.desc
! int set_outputformat(format) 
!
! char *format;         data format (i.e. midas, iraf (stsdas), fits) 
!							
!.ed
--------------------------------------------------------------------*/

int 
set_outputformat(char *format)
{
	char errtext[132];
	IO_Format IOstruct;

	if (decode_format(format,&IOstruct)) {

		sprintf(errtext,"set_outputformat, %s has not been included into I/O library\n",format);
		Handle_Error(errtext,ERR_BAD_TYPE);
		return(ERR_BAD_TYPE);
	}
	OutputIO = IOstruct;
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       set_user_dataformat()
!							
!.purp        set user's default format for data storage
!				                                                      
!.desc
! int set_user_dataformat() 
!							
!.ed
--------------------------------------------------------------------*/

int 
set_user_dataformat() 
{
	char format_strg[132];
	char *pt_env;
	int status;

	pt_env = getenv("IFU_DEFAULT_FMT");
	if ((pt_env == NULL) || (*pt_env == 0))
		return(FITS_B_FORMAT);

	if (strlen(pt_env) > 80)
		pt_env[80] = 0;
	strcpy(format_strg,pt_env);

	first_blk(format_strg);
	status = set_inputformat(format_strg);
	status = set_outputformat(format_strg);
	return(status);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk          	 Retrieving I/O parameters according to data format
!
!							
!.func                       append_ima_extension()
!							
!.purp        appends extension to filename according to given format
!
!.desc
! void append_ima_extension(ima_name,format)
!
! char *ima_name;       image name
! short format;         image format
!.ed								
-------------------------------------------------------------------- */

void append_ima_extension(char *name, short format)
{
	char *pt_filename;

	pt_filename = strrchr(name,'/');
	if (pt_filename == NULL)
		pt_filename = name;

	if (strchr(pt_filename,'.') == NULL) 
        switch (format) { 
            case MIDAS_FORMAT  :
                        strcat(name,".bdf"); 
                        break; 
            case STSDAS_FORMAT :
            case IRAF_FORMAT   :
                        strcat(name,".imh"); 
                        break; 
            case FITS_A_FORMAT :
            case FITS_B_FORMAT :
                        strcat(name,".fits");
                        break; 
        };
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       append_tbl_extension()
!							
!.purp        appends extension to filename according to given format
!
!.desc
! void append_tbl_extension(tbl_name,format)
!
! char *tbl_name;       table name
! short format;         table format
!.ed								
-------------------------------------------------------------------- */

void append_tbl_extension(char *name, short format)
{
	char *pt_filename;

	pt_filename = strrchr(name,'/');
	if (pt_filename == NULL)
		pt_filename = name;

	if (strchr(pt_filename,'.') == NULL)
        switch (format) {
            case MIDAS_FORMAT  : 
                        strcat(name,".tbl");
                        break; 
            case IRAF_FORMAT   :
            case STSDAS_FORMAT :
                        strcat(name,".tab"); 
                        break; 
            case FITS_A_FORMAT :
            case FITS_B_FORMAT :
                        strcat(name,".fits");
                        break; 
        };

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       append_datacube_extension()
!							
!.purp        appends extension to filename according to given format
!
!.desc
! void append_datacube_extension(tbl_name,format)
!
! char *tbl_name;       table name
! short format;         table format
!.ed								
-------------------------------------------------------------------- */

void append_datacube_extension(char *name, short format)
{
	char *pt_filename;

	pt_filename = strrchr(name,'/');
	if (pt_filename == NULL)
		pt_filename = name;

	if (strchr(pt_filename,'.') == NULL)
        switch (format) {
            case EURO3D_FORMAT :
                        strcat(name,".fits");
                        break; 
            case TIGER_FORMAT :
                        strcat(name,".tig");
                        break; 
        };

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       remove_file_extension()
!							
!.purp                 remove a filename extension if any
!
!.desc
! void remove_file_extension(filename)
!
! char *tbl_name;       table name
! short format;         table format
!.ed								
-------------------------------------------------------------------- */

void remove_file_extension(char *name)
{
    char *pt_filename, *pt_extension;

    pt_filename = strrchr(name,'/');
    if (pt_filename == NULL)
	pt_filename = name;

    pt_extension = strchr(pt_filename,'.');

    if (pt_extension == NULL)
	return;
    *pt_extension = 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       match_extension()
!							
!.purp                 try to guess the missing extension
!
!.desc
! void match_extension(filename, userformat, extension)
!
! char *filename;       filename (without extension)
! IO_Format format;      user format structure
! char *extension;      where to store the extension found
!.ed								
-------------------------------------------------------------------- */

void match_extension(char *fname, IO_Format format, char *extension ) 
{
	char filename[lg_name+1];
	char file_list[3][lg_name+1];
	int found = 0;

	file_list[0][0] = '\0';
	file_list[1][0] = '\0';
	file_list[2][0] = '\0';

	/* check if image name may be matched */
	strcpy(filename,fname);
	append_ima_extension(filename,format.basic_io);
	if (exist(filename)) {
		strcpy(file_list[found],filename);
		found++;
	}
	/* check if table name may be matched */
	strcpy(filename,fname);
	append_tbl_extension(filename,format.basic_io);
	if (exist(filename)) {
		if ((found == 0) || (strcmp(file_list[0],filename) != 0)) {
			strcpy(file_list[found],filename);
			found++;
		}
	}
	/* check if datacube name may be matched */
	strcpy(filename,fname);
	append_datacube_extension(filename,format.datacubes);
	if (exist(filename)) {
		strcpy(file_list[found],filename);
		found++;
	}
	if (found > 1) {
		switch (found) {
			case 2:
			print_error("Can't guess which filename you need : %s or %s",file_list[0],file_list[1]);
			break;
			case 3:
			print_error("Can't guess which filename you need : %s, %s, or %s",file_list[0],file_list[1],file_list[2]);
			break;
		}
		print_error("Please submit the file extension");
		exit_session(ERR_BAD_PARAM);
	}
	return;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                        naxis_frame()
!
!.purp   	returns the number of axis for given frame
!.desc
! naxis_frame(name, dataformat)	
!
! char *name;           frame name
! short dataformat;     data format (Midas, Fits, ...)
!.ed
-------------------------------------------------------------------- */

int naxis_frame(char *filename, short ioformat)		
{
  	int status=0, nbaxes=0, imno;
	int info[5];
	int dims[3];
  	int dim=3;
#ifdef MIDAS
	int unit, nulls, nbread;
#endif
#ifdef IRAF
	int len, iomode;
#endif
#ifdef FITS
    fitsfile *fptr;
    int hdutype;
#endif

    if (!exist(filename)) return(-1);
    switch (ioformat) {

#ifdef MIDAS
	case MIDAS_FORMAT :
		status = SCFINF(filename,2,info);  
		if (status != 0) break;
		status = SCFOPN(filename, info[1], 0, F_IMA_TYPE, &imno);
		SCDRDI(imno,"NPIX",1,dim,&nbread,dims,&unit,&nulls);
		status = SCFCLO(imno);
#endif
#ifdef IRAF
	case IRAF_FORMAT :
	case STSDAS_FORMAT :
		len = strlen(filename);
		iomode = RDONLY;
	        uimopn(filename,&iomode,&imno,&status,len);
        	if (status != 0) break;
        	uimgid(&imno,&int_datatype,&dim,dims,&status);
		uimclo(&imno,&status);
		break;
#endif
#ifdef FITS
	case FITS_A_FORMAT :
	case FITS_B_FORMAT :
		status =0;
		if (fits_open_file(&fptr,filename,READONLY,&status)) break;
		if (fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status)) break;
		if (nbaxes == 0) {
			/* empty primary header, go to next extension */
			fits_movabs_hdu(fptr, (int)2, &hdutype, &status);
			if (hdutype != IMAGE_HDU) break;
			fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status);
		}
		fits_close_file(fptr, &status);
#endif
	}

    if (status) 
	nbaxes = -1;
    return(nbaxes);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       file_format()
!
!.purp       try to guess the file format (fits, ...) and type (image, table, etc ...)
!
!.desc
! int file_format(filename)
!
! char *filename;       filename
!
!.ed
--------------------------------------------------------------------*/

char *file_format(char *filename) {

    FILE *fd;
    char *format;
    char *pt_filename, *pt_ext, fname[lg_name+1], extension[10], version[16];
    char buffer[2880];
    int status = 0;

#ifdef FITS
    fitsfile *fptr;
    int hdutype, nbaxes;
#endif

    format = (char *)malloc(3*sizeof(char));
    format[0] = -1;
    format[1] = -1;
    format[2] = 0;

    pt_filename = strrchr(filename,'/');
    if (pt_filename == NULL)
	pt_filename = filename;
    strcpy(fname,filename);
    pt_ext = strchr(pt_filename,'.');
    if (pt_ext == NULL) {
	/* check which extension may meet user requirement */
        match_extension(filename,InputIO,extension);
        strcat(fname,extension);
    }
    else {
      if (strcmp(pt_ext,".max") == 0) {
	format[0] = TIGER_FORMAT;
	format[1] = T_TIGMAX;
	return(format);
      }
    }

    if (strrchr(filename,'[') != NULL) {
	/* fits file with extension */
	format[0] = FITS_B_FORMAT;
	/* check HDU type */
	fits_open_file(&fptr,fname,READONLY,&status);
	fits_get_hdu_type(fptr, &hdutype, &status);
        fits_close_file(fptr,&status);
	switch(hdutype) {
		case ASCII_TBL :
        	case BINARY_TBL :
	     		format[1] = T_TABLE;
			break;
		case IMAGE_HDU :
	     		format[1] = T_IMA1D;
			break;
	}
    }
    else {
    fd = fopen(fname,"r");
    if (fd == NULL) {
      return(format);
    }
    if (fread(buffer,1,11,fd) < 11) {
	fclose(fd);
	return(format);
    }
    if (strncmp(buffer,"SIMPLE  =",9) == 0) {
	format[0] = FITS_B_FORMAT;
	/* check HDU type */
	fits_open_file(&fptr,fname,READONLY,&status);
	fits_get_hdu_type(fptr, &hdutype, &status);
	if (!status) {	
		fits_read_key(fptr, TINT,"NAXIS", &nbaxes,NULL, &status);
		if (nbaxes == 0) { /* empty primary header, go to next */
			fits_movabs_hdu(fptr, (int)2, &hdutype, &status);
		}
	}
	switch(hdutype) {
		case ASCII_TBL :
        	case BINARY_TBL :
			status = 0;
			fits_movabs_hdu(fptr, (int)1, &hdutype, &status);
			fits_read_key_str(fptr,E3D_VERS,version, NULL, &status);
			if (status) {
				status = 0;
	     			format[1] = T_TABLE;
			}
			else {
			 	format[0] = EURO3D_FORMAT;
			 	format[1] = T_TIGER;
			}
			break;
		case IMAGE_HDU :
	     		format[1] = T_IMA1D;
			break;
	}
        fits_close_file(fptr,&status);
    }
    else {
	if (strncmp(buffer+4,"TABLE ",6) == 0) {
        		format[0] = MIDAS_FORMAT;
        		format[1] = T_TABLE;
	}     
	else {
		if (strncmp(buffer+4,"IMAGE ",6) == 0) {
			format[0] = MIDAS_FORMAT;
			format[1] = T_IMA1D;
		}
		else {
			if (strncmp(buffer,"\0i\0m\0h\0d\0r\0",11) == 0) {
				format[0] = IRAF_FORMAT;
				format[1] = T_IMA1D;
			}
			else {
				if (strncmp(buffer,"\0i\0m\0p\0i\0x\0",11) == 0) {
					format[0] = IRAF_FORMAT;
					format[1] = T_IMA1D;
				}
				else {
					if (strncmp(buffer,"v1.0",4) == 0) {
					 	format[0] = TIGER_FORMAT;
					 	format[1] = T_TIGER;
    					}
    				}
			}
	  	}
     	}
    }
    fclose(fd);
    }
    return(format);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       file_type()
!
!.purp          returns the filetype (i.e. spectrum, image, table ..)
!
!.desc
! int file_type(filename)
!
! char *filename;       filename (including extension !)
!
!.ed
--------------------------------------------------------------------*/

int file_type(char *filename)
{
    SPECTRUM spec;
    IMAGE2D Image2d;
    int type_of_file, naxis;
    int unknown = 'U';
    char *format, *file_format();

    disable_user_warnings();

    format = file_format(filename);
    
    switch (format[0]) {

	case EURO3D_FORMAT :
	case TIGER_FORMAT :
	  	type_of_file = format[1];	
		break;

	default :
		switch (format[1]) {
			case T_IMA1D :

			/* try to guess number of axis */
			naxis = naxis_frame(filename, (int)format[0]);
			switch(naxis) {
				case 1: type_of_file = T_IMA1D;
					break;
				case 2: type_of_file = T_IMA2D;
					break;
				case 3: type_of_file = T_IMA3D;
					break;
				default : type_of_file = unknown;
					break;
					
			}
			break;

			case T_TABLE :

	      		type_of_file = T_TABLE;
			break;

			default :
    			type_of_file = unknown;
			break;
		}
    }
    restore_user_warnings();
    return(type_of_file); 
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       exist_extension()
!
!.purp          returns extension hdu number if extension already exist in file, 0 otherwise
!
!.desc
! int exist_extension(file)
!
! char  *file;          filename with extension
!
!.ed
--------------------------------------------------------------------*/

int exist_extension(char *file) {

        char *pt_sep;
        int status = 0, hdu_num;
	fitsfile *fptr;

        pt_sep = strchr(file,'[');
        if (pt_sep == NULL)    /* no extension */
		return(-1);
	fits_open_file(&fptr,file,0,&status);
	if (status)
		return 0;
	else {
	        fits_get_hdu_num(fptr,&hdu_num);
		fits_close_file(fptr,&status);
		return(hdu_num);
	}
	
	return(0);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       get_iomode_code()
!
!.purp          returns the iomode code according to data format
!
!.desc
! int get_iomode_code(data_format, iomode)
!
! short data_format;    code for data format 
! int   iomode;         required i/o mode
!
!.ed
--------------------------------------------------------------------*/

int 
get_iomode_code(short data_format, int iomode)
{
	switch (data_format) {
#ifdef MIDAS
	case MIDAS_FORMAT :
		switch (iomode) {
		case I_MODE :
			return(F_I_MODE);
			break;
		case O_MODE :
			return(F_O_MODE);
			break;
		case IO_MODE :
			return (F_IO_MODE);
			break;
		}
		break;
#endif
#ifdef IRAF
	case IRAF_FORMAT:
	case STSDAS_FORMAT :
		switch (iomode) {
		case I_MODE :
			return(RDONLY);
			break;
		case O_MODE :
			return(RDWRIT);
			break;
		case IO_MODE :
			return (RDWRIT);
			break;
		}
		break;
#endif
#ifdef FITS
	case FITS_A_FORMAT :
	case FITS_B_FORMAT :
		switch (iomode) {
		case I_MODE :
			return(READONLY);
			break;
		case O_MODE :
		case IO_MODE :
			return(READWRITE);
			break;
		}
		break;
#endif
	}
	return(UNKNOWN);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       get_datatype_code()
!
!.purp          returns the datatype code according to data format
!
!.desc
! int get_datatype_code(data_format, data_type)
!
! short data_format;    code for data format 
! short data_type;      required data type
!
!.ed
--------------------------------------------------------------------*/
int 
get_datatype_code(short data_format, short data_type)
{
	switch (data_format) {
#ifdef MIDAS
	case MIDAS_FORMAT :
		switch (data_type) {
		case CHAR :
			return(D_C_FORMAT);
			break;
		case SHORT :
			return(D_I2_FORMAT);
			break;
		case INT :
		case LONG :
			return(D_I4_FORMAT);
			break;
		case FLOAT :
			return(D_R4_FORMAT);
			break;
		case DOUBLE :
			return(D_R8_FORMAT);
			break;
		}
		break;
#endif
#ifdef IRAF
	case IRAF_FORMAT:
	case STSDAS_FORMAT :
		switch (data_type) {
		case CHAR :
			return(-1);
			break;
		case SHORT :
			return(TYSHOR);
			break;
		case INT :
			return(TYINT);
			break;
		case LONG :
			return(TYINT);
			break;
		case FLOAT :
			return(TYREAL);
			break;
		case DOUBLE :
			return(TYDOUB);
			break;
		}
		break;
#endif
#ifdef FITS
	case FITS_A_FORMAT :
	case FITS_B_FORMAT :
	case EURO3D_FORMAT :
		switch (data_type) {
		case CHAR :
			return(TSTRING);
			break;
		case USHORT :
			return(TUSHORT);
			break;
		case SHORT :
			return(TSHORT);
			break;
		case INT :
		case LONG :
			return(TLONG);
			break;
		case FLOAT :
			return(TFLOAT);
			break;
		case DOUBLE :
			return(TDOUBLE);
			break;
		}
		break;

#endif
	}
	return(UNKNOWN);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       decode_datatype()
!
!.purp                      returns the data type
!
!.desc
! short decode_datatype(data_format, data_type)
!
! short data_format;    code for data format 
! short data_type;      coded data type
!
!.ed
--------------------------------------------------------------------*/
short 
decode_datatype(short data_format, short data_type)
{
	switch (data_format) {
#ifdef MIDAS
	case MIDAS_FORMAT :
		switch (data_type) {
		case D_C_FORMAT :
			return(CHAR);
			break;
		case D_I2_FORMAT :
			return(SHORT);
			break;
		case D_I4_FORMAT :
			return(LONG);
			break;
		case D_R4_FORMAT :
			return(FLOAT);
			break;
		case D_R8_FORMAT :
			return(DOUBLE);
			break;
		}
		break;
#endif
#ifdef IRAF
	case IRAF_FORMAT:
	case STSDAS_FORMAT :
		switch (data_type) {
		case TYSHOR :
			return(SHORT);
			break;
		case TYINT :
			return(LONG);
			break;
		case TYLONG :
			return(LONG);
			break;
		case TYREAL :
			return(FLOAT);
			break;
		case TYDOUB :
			return(DOUBLE);
			break;
		default :
			if (data_type < 0) 
				return(CHAR);
		}
		break;
#endif
#ifdef FITS
	case FITS_A_FORMAT :
	case FITS_B_FORMAT :
		switch (data_type) {
		case TSTRING :
			return(CHAR);
			break;
		case TUSHORT :
			return(USHORT);
			break;
		case TSHORT :
			return(SHORT);
			break;
		case TINT :
		case TLONG :
			return(LONG);
			break;
		case TFLOAT :
			return(FLOAT);
			break;
		case TDOUBLE :
			return(DOUBLE);
			break;
		}
		break;
#endif
	}
	return(UNKNOWN);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       sizeof_item()
!
!.purp                      returns the size of given item
!
!.desc
! int sizeof_item(data_type)
!
! short data_type;      coded data type
!
!.ed
--------------------------------------------------------------------*/
int 
sizeof_item(short data_type)
{
	switch (data_type) {
	case CHAR :
		return(sizeof(char));
		break;
	case SHORT :
		return(sizeof(short));
		break;
	case INT :
		return(sizeof(int));
		break;
	case LONG :
		return(sizeof(long));
		break;
	case FLOAT :
		return(sizeof(float));
		break;
	case DOUBLE :
		return(sizeof(double));
		break;
	}
	return(UNKNOWN);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	        Allocating space for data exchanges
!							
!.func                       alloc_spec_mem()
!							
!.purp                allocate memory space for 1D frames
!
!.desc
! alloc_spec_mem(spectrum, type) 
!
! SPECTRUM *spectrum;   spectrum structure
! short datatype;       data type         
!.ed
-------------------------------------------------------------------- */
int 
alloc_spec_mem(SPECTRUM *spectrum, short type) 
{
	spectrum->data_type = type;
	switch(type) {
		case SHORT :
			spectrum->data.s_data = (short *)calloc(spectrum->npts,sizeof(short));
			if (spectrum->data.s_data == 0) return(ERR_ALLOC);
			break;
		case INT :
		case LONG :
			spectrum->data.l_data = (long *)calloc(spectrum->npts,sizeof(long));
			if (spectrum->data.l_data == 0) return(ERR_ALLOC);
			break;
		case FLOAT :
			spectrum->data.f_data = (float *)calloc(spectrum->npts,sizeof(float));
			if (spectrum->data.f_data == 0) return(ERR_ALLOC);
			break;
		case DOUBLE :
			spectrum->data.d_data = (double *)calloc(spectrum->npts,sizeof(double));
			if (spectrum->data.d_data == 0) return(ERR_ALLOC);
			break;
	}
	spectrum->quality = (unsigned long *)calloc(spectrum->npts,sizeof(unsigned long));
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       free_spec_mem()
!							
!.purp                free memory space for 1D frames
!
!.desc
! free_spec_mem(spectrum) 
!
! SPECTRUM *spectrum;   spectrum structure
!.ed
-------------------------------------------------------------------- */
int 
free_spec_mem(SPECTRUM *spectrum) 
{
	if (spectrum->data.d_data == NULL)
		return(OK);
;
	switch(spectrum->data_type) {
		case SHORT :
		case INT :
		case LONG :
		case FLOAT :
		case DOUBLE :
			free((char *)spectrum->data.d_data);
			break;
	}
	spectrum->data.d_data = 0x0;
	free((char *)spectrum->quality);
	spectrum->quality = 0x0;
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       alloc_frame_mem()
!							
!.purp                allocate memory space for 2D frames
!
!.desc
! alloc_frame_mem(image, datatype) 
!
! IMAGE2D *image;       image structure
! short datatype;       data type         
!.ed
-------------------------------------------------------------------- */
int 
alloc_frame_mem(IMAGE2D *frame, short type) 
{
	frame->data_type = type;
	switch(type) {
		case USHORT :
		case SHORT :
			frame->data.s_data = (short *)calloc(frame->nx*frame->ny,sizeof(short));
			if (frame->data.s_data == 0) return(ERR_ALLOC); 
			break;
		case INT :
		case LONG :
			frame->data.l_data = (long *)calloc(frame->nx*frame->ny,sizeof(long));
			if (frame->data.l_data == 0) return(ERR_ALLOC); 
			break;
		case FLOAT :
			frame->data.f_data = (float *)calloc(frame->nx*frame->ny,sizeof(float));
			if (frame->data.f_data == 0) return(ERR_ALLOC); 
			break;
		case DOUBLE :
			frame->data.d_data = (double *)calloc(frame->nx*frame->ny,sizeof(double));
			if (frame->data.d_data == 0) return(ERR_ALLOC); 
			break;
		default :
			return(ERR_ALLOC);
			break;
	}
	frame->quality = (unsigned long *)calloc(frame->nx*frame->ny,sizeof(unsigned long));
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       free_frame_mem()
!							
!.purp                free memory space for 2D frames
!
!.desc
! free_frame_mem(image) 
!
! IMAGE2D *image;       image structure
!.ed
-------------------------------------------------------------------- */
int 
free_frame_mem(IMAGE2D *frame) 
{
	if (frame->data.d_data == NULL)
		return(OK);
;
	switch(frame->data_type) {
		case SHORT :
		case INT :
		case LONG :
		case FLOAT :
		case DOUBLE :
			free((char *)frame->data.d_data);
			break;
	}
	frame->data.d_data = 0x0;
	free((char *)frame->quality);
	frame->quality = 0x0;
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       alloc_cube_mem()
!							
!.purp                allocate memory space for 3D frames
!
!.desc
! alloc_cube_mem(image, datatype) 
!
! IMAGE3D *image;       image structure
! short datatype;       data type         
!.ed
-------------------------------------------------------------------- */
int 
alloc_cube_mem(IMAGE3D *frame, short type) 
{
	int npts = frame->nx*frame->ny*frame->nz;

	frame->data_type = type;
	switch(type) {
		case USHORT :
		case SHORT :
			frame->data.s_data = (short *)calloc(npts,sizeof(short));
			if (frame->data.s_data == 0) return(ERR_ALLOC); 
			break;
		case INT :
		case LONG :
			frame->data.l_data = (long *)calloc(npts,sizeof(long));
			if (frame->data.l_data == 0) return(ERR_ALLOC); 
			break;
		case FLOAT :
			frame->data.f_data = (float *)calloc(npts,sizeof(float));
			if (frame->data.f_data == 0) return(ERR_ALLOC); 
			break;
		case DOUBLE :
			frame->data.d_data = (double *)calloc(npts,sizeof(double));
			if (frame->data.d_data == 0) return(ERR_ALLOC); 
			break;
		default :
			return(ERR_ALLOC);
			break;
	}
	frame->quality = (unsigned long *)calloc(npts,sizeof(unsigned long));
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       free_cube_mem()
!							
!.purp                free memory space for 3D frames
!
!.desc
! free_cube_mem(image) 
!
! IMAGE3D *image;       image structure
!.ed
-------------------------------------------------------------------- */
int 
free_cube_mem(IMAGE3D *frame) 
{
	if (frame->data.d_data == NULL)
		return(OK);
;
	switch(frame->data_type) {
		case SHORT :
		case INT :
		case LONG :
		case FLOAT :
		case DOUBLE :
			free((char *)frame->data.d_data);
			break;
	}
	frame->data.d_data = 0x0;
	free((char *)frame->quality);
	frame->quality = 0x0;
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       alloc_new_desc()
!							
!.purp          allocate memory space for user descriptors
!
!.desc
! alloc_new_desc(pt_file, data_type, nb_values) 
!
! Anyfile *pt_file;     file structure pointer
! short data_type;      data type
! int nb_val;           number of occurence of data
!.ed
-------------------------------------------------------------------- */
int 
alloc_new_desc(Anyfile *pt_file, short data_type, int nb_val) 
{
	Descr_Items *desc_items;
	Descriptor *descr;
	int index;

	if (pt_file->external_info == NULL)
              pt_file->external_info = (void *)calloc(1,sizeof(Descr_Items));
	desc_items = (Descr_Items *)(pt_file->external_info);
	if (desc_items->descr_list == NULL) {
		index = 0;
		desc_items->descr_list = (Descriptor *)malloc(sizeof(Descriptor));
	}
	else {
		index = desc_items->nb_descr;
		desc_items->descr_list = (Descriptor *)realloc(
			(char *)desc_items->descr_list,(index+1)*sizeof(Descriptor));
	}
	descr = desc_items->descr_list+index;
	descr->data_type = data_type;
	descr->nb_values = nb_val;
	descr->descr_name = (char *)calloc(FLEN_KEYWORD+1,sizeof(char));
	if (data_type == CHAR)
		descr->descr_value.c_data = 
			(char *)calloc(nb_val+1,sizeof_item(data_type));
	else
		descr->descr_value.c_data = 
			(char *)calloc(nb_val,sizeof_item(data_type));
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       free_all_desc()
!							
!.purp          free memory space allocated for user descriptors
!
!.desc
! free_all_desc(pt_file);
!
! Anyfile *pt_file;     file structure
!.ed
-------------------------------------------------------------------- */
int 
free_all_desc(Anyfile *pt_file)
{
	int i;
	Descr_Items *desc_items;
	Descriptor descr;

	if (pt_file->external_info == 0) /* no descriptors */
      	return(OK);

	desc_items = (Descr_Items *)(pt_file->external_info);
	for (i=0; i < desc_items->nb_descr; i++) {
		descr = desc_items->descr_list[i];
		free(descr.descr_name);
		free(descr.descr_value.c_data);
	}
	free(desc_items->descr_list);
	free(desc_items);
	return(OK);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                read_file_class      
!
!.purp        read FCLASS descriptor in the frame for file class
!
!.desc
! int read_file_class(anyfile)
!
! void *anyfile;        file structure
!
!.ed
--------------------------------------------------------------------*/
int read_file_class(void *anyfile)
{
    Anyfile *fd;
    int type, status;
    char text[132];

    disable_user_warnings();
    
    status = RD_desc(anyfile,"SFCLASS",INT,1,&type);
    if (status >= 0) {
      if (type > 0) {
        type = SUPER_CLASS;
        restore_user_warnings();
        fd = (Anyfile *)anyfile;
        sprintf(text,"You are overriding class type for file %s: "
	      "unexpected results may occur.",fd->name);
        print_warning(text);
        return type;
      }
    }
    status = RD_desc(anyfile,"FCLASS|OAFCLASS",INT,1,&type);
    if (status < 0)
      type = 0;

    restore_user_warnings();

    return type;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                set_super_class      
!
!.purp        write true in SFCLASS descriptor in the frame 
!
!.desc
! int set_super_class(anyfile)
!
! void *anyfile;        file structure
!
!.ed
--------------------------------------------------------------------*/
int set_super_class(void *anyfile)
{
    int one=1;

    disable_user_warnings();
    WR_desc(anyfile,"SFCLASS",INT,1,&one);
    restore_user_warnings();
    return(0);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                unset_super_class      
!
!.purp        write false in SFCLASS descriptor in the frame 
!
!.desc
! int unset_super_class(anyfile)
!
! void *anyfile;        file structure
!
!.ed
--------------------------------------------------------------------*/
int unset_super_class(void *anyfile)
{
    int zero=0;

    disable_user_warnings();
    WR_desc(anyfile,"SFCLASS",INT,1,&zero);
    restore_user_warnings();
    return(0);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                write_file_class      
!
!.purp        write FCLASS descriptor in the frame for type description
!
!.desc
! int write_file_class(anyfile,type)
!
! void *anyfile;        file structure
!
!.ed
--------------------------------------------------------------------*/
int write_file_class(void *anyfile, int type)
{
    int code, status=0;
    code = type;
    disable_user_warnings();
    
    if (code == SUPER_CLASS) 
	status = set_super_class(anyfile);
    else
    	status = WR_desc(anyfile,"FCLASS",INT,1,&code);
    restore_user_warnings();
    return(status);
}

#ifdef FITS

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       fits_bitpix()
!
!.purp          returns the FITS code for element size
!
!.desc
! int fits_bitpix(data_type)
!
! short data_type;      required data type
!
!.ed
--------------------------------------------------------------------*/
int 
fits_bitpix(short data_type)
{
	switch (data_type) {
	case CHAR :
		return(BYTE_IMG);
		break;
	case USHORT :
		return(USHORT_IMG);
		break;
	case SHORT :
		return(SHORT_IMG);
		break;
	case INT :
	case LONG :
		return(LONG_IMG);
		break;
	case FLOAT :
		return(FLOAT_IMG);
		break;
	case DOUBLE :
		return(DOUBLE_IMG);
		break;
	}
	return(ERR_BAD_TYPE);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       fits_datatype()
!
!.purp        returns the FITS datatype according to bitpix value
!
!.desc
! int fits_datatype(bitpix)
!
! int bitpix;           required data type
!
!.ed
--------------------------------------------------------------------*/
int 
fits_datatype(int bitpix)
{
	switch (bitpix) {
	case BYTE_IMG :
		return(TBYTE);
		break;
	case SHORT_IMG :
		return(TSHORT);
		break;
	case USHORT_IMG :
		return(TUSHORT);
		break;
	case LONG_IMG :
		return(TLONG);
		break;
	case FLOAT_IMG :
		return(TFLOAT);
		break;
	case DOUBLE_IMG :
		return(TDOUBLE);
		break;
	}
	return(ERR_BAD_TYPE);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                       fits_non_std_desc()
!
!.purp        returns true if card is a not a standard FITS decsriptor
!
!.desc
! int fits_non_std_desc(card)
!
! char *card;           FITS card to check
!
!.ed
--------------------------------------------------------------------*/
int 
fits_non_std_desc(char *card)
{
	switch (card[0]) {
		case 'B' :
		if (strncmp(card,"BITPIX",6) == 0)
			return(0);
		if (strncmp(card,"BSCALE",6) == 0)
			return(0);
		if (strncmp(card,"BZERO",5) == 0)
			return(0);
		break;
		case 'C' :
		if (strncmp(card,"CRPIX",5) == 0)
			return(0);
		if (strncmp(card,"CRVAL",5) == 0)
			return(0);
		if (strncmp(card,"CDELT",5) == 0)
			return(0);
		break;
		case 'E' :
		if (strncmp(card,"EXTEND",6) == 0)
			return(0);
		if (strncmp(card,"EXTNAME",7) == 0)
			return(0);
		if (strncmp(card,"END",3) == 0)
			return(0);
		if (strncmp(card,"EXTNAME",7) == 0)
		break;
		case 'G' :
		if (strncmp(card,"GCOUNT",6) == 0)
			return(0);
		break;
		case 'H' :
		if (strncmp(card,"HISTORY",7) == 0)
			return(0);
		break;
		case 'L' :
		if (strncmp(card,"LHCUTS",6) == 0)
			return(0);
		break;
		case 'N' :
		if (strncmp(card,"NAXIS",5) == 0)
			return(0);
		break;
		case 'P' :
		if (strncmp(card,"PCOUNT",6) == 0)
			return(0);
		break;
		case 'S' :
		if (strncmp(card,"SIMPLE",6) == 0)
			return(0);
		break;
		case 'T' :
		if (strncmp(card,"TFIELDS",7) == 0)
			return(0);
		if (strncmp(card,"TTYPE",5) == 0)
			return(0);
		if (strncmp(card,"TFORM",5) == 0)
			return(0);
		break;
		case 'X' :
		if (strncmp(card,"XTENSION",8) == 0)
			return(0);
		break;
	}		
	return(1);
}
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!.func                copy_table_desc  
!
!.purp   copy, if it exist, TABLE descriptor from inpiut file to outputfile
!
!.desc
! int copy_table_desc(anyfile_in, anyfile_out)
!
! void *anyfile_in;     file structure
! void *anyfile_out;    file structure
!
!.ed
--------------------------------------------------------------------*/
int copy_table_desc(void *anyfile_in, void *anyfile_out)
{
    int status;
    char name[lg_name+1];

    disable_user_warnings();

    status = RD_desc(anyfile_in, "TABLE", CHAR, lg_name, name);
    if (status > 0) 
      WR_desc(anyfile_out, "TABLE", CHAR, lg_name, name);
    restore_user_warnings();
    return(0);
}
