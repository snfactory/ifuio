/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_tiger_io.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces for Tiger frames I/O
! COMMENT      
! VERSION      4.0  1994-Jul-26 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the interface with image I/O
!
!.prog                             check_tiger_io()
!
!.purp 
!
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	TIGERfile image;
	SPECTRUM signal,noise;
	SLICE s_signal;
	char **argval, **arglabel, toto[50];
	int nolens, i,j, npix=300, nb_spec=100, status, nol[105];
	double start, step;
	short sval;
	long lval;
	float fval, x[105],y[105];
	double dval;

	step = 1;
	start = 1;

	printf("IOLIB routines for Tiger format:\n\n");
	init_session(argv,argc,&arglabel,&argval);
/*	set_control_level(WARNING); */

	printf("\nOnly signal spectra:\n\n");

	/* 1 - Type SHORT */

	for (nolens=1;nolens<=nb_spec+5;nolens++) {
		nol[nolens-1] = nolens;
		x[nolens-1] = (float)nolens;
		y[nolens-1] = (float)nolens;
	}

	printf("Image creation (type of storage = Short) ...");
	fflush(stdout);
	create_tiger_frame(&image,"s_chk_tiger_io",npix,start,step,SHORT,
		"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	printf("Writing into created image ...");
	fflush(stdout);
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);

	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(short)(nolens*j));
		}
		put_tiger_spec(&image,&signal,NULL,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	WR_desc(&image,"TOTO",CHAR,10,"tototototo");
	fval = 25.6;
	WR_desc(&image,"FTOTO",FLOAT,1,&fval);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	image.history[0] = '\0';
	open_tiger_frame(&image,"s_chk_tiger_io","I");
	printf("Ok\n");
	printf("%s\n",image.history);

	nolens = (int)(nb_spec/2);
	status = exist_lens(&image,nolens);
	if (status < 0) {
		printf("unable to locate signal for lens #%d\n",nolens);
		fflush(stdout);
		return(-1);
	} 

	printf("Reading image values slice by slice ...");
	fflush(stdout);
	for (i=0;i<npix;i++) {
		get_tiger_slice(&image,&s_signal,NULL,i); 
		for (j=0;j<s_signal.npts;j++) {
			nolens = s_signal.specId[j];
			sval = (short)RD_slice(&s_signal,j);
			if (sval != (short)(nolens*i)) {
				printf("FATAL : Unexpected values in image\n");
				printf("expected value = %d, got %d\n",(short)(nolens*i),sval);
				printf("at slice %d, pixel %d\n",i,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");
	RD_desc(&image,"TOTO",CHAR,10,toto);
	printf("TOTO = %s\n",toto);
	printf("Closing image ...");
	close_tiger_frame(&image);
	printf("Ok\n");
/*
	printf("Opening previous image in IO mode...");
	fflush(stdout);
	open_tiger_frame(&image,"s_chk_tiger_io","IO");
	printf("Ok\n");
	printf("Writing image values spectrum by spectrum ...");
	fflush(stdout);
	for (nolens=nb_spec+1;nolens<=nb_spec+5;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(short)(nolens*j));
		}
		put_tiger_spec(&image,&signal,NULL,nolens);
	}
	printf("Ok\n");
	close_tiger_frame(&image);
	printf("Opening previous image ...");
	open_tiger_frame(&image,"s_chk_tiger_io","I");
	printf("Ok\n");
	printf("Reading image values spectrum by spectrum ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec+5;nolens++) {
		get_tiger_spec(&image,&signal,NULL,nol[nolens-1]); 
		for (j=0;j<signal.npts;j++) {
			sval = (short)RD_spec(&signal,j);
			if (sval != (short)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				printf("expected value = %d, got %d\n",(short)(nolens*j),sval);
				printf("at nolens %d, pixel %d\n",nolens,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");
	close_tiger_frame(&image);

*/
	printf("Deleting previous image ...");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	printf("Image creation slice by slice (type of storage = Short) ...");
	fflush(stdout);
	create_tiger_frame(&image,"s_chk_tiger_io",npix,start,step,SHORT,
		"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	if (has_common_bounds(&image))
		printf("Common bounds Ok\n");
	printf("Setting lenses coordinates ...");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Ok\n");
	printf("Writing into created image ...");
	fflush(stdout);
	for (i=0;i<npix;i++) {
		init_new_tiger_slice(&image,&s_signal,nb_spec); 
		for (j=0;j<s_signal.npts;j++) {
			nolens = s_signal.specId[j];
			WR_slice(&s_signal,j,(short)(nolens*i));
		}
		put_tiger_slice(&image,&s_signal,NULL,i);
	}
	printf("Ok\n");
	printf("Closing image ...");
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"s_chk_tiger_io","I");
	printf("Ok\n");
	get_lenses_no(&image,nol);
	printf("Reading image values spectrum by spectrum ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,NULL,nol[nolens-1]); 
		for (j=0;j<signal.npts;j++) {
			sval = (short)RD_spec(&signal,j);
			if (sval != (short)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				printf("expected value = %d, got %d\n",(short)(nolens*j),sval);
				printf("at nolens %d, pixel %d\n",nolens,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");
	close_tiger_frame(&image);

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 2 - Type LONG */

	printf("Image creation (type of storage = Long) ...");
	fflush(stdout);
	create_tiger_frame(&image,"l_chk_tiger_io",-1,start,step,LONG,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(long)(nolens*j));
		}
		put_tiger_spec(&image,&signal,NULL,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"l_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values spectrum by spectrum ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,NULL,nolens); 
		for (j=0;j<signal.npts;j++) {
			lval = (long)RD_spec(&signal,j);
			if (lval != (long)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Reading image values slice by slice ...");
	fflush(stdout);
	for (i=0;i<npix;i++) {
		get_tiger_slice(&image,&s_signal,NULL,i); 
		for (j=0;j<s_signal.npts;j++) {
			nolens = s_signal.specId[j];
			lval = (long)RD_slice(&s_signal,j);
			if (lval != (long)(nolens*i)) {
				printf("FATAL : Unexpected values in image\n");
				printf("expected value = %ld, got %ld\n",(long)(nolens*i),lval);
				printf("at slice %d, pixel %d\n",i,j);
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 3 - Type FLOAT */

	printf("Image creation (type of storage = Float) ...");
	fflush(stdout);
	create_tiger_frame(&image,"f_chk_tiger_io",npix,start,step,FLOAT,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(float)(nolens*j));
		}
		put_tiger_spec(&image,&signal,NULL,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"f_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,NULL,nolens); 
		for (j=0;j<signal.npts;j++) {
			fval = (float)RD_spec(&signal,j);
			if (fval != (float)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 4 - Type DOUBLE */

	printf("Image creation (type of storage = Double) ...");
	fflush(stdout);
	create_tiger_frame(&image,"d_chk_tiger_io",npix,start,step,DOUBLE,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(double)(nolens*j));
		}
		put_tiger_spec(&image,&signal,NULL,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"d_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,NULL,nolens); 
		for (j=0;j<signal.npts;j++) {
			dval = (double)RD_spec(&signal,j);
			if (dval != (double)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

#ifdef nonEURO3D

	printf("Only noise spectra:\n\n");

	/* 1 - Type SHORT */

	printf("Image creation (type of storage = Short) ...");
	fflush(stdout);
	create_tiger_frame(&image,"s_chk_tiger_io",npix,start,step,SHORT,
		"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<noise.npts;j++) {
			WR_spec(&noise,j,(short)(nolens*j));
		}
		put_tiger_spec(&image,NULL,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"s_chk_tiger_io","I");
	printf("Ok\n");
	nolens = (int)(nb_spec/2);
	status = exist_lens(&image,nolens);
	if (status < 0) {
		printf("unable to locate noise from lens no %d\n",nolens);
		fflush(stdout);
		return(-1);
	} 

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,NULL,&noise,nolens); 
		for (j=0;j<noise.npts;j++) {
			sval = (short)RD_spec(&noise,j);
			if (sval != (short)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image); 
	printf("Ok\n\n");

	/* 2 - Type LONG */

	printf("Image creation (type of storage = Long) ...");
	fflush(stdout);
	create_tiger_frame(&image,"l_chk_tiger_io",npix,start,step,LONG,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<noise.npts;j++) {
			WR_spec(&noise,j,(long)(nolens*j));
		}
		put_tiger_spec(&image,NULL,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"l_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,NULL,&noise,nolens); 
		for (j=0;j<noise.npts;j++) {
			lval = (long)RD_spec(&noise,j);
			if (lval != (long)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 3 - Type FLOAT */

	printf("Image creation (type of storage = Float) ...");
	fflush(stdout);
	create_tiger_frame(&image,"f_chk_tiger_io",npix,start,step,FLOAT,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<noise.npts;j++) {
			WR_spec(&noise,j,(float)(nolens*j));
		}
		put_tiger_spec(&image,NULL,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"f_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,NULL,&noise,nolens); 
		for (j=0;j<noise.npts;j++) {
			fval = (float)RD_spec(&noise,j);
			if (fval != (float)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 4 - Type DOUBLE */

	printf("Image creation (type of storage = Double) ...");
	fflush(stdout);
	create_tiger_frame(&image,"d_chk_tiger_io",npix,start,step,DOUBLE,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<noise.npts;j++) {
			WR_spec(&noise,j,(double)(nolens*j));
		}
		put_tiger_spec(&image,NULL,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"d_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,NULL,&noise,nolens); 
		for (j=0;j<noise.npts;j++) {
			dval = (double)RD_spec(&noise,j);
			if (dval != (double)(nolens*j)) {
				printf("FATAL : Unexpected values in image\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");
#endif
	printf("Signal & noise spectra:\n\n");

	/* 1 - Type SHORT */

	printf("Image creation (type of storage = Short) ...");
	fflush(stdout);
	create_tiger_frame(&image,"s_chk_tiger_io",npix,start,step,SHORT,
		"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(short)(nolens*j));
			WR_spec(&noise,j,(short)(nolens*j));
		}
		put_tiger_spec(&image,&signal,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"s_chk_tiger_io","I");
	printf("Ok\n");
	nolens = (int)(nb_spec/2);
	status = exist_lens(&image,nolens);
	if (status < 0) {
		printf("unable to locate signal from lens no %d\n",nolens);
		fflush(stdout);
		return(-1);
	} 

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,&noise,nolens); 
		for (j=0;j<signal.npts;j++) {
			sval = (short)RD_spec(&signal,j);
			if (sval != (short)(nolens*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			sval = (short)RD_spec(&noise,j);
			if (sval != (short)(nolens*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 2 - Type LONG */

	printf("Image creation (type of storage = Long) ...");
	fflush(stdout);
	create_tiger_frame(&image,"l_chk_tiger_io",npix,start,step,LONG,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(long)(nolens*j));
			WR_spec(&noise,j,(long)(nolens*j));
		}
		put_tiger_spec(&image,&signal,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"l_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,&noise,nolens); 
		for (j=0;j<signal.npts;j++) {
			lval = (long)RD_spec(&signal,j);
			if (lval != (long)(nolens*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			lval = (long)RD_spec(&noise,j);
			if (lval != (long)(nolens*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 3 - Type FLOAT */

	printf("Image creation (type of storage = Float) ...");
	fflush(stdout);
	create_tiger_frame(&image,"f_chk_tiger_io",npix,start,step,FLOAT,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(float)(nolens*j));
			WR_spec(&noise,j,(float)(nolens*j));
		}
		put_tiger_spec(&image,&signal,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"f_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,&noise,nolens); 
		for (j=0;j<signal.npts;j++) {
			fval = (float)RD_spec(&signal,j);
			if (fval != (float)(nolens*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			fval = (float)RD_spec(&noise,j);
			if (fval != (float)(nolens*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	/* 4 - Type DOUBLE */

	printf("Image creation (type of storage = Double) ...");
	fflush(stdout);
	create_tiger_frame(&image,"d_chk_tiger_io",npix,start,step,DOUBLE,"associated_tbl","dummy frame",NULL);
	printf("Ok\n");
	set_lenses_coord(&image,"XPOS","YPOS",nb_spec,nol,x,y);
	printf("Writing into created image ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		init_new_tiger_spec(&image,&signal,npix,start); 
		init_new_tiger_spec(&image,&noise,npix,start); 
		for (j=0;j<signal.npts;j++) {
			WR_spec(&signal,j,(double)(nolens*j));
			WR_spec(&noise,j,(double)(nolens*j));
		}
		put_tiger_spec(&image,&signal,&noise,nolens);
	}
	printf("Ok\n");

	printf("Saving created image ...");
	fflush(stdout);
	close_tiger_frame(&image);
	printf("Ok\n");

	printf("Opening previous image ...");
	fflush(stdout);
	open_tiger_frame(&image,"d_chk_tiger_io","I");
	printf("Ok\n");

	printf("Reading image values ...");
	fflush(stdout);
	for (nolens=1;nolens<=nb_spec;nolens++) {
		get_tiger_spec(&image,&signal,&noise,nolens); 
		for (j=0;j<signal.npts;j++) {
			dval = (double)RD_spec(&signal,j);
			if (dval != (double)(nolens*j)) {
				printf("FATAL : Unexpected values in signal\n");
				return(-1);
			}
			dval = (double)RD_spec(&noise,j);
			if (dval != (double)(nolens*j)) {
				printf("FATAL : Unexpected values in noise\n");
				return(-1);
			}
		}
	}
	printf("Ok\n");

	printf("Deleting previous image ...");
	fflush(stdout);
	delete_tiger_frame(&image);
	printf("Ok\n\n");

	exit_session(0);
	return(0);
}
