/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT        parse_arg.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     None
! PURPOSE      Utilities to handle arguments
! VERSION      4.0  1994-May-31 : Creation, AR    
______________________________________________________________________________*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <IFU_datatypes.h>
#include <data_io.h>
#include <gendef.h>

#define OPTION_MAXSIZE 25
#define DEFVALUE_MAXSIZE 2048

int DEBUG = 0;
int VERBOSE = 1;
int TK = 0;
int PROGRESS = 20;
int ASK = 1;
int ASK_BACK = 0;

char **pt_ArgLabels = NULL;
char **pt_ArgValues = NULL;

char StandardArg_List[][OPTION_MAXSIZE] = {
	"-h",
	"-help",
	"-inputformat",
	"-outputformat",
	"-version",
	"-debug",
	"-tk",
	"-quiet",
	"-noask"
};								/* list of standard options           */
int  StandardList_Length=9;		/* length of previous list            */

char **Arg_List;				/* saved list of user'allowed options */
char **DefVal_List;				/* saved list of user'allowed options */
int  List_Length;				/* list length                        */

char *General_Purpose = NULL;

extern char soft_version[];

/*-----------------------------------------------------------------------------
!
!.blk           Miscellaneous routines to handle arguments
!
!.func                             set_arglist()
!
!.purp      specify list of options allowed and their default values
!.desc
! int set_arglist(arg_label)
!
! char *arg_label;      description string for options provided as :
!                       "-option1 def_value1 -option2 def_value2 ..."
!                       No default value means boolean option type.
!                       Use "none" if no default value is given.
!                       Default values may contain comma or blanks.
!.ed
------------------------------------------------------------------------------*/

int 
set_arglist(char *arg_label)
{
	char *pt_buf;
	char optlist[2048];
	char tmp_string[2048];

	List_Length = 0;

	strcpy(optlist,arg_label);
	pt_buf = optlist;
	Arg_List = (char **)malloc(sizeof(char *));
	DefVal_List = (char **)malloc(sizeof(char *));
	if ((Arg_List == (char **)0) || (Arg_List == (char **)0)) {
		Handle_Error("set_arglist : Unable to allocate memory",-1);
		return (-1);
	}

	while ( *pt_buf != '\0' ) {
			DefVal_List[List_Length] =
				(char *)malloc(DEFVALUE_MAXSIZE*sizeof(char));
			if (DefVal_List[List_Length] == (char *)0) {
				Handle_Error("set_arglist : Unable to allocate memory",-1);
				return (-1);
			}
											/* save option label */
			DefVal_List[List_Length][0] = '\0';
			if (sscanf(pt_buf," -%s",tmp_string) != 1) {
                Handle_Error("set_arglist : Syntax error",-1);
				return(-1);
			}
			Arg_List[List_Length] = 
				(char *)malloc((strlen(tmp_string)+2)*sizeof(char));
			if (Arg_List[List_Length] == (char *)0) {
				Handle_Error("set_arglist : Unable to allocate memory",-1);
				return (-1);
			}
			sprintf(Arg_List[List_Length],"-%s",tmp_string);
			pt_buf = strpbrk(pt_buf,Arg_List[List_Length])
					+ strlen(tmp_string);
			if (*pt_buf != '\0') pt_buf++;

 									/* scan default value until next option */
			while (sscanf(pt_buf," %s",tmp_string) == 1 ) { 
											
				if ((tmp_string[0] == '-') && !is_num(tmp_string[1]))
					break;
											/* decode default value */
				strcat(DefVal_List[List_Length],tmp_string);
				pt_buf = strpbrk(pt_buf,DefVal_List[List_Length])
					+ strlen(tmp_string);
				if (*pt_buf != '\0') pt_buf++;
				strcat(DefVal_List[List_Length]," ");
			}
			if (strlen(DefVal_List[List_Length]) != 0)
				DefVal_List[List_Length][strlen(DefVal_List[List_Length])-1] = '\0';
			else
				strcpy(DefVal_List[List_Length],"false");
			List_Length++;
			Arg_List = (char **)realloc(Arg_List,
				(List_Length+1)*sizeof(char *));
			DefVal_List = (char **)realloc(DefVal_List,
				(List_Length+1)*sizeof(char *));
		}
		return (0);
}

/*-----------------------------------------------------------------------------
!
!.func                             set_purpose()
!
!.purp      describes the general purpose of the program
!.desc
! int set_purpose(text)
!
! char *text;           description string
!.ed
------------------------------------------------------------------------------*/

