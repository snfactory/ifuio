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
!.prog                             check_image_io()
!
!.purp 	              Facility to check images I/O on new OS
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	IMAGE2D image;
	char **argval, **arglabel;
	int i, j, npix[2];
	double start[2], step[2];
	short sval;
	long lval;
	float fval;
	double dval;

	start[0] = 1; start[1] = 1;
	step[0] = 1; step[1] = 1;
	npix[0] = 500; npix[1] = 1000;

	printf("IOLIB routines for 2D images:\n\n");
	fflush(stdout);
	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

						/*  create_frame() checking */	
	/* 1 - Type SHORT */

	printf("Image creation (type of storage = Short) ...");
	fflush(stdout);
	create_frame(&image,"s_chk_ima_io",npix,start,step,SHORT,"dummy frame",NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created image ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			WR_frame(&image,i,j,(i*j));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created image ...");
	fflush(stdout);

	close_frame(&image);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous image ...");
	fflush(stdout);
	open_frame(&image,"s_chk_ima_io","I");
	printf("Ok\n");
	fflush(stdout);
	printf("Checking descriptor type ...\n");
	get_descr_type(&image,"HISTORY",&sval);
	printf("HISTORY type = %c\n",(char)sval);

	printf("Reading image values ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			sval = (short)RD_frame(&image,i,j);
			if (sval != (short)(i*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_frame(&image);
	printf("Ok\n\n");
	fflush(stdout);

	/* 2 - Type LONG */

	printf("Image creation (type of storage = Long) ...");
	fflush(stdout);
	create_frame(&image,"l_chk_ima_io",npix,start,step,INT,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created image ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			WR_frame(&image,i,j,(i*j));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created image ...");
	fflush(stdout);
	close_frame(&image);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous image ...");
	fflush(stdout);
	open_frame(&image,"l_chk_ima_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading image values ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			lval = (long)RD_frame(&image,i,j);
			if (lval != (long)(i*j)) {
				printf("FATAL : Unexpected values in image, got %ld, expected %d\n",lval,(i*j));
				return(-1);
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_frame(&image);
	printf("Ok\n\n");
	fflush(stdout);

	/* 3 - Type FLOAT */

	printf("Image creation (type of storage = Float) ...");
	fflush(stdout);
	create_frame(&image,"f_chk_ima_io",npix,start,step,FLOAT,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created image ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			WR_frame(&image,i,j,(i*j));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created image ...");
	fflush(stdout);
	close_frame(&image);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous image ...");
	fflush(stdout);
	open_frame(&image,"f_chk_ima_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading image values ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			fval = (float)RD_frame(&image,i,j);
			if (fval != (float)(i*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_frame(&image);
	printf("Ok\n\n");
	fflush(stdout);

	/* 4 - Type DOUBLE */

	printf("Image creation (type of storage = Double) ...");
	fflush(stdout);
	create_frame(&image,"d_chk_ima_io",npix,start,step,DOUBLE,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created image ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			WR_frame(&image,i,j,(i*j));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created image ...");
	fflush(stdout);
	close_frame(&image);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous image ...");
	fflush(stdout);
	open_frame(&image,"d_chk_ima_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading image values ...");
	fflush(stdout);
	for (i=0;i<image.nx;i++) {
		for (j=0;j<image.ny;j++) {
			dval = (double)RD_frame(&image,i,j);
			if (dval != (double)(i*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_frame(&image);
	printf("Ok\n\n");
	fflush(stdout);

	exit_session(0);
	return(0);
}
