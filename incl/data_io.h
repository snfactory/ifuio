/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        data_io.h
! LANGUAGE     C
! 
! AUTHOR       A. Rousset
! 
! KEYWORDS     Param. relative to data storage 
! PURPOSE      
! COMMENT     
! VERSION      4.0  1994-March-10 : Creation    
______________________________________________________________________________*/

typedef struct
{
    short basic_io;
    short datacubes;

} IO_Format;

#define Quiet_Mode "-"

/* data format code */

#define MIDAS_FORMAT  (short)'M'
#define IRAF_FORMAT   (short)'I'
#define STSDAS_FORMAT (short)'S'
#define FITS_A_FORMAT (short)'B'	/* Basic Fits format (ASCII)    */
#define FITS_B_FORMAT (short)'O'	/* Original Fits format (BINARY)*/
#define TIGER_FORMAT (short)'T'
#define TIGER_FITS_FORMAT (short)'f'
#define TIGER_MIDAS_FORMAT (short)'m'
#define EURO3D_FORMAT  (short)'E'
#define DEFAULT_FORMAT (short)0

/* I/O modes */

#define I_MODE   400
#define O_MODE   200
#define IO_MODE  600

