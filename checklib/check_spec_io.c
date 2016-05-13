/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_spec_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for spectra I/O
! COMMENT      
! VERSION      4.0  1993-May-07 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with spectra I/O
!
!.prog                             check_spec_io()
!
!.purp                  Facility to check spectra I/O on new OS
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	IMAGE1D spectrum;
	/* SPECTRUM spectrum;*/
	char **argval, **arglabel;
	int i, npix=1000;
	double start=1, step=1;
	short sval;
	long lval;
	float fval;
	double dval;


	printf("IOLIB routines for spectra:\n\n");
	fflush(stdout);
	set_version("1.0");
	set_purpose("check the differents types of i/o functions for 1D images (spectra)");

	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

						/*  create_spec() checking */	
	/* 1 - Type SHORT */

	printf("Spectrum creation (type of storage = Short) ...");
	fflush(stdout);
	create_spec(&spectrum,"./s_chk_spec_io",npix,start,step,SHORT,
						"dummy spectrum",NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created spectrum ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			WR_spec(&spectrum,i,i)
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created spectrum ...");
	fflush(stdout);
	close_spec(&spectrum);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous spectrum ...");
	fflush(stdout);
	open_spec(&spectrum,"s_chk_spec_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading spectrum values ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			sval = (short)RD_spec(&spectrum,i);
			if (sval != (short)i) {
				printf("FATAL : Unexpected values in spectrum\n");
				return(-1);
			}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous spectrum ...");
	fflush(stdout);
	delete_spec(&spectrum);
	printf("Ok\n\n");

	fflush(stdout);

	/* 2 - Type LONG */

	printf("Spectrum creation (type of storage = Long) ...");
	fflush(stdout);
	create_spec(&spectrum,"l_chk_spec_io",npix,start,step,LONG,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created spectrum ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			WR_spec(&spectrum,i,i);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created spectrum ...");
	fflush(stdout);
	close_spec(&spectrum);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous spectrum ...");
	fflush(stdout);
	open_spec(&spectrum,"l_chk_spec_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading spectrum values ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			lval = (long)RD_spec(&spectrum,i);
			if (lval != (long)i) {
				printf("FATAL : Unexpected values in spectrum\n");
				return(-1);
			}
	} 
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous spectrum ...");
	fflush(stdout);
	delete_spec(&spectrum);
	printf("Ok\n\n");
	fflush(stdout);

	/* 3 - Type FLOAT */

	printf("Spectrum creation (type of storage = Float) ...");
	fflush(stdout);
	create_spec(&spectrum,"f_chk_spec_io",npix,start,step,FLOAT,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created spectrum ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			WR_spec(&spectrum,i,i);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created spectrum ...");
	fflush(stdout);
	close_spec(&spectrum);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous spectrum ...");
	fflush(stdout);
	open_spec(&spectrum,"f_chk_spec_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading spectrum values ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			fval = (float)RD_spec(&spectrum,i);
			if (fval != (float)i) {
				printf("FATAL : Unexpected values in spectrum\n");
				return(-1);
			}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous spectrum ...");
	fflush(stdout);
	delete_spec(&spectrum);
	printf("Ok\n\n");
	fflush(stdout);

	/* 4 - Type DOUBLE */

	printf("Spectrum creation (type of storage = Double) ...");
	fflush(stdout);
	create_spec(&spectrum,"d_chk_spec_io",npix,start,step,DOUBLE,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created spectrum ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			WR_spec(&spectrum,i,i);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created spectrum ...");
	fflush(stdout);
	close_spec(&spectrum);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous spectrum ...");
	fflush(stdout);
	open_spec(&spectrum,"d_chk_spec_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading spectrum values ...");
	fflush(stdout);
	for (i=0;i<spectrum.npts;i++) {
			dval = (double)RD_spec(&spectrum,i);
			if (dval != (double)i) {
				printf("FATAL : Unexpected values in spectrum\n");
				return(-1);
			}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous spectrum ...");
	fflush(stdout);
	delete_spec(&spectrum);
	printf("Ok\n\n");
	fflush(stdout);

	exit_session(0);
	return(0);
}