int set_purpose(char *text) {

	General_Purpose = (char *)malloc((strlen(text)+1)*sizeof(char));
	strcpy(General_Purpose,text);
	return(0);
}

/*-----------------------------------------------------------------------------
!
!.func                             parse_arg()
!
!.purp        parse arguments, return values and decode the standard ones
!            the routine allocates the array of returned values by it's own
!.desc
! int parse_arg(argv, arg_no, arg_label, arg_value)
!
! char **argv;          parameter list passed to main program
! int  argno;           number of parameters 
! char ***arg_label;    labels of options ordered by appearence in
!                       the definition string (cf set_arglist()).
! char ***arg_value;    decoded values ordered by their appearance in
!                       the definition string (cf set_arglist()).
!.ed
------------------------------------------------------------------------------*/
#define set_arglabel(arglabel) \
	strcpy(pt_ArgLabels[j],arglabel);

#define set_argval(argvalue) \
	argvalue[0] = '\0'; \
	if (i+1 != argno) { \
    	if (argv[i+1][0] != '-') {\
			strcpy(argvalue,argv[i+1]); \
			i++; \
		} \
		else { \
    			if (is_num(argv[i+1][1])) {\
				strcpy(argvalue,argv[i+1]); \
				i++; \
			} \
		} \
	}

extern char Calling_Prog[];
int 
parse_arg(char **argv, int argno, char ***pt_arglabel, char ***pt_argval)
{
	int  i, j, k, nb_arg, status;
	char format[256], err_text[512],
		*pt_delim, *pt_arg, tmp_arg[DEFVALUE_MAXSIZE];
	int set_inputformat(char *), set_outputformat(char *);

	DEBUG = 0;
	TK = 0;

	nb_arg = List_Length;
	if (nb_arg !=0) {
		alloc2d(pt_arglabel,nb_arg,DEFVALUE_MAXSIZE,CHAR);
		alloc2d(pt_argval,nb_arg,DEFVALUE_MAXSIZE,CHAR);
	}
	pt_ArgLabels = *pt_arglabel;
	pt_ArgValues = *pt_argval;

	for (i=0; i<List_Length;i++) {           /* copy default values */
		strcpy(pt_ArgValues[i],DefVal_List[i]);
		strcpy(pt_ArgLabels[i],Arg_List[i]);
	}

	for (i=1; i<argno;i++) {		/* special care for -tk */
		if (strcmp(argv[i],"-tk") == 0)	{	/* option specified */
			TK = 1;
		}
	}
	for (i=1; i<argno;i++) {
		if (argv[i][0] == '-')	{	/* option specified */
									/* is it a standard option ? */
			for (j=0;j<StandardList_Length && 
				strcmp(StandardArg_List[j],argv[i]) != 0;j++);

			if (j == StandardList_Length) { 	/* non standard option      */
												/* is this option allowed ? */
				for (j=0;j<List_Length;j++) {
												/* multiple options */
					if ((pt_delim = strchr(Arg_List[j],'|')) != NULL) {
						pt_arg = &(Arg_List[j][1]);
						while (pt_delim != NULL) {
							strncpy(tmp_arg,pt_arg,(pt_delim - pt_arg));
							tmp_arg[pt_delim - pt_arg] ='\0';
							if (strcmp(tmp_arg,&(argv[i][1])) == 0) break;
							pt_arg = pt_delim+1;
							pt_delim = strchr(pt_arg,'|');
						}
						if (pt_delim == NULL) {
							if (strcmp(pt_arg,&(argv[i][1])) == 0) break;
						}
						else
							break;
					}
					else
						if (strcmp(Arg_List[j],argv[i]) == 0) break;
				}
				if (j == List_Length) {	        /* unknown option     */
					sprintf(err_text,"parse_arg : option %s ",argv[i]);
					Handle_Error(err_text,-1);
					return(-1);				
				}
				set_arglabel(argv[i]);	 		/* got label */
				set_argval(pt_ArgValues[j]);	 	/* got value */
				if (pt_ArgValues[j][0] == '\0') {
					if (strcmp(DefVal_List[j],"false") == 0)
						strcpy(pt_ArgValues[j],"true");
					if (strcmp(DefVal_List[j],"none") == 0)
						strcpy(pt_ArgValues[j],DefVal_List[j]);
				}
			}
			else {						/* handle standard option */
				switch (j) {
				case 0 :				/* display help */
				case 1 :				/* display help */
					print_msg("available options :");
					err_text[0] = '\0';
					for (k=0;k<List_Length; k++) {
						if (strcmp(DefVal_List[k],"none") == 0) {
							strcat(err_text,Arg_List[k]);
							strcat(err_text," nodefault ");
						}
						else {
							strcat(err_text,"[");
							strcat(err_text,Arg_List[k]);
							if (strcmp(DefVal_List[k],"false") != 0) {
								strcat(err_text," ");
								strcat(err_text,DefVal_List[k]);
								strcat(err_text," ");
							}
							strcat(err_text,"] ");
						}
					}
					for (k=0;k<StandardList_Length; k++) {
						strcat(err_text,"[");
						strcat(err_text,StandardArg_List[k]);
						strcat(err_text,"] ");
					}
					print_msg(err_text);

					if (General_Purpose != NULL)
						print_msg("\nPurpose :\n%s\n",General_Purpose);

					exit(0);    			/* only want's help */
					break;

				case 2 :				/* set input format */
					set_argval(format);
					status = set_inputformat(format);
					if (status) {
						return(status);
					};
					break;

				case 3 :				/* set output format */
					set_argval(format);
					status = set_outputformat(format);
					if (status) {
						return(status);
					};
					break;

				case 4 :				/* print software version */
					printf("%s-%s\n",soft_version,VERSION);
					exit(0);   			/* only want's version */
					break;

				case 5 :				/* set debug mode */
					DEBUG = 1;
					break;

				case 6 :				/* set TK mode */
					TK = 1;
					break;

				case 7 :				/* set quiet mode */
					VERBOSE = 0;
					strcpy(Calling_Prog,Quiet_Mode);
					break;

				case 8 :
				        ASK = 0;
				        break;

				default :
					sprintf(err_text,"parse_arg : Unknown option %s",argv[i]);
					Handle_Error(err_text,-1);
					return(-1);
					break;
				}
			}
		}
	}
	for (i=0; i<List_Length;i++) {
		free(Arg_List[i]);
		free(DefVal_List[i]);
	}
	free(Arg_List);
	free(DefVal_List);

	for (i=0; i<List_Length;i++) 
		if (strcmp(pt_ArgValues[i],"none") == 0) {	
			pt_ArgValues[i][0] = '\0';
			sprintf(err_text,"parse_arg : option %s : value must be specified ",
					pt_ArgLabels[i]);
			Handle_Error(err_text,ERR_NB_PARAM);
			return(-1);					/* unknown option     */
		}
	return(0);
}

