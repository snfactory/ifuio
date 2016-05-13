/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_selection.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check interfaces with Midas 
! COMMENT      
! VERSION      4.0  1993-May-12 : Creation, AR
-----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <IFU_io.h>

/*-----------------------------------------------------------------------------
!
!.blk               Checking of the I/O library
!
!.prog                             check_selection()
!
!.purp      	   Checks each table i/O routines with selection enable
!
-----------------------------------------------------------------------------*/
#define NBROW 1000
#define NBCOL 5

int
main(int argc, char **argv)
{
	int nocol_short, nocol_long, nocol_float, nocol_double;
	int status;
	char **argval, **arglabel;
	int i, nbrow = NBROW, nbcol=NBCOL, sel[NBROW];
	short sval, s_colbuf[NBROW], s_key;
	long lval, l_colbuf[NBROW];
	float fval, f_colbuf[NBROW];
	double dval, d_colbuf[NBROW];
	char ident[20];
	char selection[30];
	TABLE table;

						/*  init_session() checking */	


	printf("IOLIB environment routines :\n\n");
	fflush(stdout);
	init_session(argv,argc,&arglabel,&argval);
/* 	set_control_level(WARNING); */

	printf("Table creation ...");
	fflush(stdout);
	strcpy(ident,"Dummy table");
	create_table(&table,"chk_tbl_io",nbrow,nbcol,'W',ident);
	printf("Ok\n");
	fflush(stdout);
/*
	printf("Column creation ...(format Characters) ");
	fflush(stdout);
	nocol_char = create_col(&table,":TEXT",CHAR,'N',"A20",NULL);
	if (nocol_char < 0) {
		printf("Problem creating column TEXT, status returned = %d\n", nocol_char);
		exit (-1);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created column ...");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		sprintf(text,"String no %2d",i);
		WR_tbl(&table,i,nocol_char,text);
	}
	printf("Ok\n");
	fflush(stdout);
*/
	printf("Column creation ...(format Short) ");
	fflush(stdout);
	nocol_short = create_col(&table,":SHORT",SHORT,'N',"I2",NULL);
	if (nocol_short < 0) {
		printf("Problem creating column SHORT, status returned = %d\n", nocol_short);
		exit (-1);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created column ...");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		sval = (short)i;
		WR_tbl(&table,i,nocol_short,&sval);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Column creation ...(format Long) ");
	fflush(stdout);
	nocol_long = create_col(&table,":LONG",LONG,'N',"I4",NULL);
	if (nocol_long < 0) {
		printf("Problem creating column LONG, status returned = %d\n", nocol_long);
		exit (-1);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created column ...");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		lval = (long)i;
		WR_tbl(&table,i,nocol_long,&lval);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Column creation ...(format Float) ");
	fflush(stdout);
	nocol_float = create_col(&table,":FLOAT",FLOAT,'N',"F9.6",NULL);
	if (nocol_float < 0) {
		printf("Problem creating column FLOAT, status returned = %d\n", nocol_float);
		exit (-1);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created column ...");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		fval = (float)i;
		WR_tbl(&table,i,nocol_float,&fval);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Column creation ...(format Double) ");
	fflush(stdout);
	nocol_double = create_col(&table,":DOUBLE",DOUBLE,'N',"E15.5",NULL);
	if (nocol_double < 0) {
		printf("Problem creating column DOUBLE, status returned = %d\n", nocol_double);
		exit (-1);
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Writing into created column ...");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		dval = (double)i;
		WR_tbl(&table,i,nocol_double,&dval);
	}
	printf("Ok\n\n");
	fflush(stdout);

	for (i=0; i<nbrow; i++) {
		if (i%2 == 0)
			sel[i] = 0;
		else
			sel[i] = 1;
	}
	write_selection(&table,sel,"check_selection");

	printf("Saving created table ...");
	fflush(stdout);
	close_table(&table);
	printf("Ok\n");
	fflush(stdout);

	printf("Opening previous table ...");
	fflush(stdout);
	open_table(&table,"chk_tbl_io","I");
	printf("Ok\n");
	fflush(stdout);

	printf("Handling selection ...");
	fflush(stdout);
	handle_select_flag(&table,'W',selection);
	printf("Ok\n");
	fflush(stdout);

	printf("Reading columns info ...");
	fflush(stdout);
/*	nocol_char = get_col_ref(&table,":TEXT"); */
	nocol_short = get_col_ref(&table,":SHORT");
	nocol_long = get_col_ref(&table,":LONG");
	nocol_float = get_col_ref(&table,":FLOAT");
	nocol_double = get_col_ref(&table,":DOUBLE");
	printf("Ok\n");
	nbrow = table.row;
/*
	printf("Reading previous column ...(format Characters) ");
	fflush(stdout);
	RD_col(&table,nocol_char,c_colbuf);
	for (i=0; i<nbrow; i++) {
		sprintf(text,"String no %2d",i);
		if (strcmp(text,(c_colbuf+i*21)) != 0) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");

	printf("Reading previous column item by item  ...(format Characters) ");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		RD_tbl(&table,i,nocol_char,Rtext);
		sprintf(text,"String no %2d",i);
		if (strcmp(text,Rtext) != 0) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			printf("*%s* is not *%s*\n", Rtext, text);
	 	return(-1); 
		}
	}
	printf("Ok\n");
*/
	printf("Reading previous column ...(format Short) ");
	fflush(stdout);
	s_colbuf[0] = 0;
	RD_col(&table,nocol_short,s_colbuf);
	printf("Ok\n");

	printf("Searching for no 501 ... ");
	fflush(stdout);
	
	s_key = 501;
	status = search_in_col(&table,nocol_short,&s_key);
	if (status >= 0)
		printf("Found in line %d\n",status);
	else {
		printf("Unable to find it\n");
		return(-1);
	}
	
	printf("Searching for no 500 ... ");
	fflush(stdout);
	
	s_key = 500;
	status = search_in_col(&table,nocol_short,&s_key);
	if (status >= 0)
		printf("Found in line %d\n",status);
	else {
		printf("Unable to find it (not selected)\n");
	}

	for (i=0; i<nbrow; i++) {
		if (s_colbuf[i] != (short)(2*i+1)) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			printf("Read %d, Expected %d\n",s_colbuf[i],2*i+1);
			return(-1);
		}
	}
	printf("Ok\n");
	printf("Reading previous column item by item ...(format Short) ");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		RD_tbl(&table,i,nocol_short,&sval);
		if (sval != (short)(2*i+1)) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");

	printf("Reading previous column ..(format Long) ");
	fflush(stdout);
	RD_col(&table,nocol_long,l_colbuf);
	for (i=0; i<nbrow; i++) {
		if (l_colbuf[i] != (long)(2*i+1)) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");
	printf("Reading previous column item by item ...(format Long) ");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		RD_tbl(&table,i,nocol_long,&lval);
		if (lval != (long)(2*i+1)) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");

	printf("Reading previous column ...(format Float) ");
	fflush(stdout);
	RD_col(&table,nocol_float,f_colbuf);
	for (i=0; i<nbrow; i++) {
		if (f_colbuf[i] != (float)(2*i+1)) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");
	printf("Reading previous column item by item ...(format Float) ");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		RD_tbl(&table,i,nocol_float,&fval);
		if (fval != (float)(2*i+1)) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");

	printf("Reading previous column ..(format Double) ");
	fflush(stdout);
	RD_col(&table,nocol_double,d_colbuf);
	for (i=0; i<nbrow; i++) {
		if (ABS(d_colbuf[i] - (double)(2*i+1)) > 1e-8) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");
	printf("Reading previous column item by item ...(format Double) ");
	fflush(stdout);
	for (i=0; i<nbrow; i++) {
		RD_tbl(&table,i,nocol_double,&dval);
		if (ABS(dval - (double)(2*i+1)) > 1e-8) {
			printf("FATAL : Unexpected values in table, line %d\n",i);
			return(-1);
		}
	}
	printf("Ok\n");
	fflush(stdout);

	printf("Deleting previous table ...");
	fflush(stdout);
	delete_table(&table);
	printf("Ok\n");
	fflush(stdout);

	exit_session(0);
	return(0);
}
