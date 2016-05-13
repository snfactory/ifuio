/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_ima_ext_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for iamges I/O
! COMMENT      
! VERSION      4.0  1993-May-11 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>
#include <fitsio.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with image I/O
!
!.prog                             check_image_ext_io()
!
!.purp 	              Facility to check images I/O in case of FITS extensions
!
-----------------------------------------------------------------------------*/

int info_fitsfile(IMAGE2D *image) 
{
	fitsfile *fptr;

	fptr = (fitsfile *)image->external_info;
	printf("current HDU pos = %d \n", fptr->HDUposition+1);

	return 0;
}
int create_2D_image(IMAGE2D *image, char *filename)
{
	int i, j, npix[2];
	double start[2], step[2];
	int status;

	start[0] = 1; start[1] = 1;
	step[0] = 1; step[1] = 1;
	npix[0] = 500; npix[1] = 1000;

	printf("Image creation (%s) ...", filename);
	fflush(stdout);
	status = create_frame(image,filename,npix,start,step,SHORT,"dummy frame",NULL);
	if (status)
		printf("Bad, status=%d\n", status);
	else
		printf("Ok\n");
	fflush(stdout);

	info_fitsfile(image);
	return status;
}

int open_2D_image(IMAGE2D *image, char *filename)
{
	int status;

	printf("Opening image (%s) ...", filename);
	fflush(stdout);
	status = open_frame(image,filename,"IO");
	if (status)
		printf("Bad, status=%d\n", status);
	else
		printf("Ok\n");
	fflush(stdout);

	info_fitsfile(image);
	return status;
}

int close_2D_image(IMAGE2D *image)
{
	int status;

	printf("Closing frame (%s) ...", image->name);
	fflush(stdout);

	status = close_frame(image);
	if (status)
		printf("Bad, status=%d\n", status);
	else
		printf("Ok\n");
	fflush(stdout);
	return status;
}

int delete_2D_image(IMAGE2D *image)
{
	int status;

	printf("Deleting image (%s) ...", image->name);
	fflush(stdout);
	status = delete_frame(image);
	if (status)
		printf("Bad, status=%d\n", status);
	else
		printf("Ok\n");
	fflush(stdout);
	return status;
}

int write_2D_image(IMAGE2D *image)
{
	int i,j;

	printf("Writing into %s ...", image->name);
	fflush(stdout);
	for (i=0;i<image->nx;i++) {
		for (j=0;j<image->ny;j++) {
			WR_frame(image,i,j,(i*j));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	info_fitsfile(image);
	return 0;
}

int read_2D_image(IMAGE2D *image) 
{
	int i,j;
	short sval;

	printf("Reading image %s ...", image->name);
	fflush(stdout);
	for (i=0;i<image->nx;i++) {
		for (j=0;j<image->ny;j++) {
			sval = (short)RD_frame(image,i,j);
			if (sval != (short)(i*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	info_fitsfile(image);
	return 0;
}

int main(int argc, char **argv)
{
	IMAGE2D image, image2;
	char **argval, **arglabel;

	printf("IOLIB routines for 2D images with extensions:\n\n");
	fflush(stdout);
	init_session(argv,argc,&arglabel,&argval);

	printf("Alphanumerical extensions:\n\n");

	create_2D_image(&image, "chk_ima_ext.fits[HELLO1]");
	write_2D_image(&image);
	close_2D_image(&image);

	open_2D_image(&image, "chk_ima_ext.fits[HELLO1]");
	if (read_2D_image(&image) < 0) exit(-1);
	close_2D_image(&image);
	delete_2D_image(&image);

	create_2D_image(&image, "chk_ima_ext.fits");
	write_2D_image(&image);
	close_2D_image(&image);

	create_2D_image(&image, "chk_ima_ext.fits[HELLO1]");
	write_2D_image(&image);
	close_2D_image(&image);

	create_2D_image(&image, "chk_ima_ext.fits[HELLO2]");
	write_2D_image(&image);
	close_2D_image(&image);

	open_2D_image(&image2, "chk_ima_ext.fits[HELLO2]");
	create_2D_image(&image, "chk_ima_ext.fits[HELLO1]");

	if (read_2D_image(&image2) < 0) exit(-1);
	write_2D_image(&image);
	close_2D_image(&image);
	delete_2D_image(&image);

	close_2D_image(&image2);
	delete_2D_image(&image2);

	create_2D_image(&image, "chk_ima_ext.fits[HELLO2]");
	write_2D_image(&image);
	close_2D_image(&image);
	open_2D_image(&image, "chk_ima_ext.fits");
	delete_2D_image(&image);

	exit_session(0);
	return(0);
}