/*-----------------------------------------------------------------------------
!
!.func                             get_argval()
!
!.purp       decode the value of the ith argument using given scan format
!            and returns if the scan was OK. Otherwise, display error
!            message and exit
!.desc
! void get_argval(i, format, value)
!
! int  i;               argument indix (from 0 to n)
! char *format;         decoding format (like in scanf)
! void *value;          decoded value
!.ed
------------------------------------------------------------------------------*/
void get_argval(int i, char *format, void *value)
{
	char errtext[132];
	int exit_session(int);

    if (sscanf(pt_ArgValues[i],format,value) != 1) {
        sprintf(errtext,"Inconsistent value found for %s. Got %s",
            pt_ArgLabels[i],pt_ArgValues[i]); 
        print_error(errtext);
        exit_session(ERR_BAD_PARAM);
    }
}

/*-----------------------------------------------------------------------------
                       functions defined in macro.h
!
!.func                             is_true()
!
!.purp       returns true if specified option is set, false if not 
!.desc
! boolean is_true(option_val)
!
! char *option_val;     option value returned by parse_arg()
!.ed
!
!.func                             is_false()
!
!.purp       returns false if specified option is set, true if not 
!.desc
! boolean is_false(option_val)
!
! char *option_val;     option value returned by parse_arg()
!.ed
!
!.func                             is_set()
!
!.purp   returns true if the option is set (i.e. its value is not NULL)
!.desc
! boolean is_set(option_val)
!
! char *option_val;     option value returned by parse_arg()
!.ed
!
!.func                             is_unset()
!
!.purp   returns true if the option is not set (i.e. its value is NULL)
!.desc
! boolean is_unset(option_val)
!
! char *option_val;     option value returned by parse_arg()
!.ed
------------------------------------------------------------------------------*/
