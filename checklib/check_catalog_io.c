/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_catalog_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for catalog I/O
! COMMENT      
! VERSION      4.0  1994-Jul-05 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface for catalog I/O
!
!.prog                             check_catalog_io()
!
!.purp                 Facility to check catalog I/O on new OS
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	char **argval, **arglabel, filename[21];

	printf("IO routines for catalogs:\n\n");
	init_session(argv,argc,&arglabel,&argval);

	printf("\nNumber of files in catalog is %d\n\n",f_lines("gen.cat")-1);

	printf("Reading catalog values ...\n");
	while (RD_catalog("gen.cat",filename) == TRUE) {
		printf("%s\n",filename);
	}
	printf("OK\n");

	exit_session(0);
	return(0);
}
