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

#include <stdlib.h>
#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with spectra I/O
!
!.prog                             check_spec_io()
!
!.purp 
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	SPECTRUM spec1, spec2;
	char **argval, **arglabel;
	int i, npix=1000;
	double start=1, step=1;
	long iarray[10], iref[10], testd=9999;
	float farray[10], fref[10];
	double darray[10], dref[10];
	char carray[6], cref[6];
	double d_temp = -79.2000; 

	for (i=0;i<10;i++) {
		iref[i] = i;
		fref[i] = i;
		dref[i] = (double)(1.1*i);
	}
	strcpy(cref,"ABCDE");

	printf("IO routines for descriptors:\n\n");
	fflush(stdout);
	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

						/*  create_spec() checking */	
	printf("Spectrum creation (type of storage = Short) ...");
	fflush(stdout);
	create_spec(&spec1,"s_chk_spec_io",npix,start,step,SHORT,
						"dummy spectrum",NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created spectrum ...");
	fflush(stdout);
	for (i=0;i<spec1.npts;i++) {
			WR_spec(&spec1,i,i)
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving descriptors ...");
	fflush(stdout);
	WR_desc(&spec1,"INT",LONG,10,iref);
	WR_desc(&spec1,"FLOAT",FLOAT,10,fref);
	WR_desc(&spec1,"DOUBLE",DOUBLE,10,dref);
	WR_desc(&spec1,"CHAR",CHAR,5,cref);
		/* hiearchical keywords */
	WR_desc (&spec1, "ESO INSTEMP5 VAL", DOUBLE, 1, &d_temp);
        WR_desc (&spec1, "eso detTEMP5 VAL", DOUBLE, 1, &d_temp);
        WR_desc (&spec1, "TEST INSTEMP5 VAL blah blah", DOUBLE, 1, &d_temp);
        WR_desc (&spec1, "reallylongkeyword", DOUBLE, 1, &d_temp);
	printf("Ok\n");
	fflush(stdout);
	printf("Saving created spectrum ...");
	fflush(stdout);
	close_spec(&spec1);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous spectrum ...");
	fflush(stdout);
	open_spec(&spec1,"s_chk_spec_io","I");
	printf("Ok\n");

	printf("Spectrum creation (type of storage = Long) ...");
	fflush(stdout);
	create_spec(&spec2,"l_chk_spec_io",npix,start,step,LONG,NULL,NULL);
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created spectrum ...");
	fflush(stdout);
	for (i=0;i<spec2.npts;i++) {
			WR_spec(&spec2,i,i);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Saving descriptors ...");
	fflush(stdout);
	CP_non_std_desc(&spec1,&spec2);
	WR_desc(&spec2,"TESTD",INT,1,&testd);
	printf("Ok\n");
	fflush(stdout);
	printf("Saving created spectrum ...");
	fflush(stdout);
	close_spec(&spec2);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous spectrum ...");
	fflush(stdout);
	open_spec(&spec2,"l_chk_spec_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Reading descriptors ...");
	fflush(stdout);
	RD_desc(&spec2,"INT",INT,10,iarray);
	RD_desc(&spec2,"FLOAT",FLOAT,10,farray);
	RD_desc(&spec2,"DOUBLE",DOUBLE,10,darray);
	RD_desc(&spec2,"CHAR",CHAR,5,carray);

	for (i=0;i<10;i++) {
		if (iarray[i] != iref[i]) {
			printf("Unexpected value in iarray. Got %ld , expected %ld\n",(long)iarray[i],(long)iref[i]);
			exit(-1);
		};
		if (farray[i] != fref[i]) {
			printf("Unexpected value in farray. Got %f , expected %f\n",farray[i],fref[i]);
			exit(-1);
		};
		if (ABS(darray[i] - dref[i])>0.05) {
			printf("Unexpected value in darray. Got %f , expected %f\n",darray[i],dref[i]);
			exit(-1);
		};
	}
	if (strcmp(carray,cref) != 0 ) {
			printf("Unexpected value in carray. Got %s , expected %s\n",carray,cref);
			exit(-1);
	}
	printf("Ok\n");
	fflush(stdout);
	printf("Closing created spectrum ...");
	fflush(stdout);
	close_spec(&spec2);
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous spectra ...");
	fflush(stdout);
/*	delete_spec(&spec1);
	delete_spec(&spec2); */
	printf("Ok\n\n");

	exit_session(0);
	return(0);
}
