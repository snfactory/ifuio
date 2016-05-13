/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  2004 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        parse_wcs.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      utilities for parsing WCS coordinates
! COMMENT      
! VERSION      4.0  2004-Oct-04 : Creation AP 
---------------------------------------------------------------------*/

#include <gendef.h>
#include <items.h>

#include <wcslib/wcshdr.h>
#include <wcslib/wcs.h>
#include <fitsio.h>
#include <longnam.h>

#include <error_codes.h>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.blk               Routines for parsing WCS coordinates
!
!.func                     parse_wcs()
!
!.purp           prints a text string on device and logs it
!.desc
! parse_wcs(filename, wcs)
!
! char *filename;       fits file extension
! struct wcsprm *wcs    wcs structure (to be use in further computations)
!.ed
--------------------------------------------------------------------*/

int parse_wcs(void *file) {

   char *header;
   char errtext[132];
   int  nkeys, nreject, relax, status=0;
   fitsfile *fptr;
   Anyfile *anyfile;
   struct wcsprm *wcs;
   int nwcs;

   anyfile = (Anyfile *)file;
   if (fits_open_file(&fptr, anyfile->name, READONLY, &status)) {
      return ERR_OPEN;
   }
   if (fits_hdr2str(fptr, 1, NULL, 0, &header, &nkeys, &status)) {
      return ERR_BAD_PARAM;
   }
   fits_close_file(fptr, &status);

   relax = WCSHDR_all;
   nwcs = 0;
   wcspih(header, nkeys, relax, 0, &nreject, &nwcs, &wcs);
   anyfile->nwcs = nwcs;
   anyfile->wcs = wcs;

   return status;
}

int wcs_free(void *file) {

   Anyfile *anyfile;
   struct wcsprm **wcs;
   int nwcs;
   int status = 0;

   anyfile = (Anyfile *)file;
   wcs = &(anyfile->wcs);
   nwcs = anyfile->nwcs;
   if (wcs != NULL) {
   	status = wcsvfree(&nwcs, wcs);
	anyfile->wcs = NULL;
   }
   return status;
}
