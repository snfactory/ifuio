/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        version.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     None
! PURPOSE      Utilities related to version control
! VERSION      4.0  2000-March-6 : Creation, AP   
______________________________________________________________________________*/

#include <string.h>

extern char soft_version[];

/*-----------------------------------------------------------------------------
!
!.blk           Miscellaneous routines to handle versions
!
!.func                             set_version()
!
!.purp      specify software version number
!.desc
! int set_version(no)
!
! char *no;             version number (exple 1.0, limited to 9 characters)
!.ed
------------------------------------------------------------------------------*/

int 
set_version(char *no)
{
	strcpy(soft_version,no);
	return(1);
}
