/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        check_args_parsing.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     
! PURPOSE      Facility to check options specification and argument parsing
! COMMENT      
! VERSION      4.0  1994-Jun-03 : Creation, AR
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
!
!.blk               Checking of the options specification interface 
!
!.prog                             check_args_parsing()
!
!.purp      		Checks options specification and argument parsing
!
-----------------------------------------------------------------------------*/
#include <IFU_io.h>

int main(int argc, char **argv)
{
	int i;
	char **arg_label;
	char **arg_val;

	set_arglist("-option1 -123.4 -option2|option2b|option2c -option3 none -option4|option4b 30mm filter -option5");
	init_session(argv,argc,&arg_label,&arg_val);
	for (i=0; i<5; i++)
		print_msg("%s %s",arg_label[i],arg_val[i]);

	if (DEBUG) {
		for (i=0; i<5; i++)
			fprintf(stderr,"%s %s\n",arg_label[i],arg_val[i]);
	}
	if (VERBOSE) {
		print_msg("Verbose mode ON");
    }
	else {
		print_msg("Verbose mode OFF");
	}
	exit_session(0);
	return(0);
}
