/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_ima_io.c
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
!.prog                             check_flip_ima()
!
!.purp 
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	IMAGE2D image;
	char **argval, **arglabel;
	int i, j, npix[2];
	double start[2], step[2];

	start[0] = 1; start[1] = 1;
	step[0] = 1; step[1] = 1;
	npix[0] = 6; npix[1] = 8;

	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

	create_frame(&image,"flip",npix,start,step,FLOAT,NULL,NULL);
	fflush(stdout);
	for (j=0;j<image.ny;j++) {
		for (i=0;i<image.nx;i++) {
			WR_frame(&image,i,j,(float)i);
		}
	}
	for(j=0; j<image.ny;j++) {
		for(i=0; i<image.nx;i++) {
			printf("%f ",RD_frame(&image,i,j));
		}
		printf("\n");
	}

	printf("Flipping columns ...");
	fflush(stdout);
	flip_frame(&image,TopToBottom);
	printf("Ok\n");
	fflush(stdout);

	for(j=0; j<image.ny;j++) {
		for(i=0; i<image.nx;i++) {
			printf("%f ",RD_frame(&image,i,j));
		}
		printf("\n");
	}

	delete_frame(&image);
	printf("Ok\n");
	fflush(stdout);

	exit_session(0);
	return(0);
}
