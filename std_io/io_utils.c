/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        io_utils.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      utilities for I/O
! COMMENT      
! VERSION      4.0  1992-June-15 : Creation AR 
---------------------------------------------------------------------*/

#include <stdlib.h>
#include <IFU_io.h>
#include <stdarg.h>

int wrcr = 0;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk               Routines for LOGGING/PRINTING messages
!
!.func                     print_msg()
!
!.purp           prints a text string on device and logs it
!.desc
! print_msg(format, args)
!
! char *format;         format
! args                  list of variables to be printed
!.ed
--------------------------------------------------------------------*/
int 
print_msg(char *string, ...)
{
   va_list args ;
   char *format;
   
   va_start(args, string);
   format = (char *)malloc((strlen(string) + 20)*sizeof(char));

   if (DEBUG) {
	sprintf(format,"%s\n",string);
	vfprintf(stderr,format,args);
   }

   if (VERBOSE) {
	if(!TK) {
       	    if (wrcr) {
		sprintf(format,"\n%s\n",string);
	 	vprintf(format,args);
       	    }
	    else {
		sprintf(format,"%s\n",string);
	 	vprintf(format,args);
	    }
	    wrcr = 0;
     	}
     	else {
	    sprintf(format,"%s\n",string);
 	    vprintf(format,args);
     	}
     	fflush(stdout);
   }
    
   free(format);
   va_end(args);

   return(0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     print_msg_nocr()
!
!.purp           prints a text string without carriage return on device 
!.desc
! print_msg_nocr(text_string)
!
! char *text_string;    string to be printed
!.ed
--------------------------------------------------------------------*/
int 
print_msg_nocr(char *text)
{
    int status;

    status = 0;
    if (VERBOSE) {
      printf("%s",text);
      fflush(stdout);
      wrcr = 1;
    }
    return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     print_label()
!
!.purp           prints a text string on device but with /r option
!.desc
! print_label(text_string)
!
! char *text_string;    string to be printed
!.ed
--------------------------------------------------------------------*/
int 
print_label(char *text)
{
   int status=0;

   if(!TK) {
       if (VERBOSE) {
	   printf("\r%s",text);
	   fflush(stdout);
	   wrcr = 1;
	 }
	
       if (DEBUG) fprintf(stderr,"%s\n",text);
   }
   else {
     if(VERBOSE) {
       printf("@ L \"%s\"\n",text);
       fflush(stdout);
     }
   }
 
   return(status);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     print_progress()
!
!.purp           Print a percent gauge on the screen or on TK
!.desc
! print_progress(text,pourcent)
!
! char *text;           string to be printed
! float pourcent;       pourcent of running job
!.ed
--------------------------------------------------------------------*/
int print_progress(char *text,float pourcent,float step)
{
  static float last = 0.0;

  if(step<0)
	{
	 last=0.0;
	 return 0;
	}

  if((pourcent-last)>=step) 
  {
    if(TK)
      {
	printf("@ P \"%s\" %4.1f\n",text,pourcent);
	fflush(stdout);
      }
    else
      {
	if(VERBOSE)
	  {	
	    printf("\r%s %4.1f%%",text,pourcent);
	    fflush(stdout);
	    wrcr = 1;
	  }
      }
    last=pourcent; 
  }
  else
    if (pourcent>=100.0)
      {
	if(TK)
	  {
	    printf("@ P \"%s\" %4.1f\n",text,100.0);
	    fflush(stdout);
	  }
    	else
	  {
	    if(VERBOSE)
	      {	
		printf("\r%s %4.1f%%",text,100.0);
		fflush(stdout);
		wrcr = 1;
	      }
	  }
	last=100.0;
      }	 
   return(0);
}
int reset_print_progress()
{
  return(print_progress("", 0, -1.0));
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     print_warning()
!
!.purp           Print a warning message under TK
!.desc
! print_warning(format,args)
!
! char *format;         format
! args                  list of variables to be printed
! 
!.ed
--------------------------------------------------------------------*/
int 
print_warning(char *string, ...)
{
   va_list args ;
   char *format;
   
   va_start(args, string);
   format = (char *)malloc((strlen(string) + 20)*sizeof(char));

   if (DEBUG) {
	sprintf(format,"%s\n",string);
	vfprintf(stderr,format,args);
   }

   if(TK) {
   	sprintf(format,"@ W \"%s\"\n",string);
	vprintf(format,args);
   }
   else	{
	if(VERBOSE) {
      	    if (wrcr) {
       		sprintf(format,"\nWARNING: %s\n",string);
		vprintf(format,args);
	    }
	    else {
       		sprintf(format,"WARNING: %s\n",string);
		vprintf(format,args);
	    }
      	    wrcr = 0;
   	}
   }
   free(format);
   va_end(args);
   return(0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                     print_error()
!
!.purp           Print a error message under TK
!.desc
! print_error(format,args)
!
! char *format;         format
! args                  list of variables to be printed
! 
!.ed
--------------------------------------------------------------------*/
int 
print_error(char *string, ...)
{
   va_list args ;
   char *format;
   
   va_start(args, string);
   format = (char *)malloc((strlen(string) + 20)*sizeof(char));

   if (DEBUG) {
	sprintf(format,"%s\n",string);
	vfprintf(stderr,format,args);
   }

   if(TK) {
   	sprintf(format,"@ R \"%s\"\n",string);
	vprintf(format,args);
   }
   else	{
     if(VERBOSE) {  /* on error always print error message ! */
      	    if (wrcr) {
       		sprintf(format,"\nERROR: %s\n",string);
		vprintf(format,args);
	    }
	    else {
       		sprintf(format,"ERROR: %s\n",string);
		vprintf(format,args);
	    }
      	    wrcr = 0;
     } 
   }
   free(format);
   va_end(args);
   return(0);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	Transformations between pixels and word coordinates

     	      (transformation for spectra are described in funcdef.h)
!							
!.func                       coord_spec()
!							
!.purp          transformation in world coordinates for a spectrum
!
!.desc
! float world_coord = coord_spec(spectrum,pixel) 
!
! SPECTRUM *spectrum;   spectrum structure
! int pixel;            pixel
!.ed								
!							
!.func                       pixel_spec()
!							
!.purp          transformation in pixels for a spectrum
!
!.desc
! int pixel = pixel_spec(spectrum,world_coord) 
!
! SPECTRUM *spectrum;   spectrum structure
! float world_coord;    world coordinate
!.ed								
!							
!.func                       coord_frame()
!							
!.purp          transformation in world coordinates for a frame
!
!.desc
! void coord_frame(image,pixel_x,pixel_y,*world_x,*world_y) 
!
! IMAGE2D *image;       image structure
! int pixel_x;          pixel along x axis
! int pixel_y;          pixel along y axis
! float *world_x;       coordinate along x axis
! float *world_y;       coordinate along y axis
!.ed								
--------------------------------------------------------------------*/

void 
coord_frame(IMAGE2D *frame,int pixel_x,int pixel_y, float *x, float *y)
{
	*x = frame->startx + pixel_x * frame->stepx;
	*y = frame->starty + pixel_y * frame->stepy;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       pixel_frame()
!							
!.purp          transformation in pixels for a frame
!
!.desc
! void pixel_frame(frame,world_x,world_y,pixel_x,pixel_y)
!
! IMAGE2D *image;       image structure
! float world_x;        coordinate along x axis
! float world_y;        coordinate along y axis
! int *pixel_x;         pixel along x axis
! int *pixel_y;         pixel along y axis
!.ed								
-------------------------------------------------------------------- */

void 
pixel_frame(IMAGE2D *frame, float x, float y, int *pixel_x, int *pixel_y)
{
	*pixel_x = 0.5 + (x - frame->startx)/frame->stepx;
	*pixel_y = 0.5 + (y - frame->starty)/frame->stepy;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       in_spectrum()
!							
!.purp          checks if pixel of coordinates (x) is inside spectrum
!
!.desc
! int in_spectrum(frame,world_x)
!
! IMAGE2D *image;       image structure
! float world_x;        coordinate along x axis
!.ed								
-------------------------------------------------------------------- */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!							
!.func                       in_frame()
!							
!.purp          checks if pixel of coordinates (x,y) is inside frame
!
!.desc
! int in_frame(frame,world_x,world_y)
!
! IMAGE2D *image;       image structure
! float world_x;        coordinate along x axis
! float world_y;        coordinate along y axis
!.ed								
-------------------------------------------------------------------- */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	        Working on sub-domain of SPECTRA
!							
!.func                       set_subspec()
!							
!.purp                sets spectrum working bounds
!
!.desc
! int set_subspec(spectrum,binf,bsup) 
!
! SPECTRUM *spectrum;   spectrum structure
! double binf;          lower bound
! double bsup;          upper bound
!.ed
-------------------------------------------------------------------- */

int 
set_subspec(SPECTRUM *spectrum,double binf,double bsup)
{
	char errtext[132];
	
	if (binf > bsup ) {		
		sprintf(errtext,"set_subspec: spec %s",spectrum->name);
		Handle_Error(errtext,ERR_IMA_BOUND);
		return(ERR_IMA_BOUND);
	}	
	if (binf < (spectrum->start - (spectrum->step/2))) {
		sprintf(errtext,"set_subspec: spec %s",spectrum->name);
		Handle_Error(errtext,ERR_IMA_BOUND);
		return(ERR_IMA_BOUND);
	}	
	if (bsup > (spectrum->end + (spectrum->step/2))) {
		sprintf(errtext,"set_subspec: spec %s",spectrum->name);
		Handle_Error(errtext,ERR_IMA_BOUND);
		return(ERR_IMA_BOUND);
	}
	/* Redundant with 1st test ??? YC */
	/* if ((binf > bsup )) {		
	   sprintf(errtext,"set_subspec: spec %s",spectrum->name);
	   Handle_Error(errtext,ERR_IMA_BOUND);
	   return(ERR_IMA_BOUND);
	} */
	spectrum->iwstart = (int)(((binf - spectrum->start) / spectrum->step)
	 	                   + 0.5);
	spectrum->iwend = (int)(((bsup - spectrum->start) / spectrum->step)
		                   + 0.5);
	spectrum->wstart = spectrum->start +
		                  spectrum->iwstart*spectrum->step;
	spectrum->wend = spectrum->start +
		                spectrum->iwend*spectrum->step;
	return(OK);
}		
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func                       spec_minmax()
!							
!.purp       finds spectrum min and max values in the current (sub-)domain
!
!.desc
! spec_minmax(spectrum) 
!
! SPECTRUM *spectrum;   spectrum structure
!.ed
-------------------------------------------------------------------- */

int 
spec_minmax(SPECTRUM *spectrum)	
{
	long i;
	
	spectrum->wmin = MAXFLOAT;
	spectrum->wmax = -MAXFLOAT;
	switch(spectrum->data_type) {
	case SHORT :
		for (i=spectrum->iwstart;i<=spectrum->iwend;i++){
			if (spectrum->data.s_data[i] < spectrum->wmin) 
					spectrum->wmin= (float)spectrum->data.s_data[i];
			if (spectrum->data.s_data[i] > spectrum->wmax) 
					spectrum->wmax= (float)spectrum->data.s_data[i];
		}
		break;

	case LONG :
		for (i=spectrum->iwstart;i<=spectrum->iwend;i++){
			if (spectrum->data.l_data[i] < spectrum->wmin) 
					spectrum->wmin= (float)spectrum->data.l_data[i];
			if (spectrum->data.l_data[i] > spectrum->wmax) 
					spectrum->wmax= (float)spectrum->data.l_data[i];
		}
		break;

	case FLOAT :
		for (i=spectrum->iwstart;i<=spectrum->iwend;i++){
			if (spectrum->data.f_data[i] < spectrum->wmin) 
					spectrum->wmin= (float)spectrum->data.f_data[i];
			if (spectrum->data.f_data[i] > spectrum->wmax) 
					spectrum->wmax= (float)spectrum->data.f_data[i];
		}
		break;

	case DOUBLE :
		for (i=spectrum->iwstart;i<=spectrum->iwend;i++){
			if (spectrum->data.d_data[i] < spectrum->wmin) 
					spectrum->wmin= (float)spectrum->data.d_data[i];
			if (spectrum->data.d_data[i] > spectrum->wmax) 
					spectrum->wmax= (float)spectrum->data.d_data[i];
		}
		break;

	default :
		return(ERR_BAD_TYPE);
	}
	return(OK);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func                       inter_spec()
!							
!.purp            sets common domain between two spectra
!
!.desc
! inter_spec(spectrum1,spectrum2) 
!
! SPECTRUM *spectrum1;  spectrum structure
! SPECTRUM *spectrum2;  spectrum structure
!.ed
-------------------------------------------------------------------- */

int 
inter_spec(SPECTRUM *spec1,SPECTRUM *spec2)			
{
	double l1, l2;
	l1 = MAX(spec1->start,spec2->start);
	l2 = MIN(spec1->end,spec2->end);
	if (l1 >= l2) return(ERR_BAD_PARAM);
 	spec1->iwstart = ((l1 - spec1->start) / spec1->step) + 0.5;
	spec1->iwend = ((l2 - spec1->start) / spec1->step) + 0.5;
	spec1->wstart = spec1->start + spec1->iwstart*spec1->step;
	spec1->wend = spec1->start + spec1->iwend*spec1->step;
 	spec2->iwstart = ((l1 - spec2->start) / spec2->step) + 0.5;
	spec2->iwend = ((l2 - spec2->start) / spec2->step) + 0.5;
	spec2->wstart = spec2->start + spec2->iwstart*spec2->step;
	spec2->wend = spec2->start + spec2->iwend*spec2->step;
	return(OK);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	        Working on IMAGES
!							
!.func                       image_minmax()
!							
!.purp                sets image min and max values
!
!.desc
! image_minmax(image) 
!
! IMAGE2D *image;       image structure
!.ed
-------------------------------------------------------------------- */

int 
image_minmax(IMAGE2D *frame) 
{
	int i,j;
	double max, min;
	double intens;

	max=-HUGE; min=HUGE;
	for (j=0;j<frame->ny;j++) {
		for (i=0;i<frame->nx;i++) {
			intens =(double)RD_frame(frame,i,j);
			if (intens > max) max=intens;
			if (intens < min) min=intens;
		}
	}	
	frame->max = max;
	frame->min = min;
	return(OK);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                         flip_frame()
!
!.purp                    flip image (swap columns) 
!
!.desc
! int flip_frame(image)
!
! IMAGE2D  *image;      image structure
! int      flag;        how to flip (TopToBottom or LeftToRight)
!.ed
-----------------------------------------------------------------------------*/
 
int flip_frame(IMAGE2D *image, int flag)
{
  int half_frame, i, i1, j, j1;
  short sval;
  int ival;
  float fval;
  double dval;
  
  if (flag != LeftToRight && flag != TopToBottom) {
    return(ERR_BAD_TYPE);
  }

  switch (flag) {

   case LeftToRight :

    half_frame = (int)(image->nx/2);

    switch (image->data_type) {
      
    case SHORT : 
      for (j=0; j<image->ny; j++) {
	for (i=0; i<half_frame; i++) {
	  i1 = image->nx -1 -i;
	  sval = image->data.s_data[j*image->nx+i];
	  image->data.s_data[j*image->nx+i] = image->data.s_data[j*image->nx+i1];
	  image->data.s_data[j*image->nx+i1] = sval;
	}
      }
      break;
      
    case LONG : 
    case INT : 
      for (j=0; j<image->ny; j++) {
	for (i=0; i<half_frame; i++) {
	  i1 = image->nx -1 -i;
	  ival = image->data.l_data[j*image->nx+i];
	  image->data.l_data[j*image->nx+i] = image->data.l_data[j*image->nx+i1];
	  image->data.l_data[j*image->nx+i1] = ival;
	}
      }
      break;
      
    case FLOAT : 
      for (j=0; j<image->ny; j++) {
	for (i=0; i<half_frame; i++) {
	  i1 = image->nx -1 -i;
	  fval = image->data.f_data[j*image->nx+i];
	  image->data.f_data[j*image->nx+i] = image->data.f_data[j*image->nx+i1];
	  image->data.f_data[j*image->nx+i1] = fval;
	}
      }
      break;
      
    case DOUBLE : 
      for (j=0; j<image->ny; j++) {
	for (i=0; i<half_frame; i++) {
	  i1 = image->nx -1 -i;
	  dval = image->data.d_data[j*image->nx+i];
	  image->data.d_data[j*image->nx+i] = image->data.d_data[j*image->nx+i1];
	  image->data.d_data[j*image->nx+i1] = dval;
	}
      }
      break;
      
    default :
      return(ERR_BAD_TYPE);
    }
    break;

    case TopToBottom :

    half_frame = (int)(image->ny/2);

    switch (image->data_type) {
      
    case SHORT : 
      for (i=0; i<image->nx; i++) {
	for (j=0; j<half_frame; j++) {
	  j1 = image->ny -1 -j;
	  sval = image->data.s_data[j*image->nx+i];
	  image->data.s_data[j*image->nx+i] = image->data.s_data[j1*image->nx+i];
	  image->data.s_data[j1*image->nx+i] = sval;
	}
      }
      break;
      
    case LONG : 
    case INT : 
      for (i=0; i<image->nx; i++) {
	for (j=0; j<half_frame; j++) {
	  j1 = image->ny -1 -j;
	  ival = image->data.l_data[j*image->nx+i];
	  image->data.l_data[j*image->nx+i] = image->data.l_data[j1*image->nx+i];
	  image->data.l_data[j1*image->nx+i] = ival;
	}
      }
      break;
      
    case FLOAT : 
      for (i=0; i<image->nx; i++) {
	for (j=0; j<half_frame; j++) {
	  j1 = image->ny -1 -j;
	  fval = image->data.f_data[j*image->nx+i];
	  image->data.f_data[j*image->nx+i] = image->data.f_data[j1*image->nx+i];
	  image->data.f_data[j1*image->nx+i] = fval;
	}
      }
      break;
      
    case DOUBLE : 
      for (i=0; i<image->nx; i++) {
	for (j=0; j<half_frame; j++) {
	  j1 = image->ny -1 -j;
	  dval = image->data.d_data[j*image->nx+i];
	  image->data.d_data[j*image->nx+i] = image->data.d_data[j1*image->nx+i];
	  image->data.d_data[j1*image->nx+i] = dval;
	}
      }
      break;
      
    default :
      return(ERR_BAD_TYPE);
    }
  }
  return(0);
}              

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	        Working on CUBES
!							
!.func                       cube_minmax()
!							
!.purp                sets cube min and max values
!
!.desc
! cube_minmax(cube) 
!
! IMAGE3D *cube;        cube structure
!.ed
-------------------------------------------------------------------- */

int 
cube_minmax(IMAGE3D *cube) 
{
	int i,j,k;
	double max, min;
	double intens;

	max=-HUGE; min=HUGE;
	for (i=0;i<cube->nx;i++) {
		for (j=0;j<cube->ny;j++) {
			for (k=0;k<cube->nz;k++) {
				intens =(double)RD_cube(cube,i,j,k);
				if (intens > max) max=intens;
				if (intens < min) min=intens;
			}
		}
	}	
	cube->max = max;
	cube->min = min;
	return(OK);
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk          	        Working on TABLES
!							
!
!.func                     search_in_col()
!
!.purp         search for a specific item in a column
!.desc
! search_in_col (table,num,item)
!
! TABLE *table;         table structure
! int  num;             column number
! (type) *item;         item (same type as column type)
!.ed
----------------------------------------------------------------------*/
int 
search_in_col (TABLE *table, int num, void *item)
{
    int  status, i, len, row, nocol=num, nbbytes, type;
    char form[20], unit[lg_unit+1], *pt_items;
    short S_item;
    int I_item;
    float R_item;
    double D_item;

	if (table->row <= 0)
		return(ERR_NODATA);
	
    status = get_col_info(table,nocol,&type,form,unit);
    if (status) {
		status = get_tiger_errcode(table->data_format,status);	
        Handle_Error("Search_in_col",status);
        return(status);
    }
    nbbytes = table->row*sizeof_item((short)type);
	if ((short)type == CHAR) {
        sscanf(form,"A%d",&len);
        nbbytes *= len;
    }
    pt_items = (char *)malloc(nbbytes);
    status = RD_col(table,nocol,pt_items);
    if (status) {
		status = get_tiger_errcode(table->data_format,status);	
        Handle_Error("Search_in_col",status);
        return(status);
    }
    row = -1;
    switch((short)type) {
        case CHAR :
            for (i=0; i<table->row && row <0; i++) {
                if (strcmp(&(pt_items[i*len]),(char *)item) == 0)
					row = i;
            }
            break;
        case SHORT :
            S_item = *((short *)item);
            for (i=0; i<table->row && row <0; i++) {
                if (*((short*)(pt_items)+i) == S_item) row = i;
            }
            break;
        case INT :
        case LONG :
            I_item = *((int *)item);
            for (i=0; i<table->row && row <0; i++) {
                if (*((int*)(pt_items)+i) == I_item) row = i;
            }
            break;
        case FLOAT :
            R_item = *((float *)item);
            for (i=0; i<table->row && row <0; i++) {
                if (*((float*)(pt_items)+i) == R_item) row = i;
            }
            break;
        case DOUBLE :
            D_item = *((double *)item);
            for (i=0; i<table->row && row <0; i++) {
                if (*((double*)(pt_items)+i) == D_item) row = i;
            }
            break;
        default :
            status = -1;
            break;
    }
    free((char *)pt_items);
    return(row);
}

/*--------------------------------------------------------------------
!
!.func                     fits_get_col_width()
!
!.purp          returns column width (FITS vector columns)
!.desc
! get_col_width (table,num)
!
! TABLE *table;         table structure
! int  num;             column number
!.ed
----------------------------------------------------------------------*/
int get_col_width(TABLE *table, int colno) {

    char key[lg_label+1], format[lg_label+1], c;
    int npts;
    
    sprintf(key,"TFORM%d",colno);
    RD_desc(table,key,CHAR,lg_label,format);
    sscanf(format,"%d%c",&npts,&c);
    return(npts);
};

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!           
!.blk                  Routines for CATALOG read/write		
!          
!.func                         RD_catalog()
!
!.purp             successive reads of catalog members
!.desc
! RD_catalog(catalog,filename)
!
! char *catalog;        name of catalog
! char *filename;       name of catalog member
!.ed
----------------------------------------------------------------------*/

int 
RD_catalog( char *catalog, char *filename)
{
	int	 i, status;
	char text[132];
	static FILE **fd_catal = 0;
	static char **nameof_catal = 0;
	static int nb_catal = 0;
	
	i = 0; status = 0;
	filename[0] = '\0';
	if (nb_catal != 0)
		for (i=0;i<nb_catal && (strcmp(catalog,nameof_catal[i]) != 0); i++);
	else {
		fd_catal = (FILE **)malloc(sizeof(FILE *));
		nameof_catal = (char **)malloc(sizeof(char *));
	}
	if (i >= nb_catal) {
		fd_catal=
			(FILE **)realloc((char *)fd_catal,(nb_catal+1)*sizeof(FILE *));
		nameof_catal=
			(char **)realloc((char *)nameof_catal,(nb_catal+1)*sizeof(char *));
		nameof_catal[nb_catal] = (char *)malloc(lg_name*sizeof(char));
		strcpy(nameof_catal[nb_catal],catalog);
		fd_catal[nb_catal] = fopen(catalog,"r");
		if (fd_catal[nb_catal] == NULL) {
			sprintf(text,"RD_catalog %s",catalog);
			Handle_Error(text,ERR_BAD_CAT);
			return(FALSE);		    
		}
		else {
			fgets(text,80,fd_catal[i]); /* get catalog description line */
			nb_catal++;
		}
	}
	if (feof(fd_catal[i])) 
		fclose(fd_catal[i]);
	else {
		fgets(text,80,fd_catal[i]);
		if (feof(fd_catal[i])) {
			fclose(fd_catal[i]);
		}
		else {
			text[first_blk(text)] = '\0';
			strcpy(filename,text);
		}
	}
	if (filename[0] == '\0') {
		nb_catal = 0; i =0;
		return(FALSE);
	}
	return(TRUE);
}
