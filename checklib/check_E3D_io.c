/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_3D_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for Tiger frames I/O
! COMMENT      
! VERSION      4.0  1994-Jul-26 : Creation, AR
!              4.1  2002-Nov-30 : Convert to Euro3D format
------------------------------------------------------------------------------*/

#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with datacube I/O
!
!.prog                             check_3D_io()
!
!.purp                Facility to check datacube I/O on new OS
!
-----------------------------------------------------------------------------*/

#define NB_SPEC 100
#define NPIX 300

int main(int argc, char **argv)
{
	E3D_file image;
	SPECTRUM signal,noise;
	SLICE s_signal;
	SPAXEL *spaxels, spax;
	IMAGE2D img2D;
	TABLE tbl;
	char **argval, **arglabel, tab_name[lg_name+1];
	int specId, i,j, npix=NPIX, nb_spec=NB_SPEC, status, nol[NB_SPEC], sel[NB_SPEC];
	double start, step;
	short sval;
	long lval;
	float fval, x[NB_SPEC],y[NB_SPEC];
	double dval;

	step = 1;
	start = 1;

	printf("IOLIB routines for Euro3D format:\n\n");
	init_session(argv,argc,&arglabel,&argval);

/*	set_control_level(WARNING); */

	for (specId=1;specId<=nb_spec;specId++) {
		nol[specId-1] = specId;
		x[specId-1] = (float)specId;
		y[specId-1] = (float)specId;
	}


	printf("\nOnly signal spectra:\n\n");

	/* 1 - Type SHORT */

	printf("Datacube creation (type of storage = Short) ...");
	fflush(stdout);
	create_E3D_file(&image,"s_chk_E3D_io",npix,start,step,SHORT,"dummy frame",NULL);
	printf("Ok\n");
	printf("Writing into created datacube ...");
	fflush(stdout);

	for (specId=1;specId<=nb_spec;specId++) {
		spax.specId = specId;
		spax.xpos = x[specId -1];
		spax.ypos = y[specId -1];
		spax.group = 1;
		init_new_E3D_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(short)(specId*j));
		}
		put_E3D_spec(&image,&signal,NULL,specId);
		put_E3D_spaxels(&image,specId,1,&spax);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	WR_desc(&image,"TOTO",CHAR,10,"tototototo");
	fval = 25.6;
	WR_desc(&image,"FTOTO",FLOAT,1,&fval);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	image.history[0] = '\0';
	open_E3D_file(&image,"s_chk_E3D_io","I");
	printf("Ok\n");
	specId = (int)(nb_spec/2);
	/* status = exist_statistical_error(&image); */
	printf("Test for Statistical error spectra returns %d\n",status);
	status = exist_spec_ID(&image,specId);
	if (status < 0) {
		printf("unable to locate signal from lens no %d\n",specId);
		fflush(stdout);
		return(-1);
	} 
	status = get_E3D_spaxels(&image,specId,&spaxels); 
	printf("Number of spaxels for spectrum %d is %d\n",specId,status);

	printf("Reading datacube values slice by slice ...");
	fflush(stdout);
	for (i=0;i<npix;i++) {
		get_E3D_slice(&image,&s_signal,NULL,i); 
		for (j=0;j<s_signal.npts;j++) {
			specId = s_signal.specId[j];
			sval = (short)RD_slice(&s_signal,j);
			if (sval != (short)(specId*i)) {
				printf("FATAL : Unexpected values in datacube\n");
				printf("expected value = %d, got %d\n",(short)(specId*i),sval);
				printf("at slice %d, pixel %d\n",i,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Reading the set of spectra as an image ...");
	get_E3D_frame(&image,&img2D,NULL);
	printf("Ok\n");
	free_frame_mem(&img2D);

	printf("Closing datacube ...");
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n");

	printf("Datacube creation slice by slice (type of storage = Short) ...");
	fflush(stdout);
	create_E3D_file(&image,"s_chk_E3D_io",npix,start,step,SHORT,"dummy frame",NULL);
	printf("Ok\n");
	if (has_common_bounds(&image))
		printf("Common bounds Ok\n");
	printf("Setting lenses coordinates ...");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Ok\n");
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (i=0;i<npix;i++) {
		init_new_E3D_slice(&image,&s_signal); 
		for (j=0;j<s_signal.npts;j++) {
			specId = s_signal.specId[j];
			WR_slice(&s_signal,j,(short)(specId*i));
		}
		put_E3D_slice(&image,&s_signal,NULL,i);
	}
	printf("Ok\n");
	printf("Closing datacube ...");
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"s_chk_E3D_io","I");
	printf("Ok\n");

	get_spectra_ID(&image,nol);
	printf("Reading datacube values spectrum by spectrum ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,NULL,nol[specId-1]); 
		for (j=0;j<signal.npts;j++) {
			sval = (short)RD_spec(&signal,j);
			if (sval != (short)(specId*j)) {
				printf("FATAL : Unexpected values in datacube\n");
				printf("expected value = %d, got %d\n",(short)(specId*j),sval);
				printf("at specId %d, pixel %d\n",specId,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* 2 - Type LONG */

	printf("Datacube creation (type of storage = Long) ...");
	fflush(stdout);
	create_E3D_file(&image,"l_chk_E3D_io",-1,start,step,LONG,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(long)(specId*j));
		}
		put_E3D_spec(&image,&signal,NULL,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"l_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values spectrum by spectrum ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,NULL,specId); 
		for (j=0;j<signal.npts;j++) {
			lval = (long)RD_spec(&signal,j);
			if (lval != (long)(specId*j)) {
				printf("FATAL : Unexpected values in datacube\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Reading datacube values slice by slice ...");
	fflush(stdout);
	for (i=0;i<npix;i++) {
		get_E3D_slice(&image,&s_signal,NULL,i); 
		for (j=0;j<s_signal.npts;j++) {
			specId = s_signal.specId[j];
			lval = (long)RD_slice(&s_signal,j);
			if (lval != (long)(specId*i)) {
				printf("FATAL : Unexpected values in datacube\n");
				printf("expected value = %ld, got %ld\n",(long)(specId*i),lval);
				printf("at slice %d, pixel %d\n",i,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* 3 - Type FLOAT */

	printf("Datacube creation (type of storage = Float) ...");
	fflush(stdout);
	create_E3D_file(&image,"f_chk_E3D_io",npix,start,step,FLOAT,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(float)(specId*j));
		}
		put_E3D_spec(&image,&signal,NULL,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"f_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,NULL,specId); 
		for (j=0;j<signal.npts;j++) {
			fval = (float)RD_spec(&signal,j);
			if (fval != (float)(specId*j)) {
				printf("FATAL : Unexpected values in datacube\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* 4 - Type DOUBLE */

	printf("Datacube creation (type of storage = Double) ...");
	fflush(stdout);
	create_E3D_file(&image,"d_chk_E3D_io",npix,start,step,DOUBLE,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(double)(specId*j));
		}
		put_E3D_spec(&image,&signal,NULL,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"d_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,NULL,specId); 
		for (j=0;j<signal.npts;j++) {
			dval = (double)RD_spec(&signal,j);
			if (dval != (double)(specId*j)) {
				printf("FATAL : Unexpected values in datacube\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	printf("Signal & statistical error spectra:\n\n");

	/* 1 - Type SHORT */

	printf("Datacube creation (type of storage = Short) ...");
	fflush(stdout);
	create_E3D_file(&image,"s_chk_E3D_io",npix,start,step,SHORT,
		"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		init_new_E3D_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(short)(specId*j));
			WR_spec(&noise,j,(short)(specId*j));
		}
		put_E3D_spec(&image,&signal,&noise,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"s_chk_E3D_io","I");
	printf("Ok\n");
	status = exist_statistical_error(&image);
	printf("Test for Statistical error spectra returns %d\n",status);
	specId = (int)(nb_spec/2);
	status = exist_spec_ID(&image,specId);
	if (status < 0) {
		printf("unable to locate signal from lens no %d\n",specId);
		fflush(stdout);
		return(-1);
	} 

	printf("Reading datacube values ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,&noise,specId); 
		for (j=0;j<signal.npts;j++) {
			sval = (short)RD_spec(&signal,j);
			if (sval != (short)(specId*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			sval = (short)RD_spec(&noise,j);
			if (sval != (short)(specId*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* 2 - Type LONG */

	printf("Datacube creation (type of storage = Long) ...");
	fflush(stdout);
	create_E3D_file(&image,"l_chk_E3D_io",npix,start,step,LONG,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		init_new_E3D_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(long)(specId*j));
			WR_spec(&noise,j,(long)(specId*j));
		}
		put_E3D_spec(&image,&signal,&noise,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"l_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,&noise,specId); 
		for (j=0;j<signal.npts;j++) {
			lval = (long)RD_spec(&signal,j);
			if (lval != (long)(specId*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			lval = (long)RD_spec(&noise,j);
			if (lval != (long)(specId*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* 3 - Type FLOAT */

	printf("Datacube creation (type of storage = Float) ...");
	fflush(stdout);
	create_E3D_file(&image,"f_chk_E3D_io",npix,start,step,FLOAT,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		init_new_E3D_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(float)(specId*j));
			WR_spec(&noise,j,(float)(specId*j));
		}
		put_E3D_spec(&image,&signal,&noise,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"f_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,&noise,specId); 
		for (j=0;j<signal.npts;j++) {
			fval = (float)RD_spec(&signal,j);
			if (fval != (float)(specId*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			fval = (float)RD_spec(&noise,j);
			if (fval != (float)(specId*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* 4 - Type DOUBLE */

	printf("Datacube creation (type of storage = Double) ...");
	fflush(stdout);
	create_E3D_file(&image,"d_chk_E3D_io",npix,start,step,DOUBLE,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		init_new_E3D_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(double)(specId*j));
			WR_spec(&noise,j,(double)(specId*j));
		}
		put_E3D_spec(&image,&signal,&noise,specId);
	}
	printf("Ok\n");

	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"d_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		get_E3D_spec(&image,&signal,&noise,specId); 
		for (j=0;j<signal.npts;j++) {
			dval = (double)RD_spec(&signal,j);
			if (dval != (double)(specId*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			dval = (double)RD_spec(&noise,j);
			if (dval != (double)(specId*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	/* Checking datacube selection */

	printf("Checking selection on Datacube\n\n");
	printf("Datacube creation (type of storage = Double) ...");
	fflush(stdout);
	create_E3D_file(&image,"d_chk_E3D_io",npix,start,step,DOUBLE,"dummy frame",NULL);
	printf("Ok\n");
	set_ID_and_coordinates(&image,nb_spec,nol,x,y);
	printf("Writing into created datacube ...");
	fflush(stdout);
	for (specId=1;specId<=nb_spec;specId++) {
		init_new_E3D_spec(&image,&signal,npix,start); 
		init_new_E3D_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(double)(specId*j));
			WR_spec(&noise,j,(double)(specId*j));
		}
		put_E3D_spec(&image,&signal,&noise,specId);
	}
	printf("Ok\n");
	printf("Saving created datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	printf("Ok\n");

	printf("Setting selection ...");
	fflush(stdout);
	get_coord_table_name("d_chk_E3D_io", tab_name);
	open_table(&tbl,tab_name,"IO");
        for (i=0; i<tbl.row; i++) {
                if (i%2 == 0)
                        sel[i] = 0;
                else
                        sel[i] = 1;
        }
        write_selection(&tbl,sel,"only.even.row.number");
	printf("Ok\n");
	close_table(&tbl);

	printf("Opening previous datacube ...");
	fflush(stdout);
	open_E3D_file(&image,"d_chk_E3D_io","I");
	printf("Ok\n");

	printf("Reading datacube values ...");
	fflush(stdout);
	for (i=0;i<image.nbspec;i++) {
		specId = image.signal[i].specId;
		get_E3D_spec(&image,&signal,&noise,specId); 
		for (j=0;j<signal.npts;j++) {
			dval = (double)RD_spec(&signal,j);
			if (dval != (double)(specId*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			dval = (double)RD_spec(&noise,j);
			if (dval != (double)(specId*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous datacube ...");
	fflush(stdout);
	close_E3D_file(&image);
	delete_E3D_file(&image);
	printf("Ok\n\n");

	exit_session(0);
	return(0);
}
