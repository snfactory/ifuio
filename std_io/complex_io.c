/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! IDENT        complex_io.c
! LANGUAGE     C
! AUTHOR       P.FERRUIT
! KEYWORDS
! PURPOSE      Library of to read/write COMPLEX images into
!              two images (one for the real part, another
!              one for the imaginary)
!              
! COMMENT
! VERSION      1.0  2002, Mar 28 PF Creation + test
!                       -   create_COMPLEX_IMAGE2D()
!                       -   free_COMPLEX_IMAGE2D_mem()
!                       -   close_COMPLEX_IMAGE2D()
!                       -   open_COMPLEX_IMAGE2D()
!              1.1  04/05/2003 PF Modification
!                       -   alloc_COMPLEX_IMAGE2D_memMemory() (no test)
!                       -   save_COMPLEX_IMAGE2D() (no test)
---------------------------------------------------------------------*/

#include <stdlib.h>
#include        <IFU_io.h>
#include        <data_io.h>

extern IO_Format OutputIO;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk             I/O ROUTINES FOR THE COMPLEX IMAGES
!
----------------------------------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  alloc_COMPLEX_IMAGE2D_memMemory()
!
!.purp	Allocate the data area of
!       a complex image. The structure of the complex
!       image must have been initialized prior any
!       call to this routine (i.e. we need to know
!       the size of the arrays...)
!
!.desc
!
!int alloc_COMPLEX_IMAGE2D_mem(COMPLEX_IMAGE2D *frame)
!
!COMPLEX_IMAGE2D  *frame;     COMPLEX_IMAGE2D structure
!
!.ed
----------------------------------------------------------------------*/

