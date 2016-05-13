/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_cplx2D_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for iamges I/O
! COMMENT      
! VERSION      4.0  1993-May-11 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with image I/O
!
!.prog                             check_image_io()
!
!.purp 	              Facility to check images I/O on new OS
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	COMPLEX_IMAGE2D image;
	char **argval, **arglabel;
	int i, j, npix[2];
	double start[2], step[2];
	double dval;

	start[0] = 1; start[1] = 1;
	step[0] = 1; step[1] = 1;
	npix[0] = 500; npix[1] = 1000;

	printf("IOLIB routines for 2D complex images:\n\n");
	fflush(stdout);
	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

	printf("Image creation (type of storage = Double) ...");
	fflush(stdout);
	create_COMPLEX_IMAGE2D(&image,"d_chk_cplx2D_io",npix,start,step,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created image ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			WR_REAL_cplxima(&image,i,j,(i*j));
			WR_IMAG_cplxima(&image,i,j,(i*j+1));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created image ...");
	fflush(stdout);
	close_COMPLEX_IMAGE2D(&image);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous image ...");
	fflush(stdout);
	open_COMPLEX_IMAGE2D(&image,"d_chk_cplx2D_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading image values ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			dval = (double)RD_REAL_cplxima(&image,i,j);
			if (dval != (double)(i*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
			dval = (double)RD_IMAG_cplxima(&image,i,j);
			if (dval != (double)(i*j+1)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous image ...");
	fflush(stdout);
	close_COMPLEX_IMAGE2D(&image);
	printf("Ok\n\n");
	fflush(stdout);

	exit_session(0);
	return(0);
}
