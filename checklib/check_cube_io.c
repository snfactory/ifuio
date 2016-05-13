/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_cube_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for images I/O
! COMMENT      
! VERSION      4.0  1993-May-11 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with cube I/O
!
!.prog                             check_cube_io()
!
!.purp               Facility to check 3D cube I/O on new OS
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	IMAGE3D cube;
	char **argval, **arglabel;
	int i, j, k, npix[3];
	double start[3], step[3];
	short sval;
	long lval;
	float fval;
	double dval;

	start[0] = 1; start[1] = 1; start[2] = 1;
	step[0] = 1; step[1] = 1; step[2] = 1;
	npix[0] = 500; npix[1] = 100; npix[2] = 15;

	printf("IOLIB routines for 3D cubes:\n\n");
	fflush(stdout);
	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

						/*  create_cube() checking */	
	/* 1 - Type SHORT */

	printf("Cube creation (type of storage = Short) ...");
	fflush(stdout);
	create_cube(&cube,"s_chk_cube_io",npix,start,step,SHORT,"dummy cube",NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created cube ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++)
				WR_cube(&cube,i,j,k,(i*j*k));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created cube ...");
	fflush(stdout);

	close_cube(&cube);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous cube ...");
	fflush(stdout);
	open_cube(&cube,"s_chk_cube_io","I");
	printf("Ok\n");
	fflush(stdout);
	printf("Checking descriptor type ...\n");
	get_descr_type(&cube,"HISTORY",&sval);
	printf("HISTORY type = %c\n",(char)sval);

	printf("Reading cube values ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++) {
				sval = (short)RD_cube(&cube,i,j,k);
				if (sval != (short)(i*j*k)) {
					printf("FATAL : Unexpected values in cube\n");
					return(-1);
				}
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous cube ...");
	fflush(stdout);
	delete_cube(&cube);
	printf("Ok\n\n");
	fflush(stdout);

	/* 2 - Type LONG */

	printf("Cube creation (type of storage = Long) ...");
	fflush(stdout);
	create_cube(&cube,"l_chk_cube_io",npix,start,step,LONG,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created cube ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++)
				WR_cube(&cube,i,j,k,(i*j*k));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created cube ...");
	fflush(stdout);
	close_cube(&cube);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous cube ...");
	fflush(stdout);
	open_cube(&cube,"l_chk_cube_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading cube values ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++) {
				lval = (long)RD_cube(&cube,i,j,k);
				if (lval != (long)(i*j*k)) {
					printf("FATAL : Unexpected values in cube\n");
					return(-1);
				}
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous cube ...");
	fflush(stdout);
	delete_cube(&cube);
	printf("Ok\n\n");
	fflush(stdout);

	/* 3 - Type FLOAT */

	printf("Cube creation (type of storage = Float) ...");
	fflush(stdout);
	create_cube(&cube,"f_chk_cube_io",npix,start,step,FLOAT,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created cube ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++)
				WR_cube(&cube,i,j,k,(i*j*k));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created cube ...");
	fflush(stdout);
	close_cube(&cube);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous cube ...");
	fflush(stdout);
	open_cube(&cube,"f_chk_cube_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading cube values ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++) {
				fval = (float)RD_cube(&cube,i,j,k);
				if (fval != (float)(i*j*k)) {
					printf("FATAL : Unexpected values in cube\n");
					return(-1);
				}
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous cube ...");
	fflush(stdout);
	delete_cube(&cube);
	printf("Ok\n\n");
	fflush(stdout);

	/* 4 - Type DOUBLE */

	printf("Cube creation (type of storage = Double) ...");
	fflush(stdout);
	create_cube(&cube,"d_chk_cube_io",npix,start,step,DOUBLE,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created cube ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++)
				WR_cube(&cube,i,j,k,(i*j*k));
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving created cube ...");
	fflush(stdout);
	close_cube(&cube);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous cube ...");
	fflush(stdout);
	open_cube(&cube,"d_chk_cube_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading cube values ...");
	fflush(stdout);
	for (i=0;i<cube.nx;i++) {
		for (j=0;j<cube.ny;j++) {
			for (k=0;k<cube.nz;k++) {
				dval = (double)RD_cube(&cube,i,j,k);
				if (dval != (double)(i*j*k)) {
					printf("FATAL : Unexpected values in cube\n");
					return(-1);
				}
			}
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous cube ...");
	fflush(stdout);
	delete_cube(&cube);
	printf("Ok\n\n");
	fflush(stdout);

	exit_session(0);
	return(0);
}