int
alloc_COMPLEX_IMAGE2D_memMem (COMPLEX_IMAGE2D * frame)
{
  frame->data =
    (double *) calloc (2 * frame->nx * frame->ny, sizeof (double));

  if (frame->data == NULL)
    {
      print_error ("In alloc_COMPLEX_IMAGE2D_memMem:");
      print_error ("Unable to allocate memory for complex data of frame %s",
		   frame->name);
      print_error ("Size to be allocated : 2 * %d * %d * sizeof(double)",
		   frame->nx, frame->ny);
      exit_session (-1);
    }

  return (0);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  free_COMPLEX_IMAGE2D_mem()
!
!.purp	Free the memory allocated for the data area of
!       a complex image.
!
!.desc
!
!int free_COMPLEX_IMAGE2D_mem(COMPLEX_IMAGE2D *frame)
!
!COMPLEX_IMAGE2D  *frame;     COMPLEX_IMAGE2D structure
!
!.ed
----------------------------------------------------------------------*/

int
free_COMPLEX_IMAGE2D_mem (COMPLEX_IMAGE2D * frame)
{
  if (frame->data == NULL)
    {
      return (OK);
    }

  free ((char *) frame->data);
  frame->data = NULL;
  return (OK);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  create_COMPLEX_IMAGE2D()
!
!.purp	Fills a COMPLEX_IMAGE2D structure with the relevant information
!       and allocates the memory for the data. Returns 0 if everything
!       went well, exits otherwise.
!       As this is a creation, the imno field is set to -1 (it will
!       be used by close_COMPLEX_IMAGE2D to discriminate between opening
!       and creation).
!
!.desc
!
!int create_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *frame, char *name, int *npix,
!         double *start, double *step,
!         char *ident, char *unit)
!
!COMPLEX_IMAGE2D  *frame;     input structure to be inialized
!char          *name;      generic name (including the path) that
!                          will be used to create the names of
!                          the real and imaginary part images.
!int           *npix;      array of dimension 2 containing the
!                          number of pixels along the x- and y-axes
!double        *start;     array of dimension 2 containing the
!                          start values for the x- and y-axes
!double        *step;      array of dimension 2 containing the
!                          step values for the x- and y-axes
!double        *ident;     optional ident string for the frame
!double        *unit;      optional unit string for the frame
!
!.ed
----------------------------------------------------------------------*/

int
create_COMPLEX_IMAGE2D (COMPLEX_IMAGE2D * frame, char *name, int *npix,
		    double *start, double *step, char *ident, char *unit)
{
  char filename[lg_name + 1];
  char *pt_filename, error_txt[257];
  int length;

/* ============================================================================ */
/* Path and filenames for the images (real and imag suffixes)                   */
/* ============================================================================ */

/* ---------------------------------------------------------------------------- */
/* Filling the name field                                                       */
/* (which is actually not used as the actual names of the two frames will       */
/* be stored in the name_real and name_imag fields of the structure             */
/* This field is kept to ensure compatibility with the Anyfile structure of the */
/* io library.                                                                  */
/* ---------------------------------------------------------------------------- */
  strcpy (frame->name, name);
  strcpy (filename, frame->name);
  first_blk (filename);
  append_ima_extension (frame->name, OutputIO.basic_io);

/* ---------------------------------------------------------------------------- */
/* Filling the other fields of the structure                                    */
/* ---------------------------------------------------------------------------- */

  frame->imno = -1;
  frame->ident[0] = '\0';
  frame->history[0] = '\0';
  strcpy (frame->cunit, "None given");
  frame->startx = start[0];
  frame->starty = start[1];
  frame->stepx = step[0];
  frame->stepy = step[1];
  frame->endx = start[0] + (npix[0] - 1) * step[0];
  frame->endy = start[1] + (npix[1] - 1) * step[1];

  frame->nx = npix[0];
  frame->ny = npix[1];
  frame->iomode = (int) O_MODE;
  frame->file_type = T_IMA2D;
  frame->data_format = OutputIO.basic_io;
  frame->external_info = 0;

  if (ident != NULL) {
      strncpy (frame->ident, ident, lg_ident);
      frame->ident[lg_ident] = '\0';
  }
  if (unit != NULL)
  {
      strncpy (frame->cunit, unit, lg_unit);
      frame->cunit[lg_unit] = '\0';
  }

/* ---------------------------------------------------------------------------- */
/* Allocating memory for the complex data                                       */
/* ---------------------------------------------------------------------------- */

  alloc_COMPLEX_IMAGE2D_memMem(frame);
  return (0);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  close_COMPLEX_IMAGE2D()
!
!.purp	Writes the contents of a complex image into its two
!       real and imaginary frames. Free the memory allocated
!       for the data.
!       Note that nothing is done if the I/O mode of the
!       complex image is I_MODE.
!
!.desc
!
!int close_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *frame)
!
!COMPLEX_IMAGE2D  *frame;     COMPLEX_IMAGE2D structure
!
!.ed
----------------------------------------------------------------------*/

int
close_COMPLEX_IMAGE2D (COMPLEX_IMAGE2D * frame)
{
  double start[2], step[2];
  int npix[2];
  char error_txt[256];
  char realname[lg_name + 1], imagname[lg_name + 1];
  IMAGE2D realframe, imagframe;
  long i, j;


/* ================================================================== */
/* INPUT MODE, WE SHOULD NOT CHANGE ANYTHING...                       */
/* ================================================================== */

  if (frame->iomode == I_MODE)
    {
      free_COMPLEX_IMAGE2D_mem (frame);
      return (OK);
    }

  sprintf(realname,"%s[%s]",frame->name,CPLX_REAL_EXT);
  sprintf(imagname,"%s[%s]",frame->name,CPLX_IMAG_EXT);

/* ================================================================== */
/* If imno = -1 this is a creation, creating the 2D frames,           */
/* else, this is an open, opening the 2D frames                        */
/* ================================================================== */
  if (frame->imno == -1)
    {
      start[0] = frame->startx;
      start[1] = frame->starty;
      step[0] = frame->stepx;
      step[1] = frame->stepy;
      npix[0] = frame->nx;
      npix[1] = frame->ny;
      create_frame (&realframe, realname, npix, start, step,
		    DOUBLE, frame->ident, frame->cunit);
      create_frame (&imagframe, imagname, npix, start, step,
		    DOUBLE, frame->ident, frame->cunit);
    }
  else
    {
      open_frame (&realframe, realname, "IO");
      open_frame (&imagframe, imagname, "IO");

      if (DEBUG) {

      /* ---------------------------------------------------------------- */
      /* Paranoid check : as the frame were not maintained open, they     */
      /* could have been changed in the meantime...                       */
      /* ---------------------------------------------------------------- */
      if ((frame->nx != imagframe.nx) || (realframe.nx != imagframe.nx)
	  || (frame->ny != imagframe.ny) || (realframe.ny != imagframe.ny))
	{
	  print_error
	    ("The complex, real and imaginary frames do not have consistent numbers of pixels.");
	  sprintf (error_txt, "Complex frame : %s -- nx = %d , ny = %d",
		   frame->name, frame->nx, frame->ny);
	  print_error (error_txt);
	  sprintf (error_txt, "Real frame    : %s -- nx = %d , ny = %d",
		   realframe.name, realframe.nx, realframe.ny);
	  print_error (error_txt);
	  sprintf (error_txt, "Imag frame    : %s -- nx = %d , ny = %d",
		   imagframe.name, imagframe.nx, imagframe.ny);
	  print_error (error_txt);
	  exit (-1);
	}

      if ((realframe.stepx != frame->stepx)
	  || (realframe.stepx != imagframe.stepx)
	  || (realframe.stepy != frame->stepy)
	  || (realframe.stepy != imagframe.stepy))
	{
	  print_error
	    ("The real and imaginary frame do not have consistent step sizes.");
	  sprintf (error_txt, "Complex frame : %s -- stepx = %e , stepy = %e",
		   frame->name, frame->stepx, frame->stepy);
	  print_error (error_txt);
	  sprintf (error_txt, "Real frame    : %s -- stepx = %e , stepy = %e",
		   realframe.name, realframe.stepx, realframe.stepy);
	  print_error (error_txt);
	  sprintf (error_txt, "Imag frame    : %s -- stepx = %e , stepy = %e",
		   imagframe.name, imagframe.stepx, imagframe.stepy);
	  print_error (error_txt);
	  exit (-1);
	}

      if ((realframe.startx != frame->startx)
	  || (realframe.startx != imagframe.startx)
	  || (realframe.starty != frame->starty)
	  || (realframe.starty != imagframe.starty))
	{
	  print_error
	    ("The real and imaginary frame do not have consistent start values.");
	  sprintf (error_txt,
		   "Complex frame : %s -- startx = %e , starty = %e",
		   frame->name, frame->startx, frame->starty);
	  print_error (error_txt);
	  sprintf (error_txt,
		   "Real frame    : %s -- startx = %e , starty = %e",
		   realframe.name, realframe.startx, realframe.starty);
	  print_error (error_txt);
	  sprintf (error_txt,
		   "Imag frame    : %s -- startx = %e , starty = %e",
		   imagframe.name, imagframe.startx, imagframe.starty);
	  print_error (error_txt);
	  exit (-1);
	}
      } /* end DEBUG */
    }

/* ================================================================== */
/* Filling the data areas                                             */
/* ================================================================== */

    for (j = 0; j < frame->ny; j++)
    {
  	for (i = 0; i < frame->nx; i++)
	{
	  WR_frame (&realframe, i, j, RD_REAL_cplxima (frame, i, j));
	  WR_frame (&imagframe, i, j, RD_IMAG_cplxima (frame, i, j));
	}
    }

/* ================================================================== */
/* CLOSING THE 2D FRAMES AND FREEING MEMORY                           */
/* ================================================================== */

  close_frame (&realframe);
  close_frame (&imagframe);
  free_COMPLEX_IMAGE2D_mem (frame);


  return (OK);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  save_COMPLEX_IMAGE2D()
!
!.purp	Writes the contents of a complex image into its two
!       real and imaginary frames. Free the memory allocated
!       for the data. Note that contrary to the close routine,
!       any existing file is overwritten.
!       This routine will exit with an error if the complex
!       image is opened read-only.
!
!.desc
!
!int save_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *frame)
!
!COMPLEX_IMAGE2D  *frame;     COMPLEX_IMAGE2D structure
!
!.ed
----------------------------------------------------------------------*/

int
save_COMPLEX_IMAGE2D (COMPLEX_IMAGE2D * frame)
{
  char filename_real[lg_name + 1], filename_imag[lg_name + 1];
  double start[2], step[2];
  int npix[2], tempASK;
  IMAGE2D realframe, imagframe;
  long i, j;


/* ================================================================== */
/* INPUT MODE, NOT ALLOWED                                            */
/* ================================================================== */

  if (frame->iomode == I_MODE)
    {
      print_error ("In save_COMPLEX_IMAGE2D:");
      print_error ("Complex image is opened read-only (%s).", frame->name);
      print_error ("Unauthorized operation.");
      exit_session (-1);
    }

/* ================================================================== */
/* Creating the images (any existing image with the same name will    */
/* be overwritten                                                     */
/* - disabling the overwrite protection and enabling it again once    */
/* the frames have been created                                       */
/* ================================================================== */
  start[0] = frame->startx;
  start[1] = frame->starty;
  step[0] = frame->stepx;
  step[1] = frame->stepy;
  npix[0] = frame->nx;
  npix[1] = frame->ny;
  tempASK = ASK;
  ASK = (int) 0;

  sprintf(filename_real,"%s[%s]",frame->name,CPLX_REAL_EXT);
  sprintf(filename_imag,"%s[%s]",frame->name,CPLX_IMAG_EXT);

  create_frame (&realframe, filename_real, npix, start, step,
		DOUBLE, frame->ident, frame->cunit);
  create_frame (&imagframe, filename_imag, npix, start, step,
		DOUBLE, frame->ident, frame->cunit);
  ASK = tempASK;

/* ================================================================== */
/* Filling the data areas                                             */
/* ================================================================== */

    for (j = 0; j < frame->ny; j++)
    {
  	for (i = 0; i < frame->nx; i++)
	{
	  WR_frame (&realframe, i, j, RD_REAL_cplxima (frame, i, j));
	  WR_frame (&imagframe, i, j, RD_IMAG_cplxima (frame, i, j));
	}
    }

/* ================================================================== */
/* CLOSING THE 2D FRAMES AND FREEING MEMORY                           */
/* ================================================================== */

  close_frame (&realframe);
  close_frame (&imagframe);
  free_COMPLEX_IMAGE2D_mem (frame);


  return (OK);
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func  open_COMPLEX_IMAGE2D()
!
!.purp	Opens the real and imaginary frames constituting
!       a complex image and fills the COMPLEX_IMAGE2D structure.
!       The two frames are closed at the end of the process.
!       The two images MUST have identical numbers of pixels,
!       as well as start and step values.
!       The ident and units are taken from the real part frame.
!
!.desc
!
!int open_COMPLEX_IMAGE2D(COMPLEX_IMAGE2D *frame, char *name, char *mode)
!
!COMPLEX_IMAGE2D  *frame;     input structure to be inialized
!char          *name;      generic name (including the path) that
!                          will be used to create the names of
!                          the real and imaginary part images.
!char          *mode;      input/output mode, "Input", "Output"
!                          or "IO".
!
!.ed
----------------------------------------------------------------------*/

int
open_COMPLEX_IMAGE2D (COMPLEX_IMAGE2D * frame, char *name, char *mode)
{
  char filename[lg_name + 1], filename_real[lg_name + 1],
    filename_imag[lg_name + 1];
  char *pt_filename, error_txt[257];
  int length;
  long i, j;
  IMAGE2D realframe, imagframe;

/* ============================================================================ */
/* Path and filenames for the images (real and imag suffixes)                   */
/* ============================================================================ */

/* ---------------------------------------------------------------------------- */
/* Filling the name field                                                       */
/* (which is actually not used as the actual names of the two frames will       */
/* be stored in the name_real and name_imag fields of the structure             */
/* This field is kept to ensure compatibility with the Anyfile structure of the */
/* io library.                                                                  */
/* ---------------------------------------------------------------------------- */
  strcpy (frame->name, name);
  strcpy (filename, frame->name);
  first_blk (filename);
  append_ima_extension (frame->name, OutputIO.basic_io);

/* ---------------------------------------------------------------------------- */
/* Filling the name_real and name_imag fields                                   */
/* ---------------------------------------------------------------------------- */
  sprintf(filename_real,"%s[%s]",frame->name,CPLX_REAL_EXT);
  sprintf(filename_imag,"%s[%s]",frame->name,CPLX_IMAG_EXT);

/* ---------------------------------------------------------------------------- */
/* Opening the input files and checking they are consistent                     */
/* ---------------------------------------------------------------------------- */
  open_frame (&realframe, filename_real, mode);
  open_frame (&imagframe, filename_imag, mode);

  if ((realframe.nx != imagframe.nx) || (realframe.nx != imagframe.nx))
    {
      print_error
	("The real and imaginary frame do not have consistent numbers of pixels.");
      sprintf (error_txt, "Real frame    : %s -- nx = %d , ny = %d",
	       realframe.name, realframe.nx, realframe.ny);
      print_error (error_txt);
      sprintf (error_txt, "Imag frame    : %s -- nx = %d , ny = %d",
	       imagframe.name, imagframe.nx, imagframe.ny);
      print_error (error_txt);
      exit (-1);
    }

  if ((realframe.stepx != imagframe.stepx)
      || (realframe.stepy != imagframe.stepy))
    {
      print_error
	("The real and imaginary frame do not have consistent step sizes.");
      sprintf (error_txt, "Real frame    : %s -- stepx = %e , stepy = %e",
	       realframe.name, realframe.stepx, realframe.stepy);
      print_error (error_txt);
      sprintf (error_txt, "Imag frame    : %s -- stepx = %e , stepy = %e",
	       imagframe.name, imagframe.stepx, imagframe.stepy);
      print_error (error_txt);
      exit (-1);
    }

  if ((realframe.startx != imagframe.startx)
      || (realframe.starty != imagframe.starty))
    {
      print_error
	("The real and imaginary frame do not have consistent start values.");
      sprintf (error_txt, "Real frame    : %s -- startx = %e , starty = %e",
	       realframe.name, realframe.startx, realframe.starty);
      print_error (error_txt);
      sprintf (error_txt, "Imag frame    : %s -- startx = %e , starty = %e",
	       imagframe.name, imagframe.startx, imagframe.starty);
      print_error (error_txt);
      exit (-1);
    }

/* ---------------------------------------------------------------------------- */
/* Setting up the I/O mode flag                                                 */
/* ---------------------------------------------------------------------------- */

  switch (mode[0])
    {
    case 'I':
      if (mode[1] == 'O')
	frame->iomode = (int) IO_MODE;
      else
	frame->iomode = (int) I_MODE;
      break;
    case 'O':
      frame->iomode = (int) O_MODE;
      break;
    default:
      frame->iomode = (int) I_MODE;
      break;
    }

/* ---------------------------------------------------------------------------- */
/* Filling the other fields of the structure                                    */
/* ---------------------------------------------------------------------------- */

/* Opening the files, imno is set to 0 (instead of -1 for a creation)           */
  frame->imno = 0;

  strncpy (frame->ident, realframe.ident, lg_ident + 1);
  strncpy (frame->cunit, realframe.cunit, lg_unit + 1);
  strncpy (frame->history, realframe.history, lg_hist + 1);
  (frame->ident)[lg_ident] = '\0';
  (frame->cunit)[lg_unit] = '\0';
  (frame->history)[lg_hist] = '\0';
  frame->startx = realframe.startx;
  frame->starty = realframe.starty;
  frame->stepx = realframe.stepx;
  frame->stepy = realframe.stepy;
  frame->nx = realframe.nx;
  frame->ny = realframe.ny;
  frame->endx = realframe.endx;
  frame->endy = realframe.endy;
  frame->file_type = T_IMA2D;
  frame->data_format = realframe.data_format;
  frame->external_info = 0;	/* NOT IMPLEMENTED */



/* ---------------------------------------------------------------------------- */
/* Allocating memory for the complex data                                       */
/* ---------------------------------------------------------------------------- */

  frame->data = (double *) calloc (2 * frame->nx * frame->ny, sizeof (double));

  if (frame->data == NULL)
    {
      sprintf (error_txt,
	       "Unable to allocate memory for complex data of frame %s",
	       frame->name);
      print_error (error_txt);
      sprintf (error_txt,
	       "Size to be allocated : 2 * %d * %d * sizeof(double)",
	       frame->nx, frame->ny);
      print_error (error_txt);
      exit (-1);
    }
/* ================================================================== */
/* Filling the data areas                                             */
/* ================================================================== */

    for (j = 0; j < frame->ny; j++)
    {
  	for (i = 0; i < frame->nx; i++)
	{
	  WR_REAL_cplxima (frame, i, j, RD_frame (&realframe, i, j));
	  WR_IMAG_cplxima (frame, i, j, RD_frame (&imagframe, i, j));
	}
    }

/* ================================================================== */
/* CLOSING THE 2D FRAMES                                              */
/* ================================================================== */

  close_frame (&realframe);
  close_frame (&imagframe);

  return (OK);
}
