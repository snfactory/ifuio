/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_file_format.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check input file format
! COMMENT      
! VERSION      4.0  1993-May-11 : Creation, AR
------------------------------------------------------------------------------*/

#include <IFU_io.h>
#include <data_io.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Checking the way we derive file format 
!
!.prog                             check_file_format()
!
!.purp 
!
-----------------------------------------------------------------------------*/

char *file_format(char *);

int main(int argc, char **argv)
{
	char **argval, **arglabel;
	char *format;
	int type;

	set_arglist("-if none");
	init_session(argv,argc,&arglabel,&argval);

	format = file_format(argval[0]);
	printf("File format : ");
	switch (format[0]) {
		case TIGER_FORMAT : printf("Tiger file\n");
			switch (format[1]) {
				case T_TIGER : printf("Datacube\n");
					break;
				case T_TIGMAX : printf("Maxima\n");
					break;
			}
			exit_session(0);
			break;
		case MIDAS_FORMAT : printf("Midas\n");
			break;
		case FITS_B_FORMAT : printf("FITS\n");
			break;
		case IRAF_FORMAT : printf("Iraf\n");
			break;
		case STSDAS_FORMAT : printf("Iraf\n");
			break;
		default : printf("unknown\n");
			break;
	}
	printf("File type : ");
	switch (format[1]) {
		case T_IMA1D : 
		case T_IMA2D : 
		case T_IMA3D : 
			printf("Image\n");
			break;
		case T_TABLE : 
			printf("Table\n");
			break;
		default : printf("unknown\n");
			break;
	}
	type = file_type(argval[0]);
	switch (type) {
		case T_TABLE : printf("Standard Table\n");
			break;
		case T_IMA1D : printf("Spectrum\n");
			break;
		case T_IMA2D : printf("2D Image\n");
			break;
		case T_IMA3D : printf("3D Image\n");
			break;
		default : printf("Unknown\n");
			break;
	}
	exit_session(0);
}

