/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        io_error.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     
! PURPOSE      I/O errors handling according to user settings
! COMMENT      
! VERSION      4.0  1992-June-15 : Creation AR 
---------------------------------------------------------------------*/

#include <stdlib.h>
#include <IFU_io.h>
#include <data_io.h>

extern int TK, ASK;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.blk                   		Handling ERRORS
!							
!.func                     set_control_level()
!							
!.purp              To define user's control level 
!
!.desc
! void set_control_level(level)
! int level;
!             FATAL (default value) causes program to exit on error.
!             WARNING display warning message and return status ( <> 0)
!             NONE return status ( <> 0)	
!.ed								
-------------------------------------------------------------------- */

static int *Error_Control_Level = NULL;   /* error control level */
static int Error_Current_Level = -1;
static int Erase_File_Sav = 0;        /* save of file erase flag */

void 
set_control_level(int level)	
{
	Error_Current_Level++;
	if (Error_Control_Level == NULL) {
		Error_Control_Level = (int *)malloc(1*sizeof(int));
	}
	else {
		Error_Control_Level = (int *)realloc((char *)Error_Control_Level,(Error_Current_Level+1)*sizeof(int));
	}
	Error_Control_Level[Error_Current_Level] = level;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func	                     Handle_Error() 
!
!.purp    defines what to do according to set level control.			
!					
!.desc					
! void Handle_Error(routine,status) 
! char *routine;        name of the routine in which error occured
! int status;           status code
!.ed					
-------------------------------------------------------------------- */

void 
Handle_Error(char *routine, int status) 
{
	char errtext[512];
	int level;

	if (Error_Control_Level != NULL) {
		level = Error_Control_Level[Error_Current_Level];
	}
	else
		level = FATAL;

	switch (level) {  
					/* What to do when an error occurs ? */
	case FATAL :		/* causes program to exit */  
	
		sprintf(errtext,"FATAL error from routine %s", routine);
		print_error(errtext);
		strcpy(errtext,"\0");
		get_tiger_errmsg(status,errtext);
		print_error(errtext);
		exit_session(status);
		exit(1);

	case WARNING :		/* display warning */  
	
		sprintf(errtext,"WARNING from routine %s", routine);
		print_warning(errtext);
		get_tiger_errmsg(status,errtext);
		print_warning(errtext);
		break;

	case NONE :		/* return status */  
		break;
	
	default : 		/* it's up to the user to handle the error */
	    break;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func	                     disable_user_warnings() 
!
!.purp    	       disables user preferences to handle errors
!					
!.desc					
! void disable_user_warnings() 
!.ed					
-------------------------------------------------------------------- */
void 
disable_user_warnings()	
{
	set_control_level(NONE);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func	                     restore_user_warnings() 
!
!.purp    	       restores user preferences to handle errors
!					
!.desc					
! void restore_user_warnings() 
!.ed					
-------------------------------------------------------------------- */
void 
restore_user_warnings()	
{
	Error_Current_Level--;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func	                    disable_erase_flag() 
!
!.purp    	      disables user preferences to earase flag
!					
!.desc					
! void disable_erase_flag() 
!.ed					
-------------------------------------------------------------------- */
void 
disable_erase_flag()	
{
  Erase_File_Sav = ASK;
  ASK = 0;
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!				                                                      
!.func	                    restore_erase_flag() 
!
!.purp    	      disables user preferences to earase flag
!					
!.desc					
! void restore_erase_flag() 
!.ed					
-------------------------------------------------------------------- */
void 
restore_erase_flag()	
{
  ASK =  Erase_File_Sav;	
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                   get_tiger_errcode()
!
!.purp        returns the error code according to Tiger conventions
!
!.desc
! int get_tiger_errcode(data_format,stat)
! short data_format;    data format
! int stat;             error status code 
!
!.ed			
-------------------------------------------------------------------- */
int 
get_tiger_errcode(short data_format, int stat)
{
	switch (data_format) {

	case FITS_A_FORMAT :
	case FITS_B_FORMAT :
	case EURO3D_FORMAT :
		switch(stat) {
             case  101:
             case  103:
             case  104:
				return(ERR_OPEN); break;
             case  105:   
				return(ERR_CREAT); break;
             case  106: 
				return(ERR_WRIT); break;
             case  107:   
             case  108: 
				return(ERR_READ); break;
             case  110:   
				return(ERR_CLOSE); break;
             case  111: 
				return(ERR_ALLOC); break;
             case  112:  
				return(ERR_ACCESS); break;
             case  201:   
				return(ERR_BAD_HEAD); break;
             case  202:   
				return(ERR_NODESC); break;
             case  203:   
				return(ERR_HEADER_SIZE); break;
             case  204:  
             case  205:  
             case  207: 
             case  208: 
             case  209: 
             case  210: 
             case  211:  
             case  212: 
             case  213:
             case  214:
             case  215:
             case  216:
             case  217: 
             case  218: 
				return(ERR_BAD_DESC); break;
             case  219:   
				return(ERR_NOCOL); break;
             case  220:  
				return(ERR_BAD_DESC); break;
             case  221: 
             case  222: 
             case  223: 
             case  224: 
             case  225: 
             case  226:  
             case  227: 
             case  228: 
             case  229: 
             case  230: 
             case  231: 
             case  232: 
				return(ERR_BAD_HEAD); break;
             case  233: 
				return(ERR_IMA_HEAD); break;
             case  234:  
				return(ERR_BAD_DESC); break;
             case  235:  
				return(ERR_TBL_HEAD); break;
             case  236:   
				return(ERR_HEADER_SIZE); break;
             case  237:  
				return(ERR_NOCOL); break;
             case  241:   
				return(ERR_BAD_DESC); break;
             case  251: 
             case  252: 
             case  253: 
             case  261:
             case  262:
             case  263: 
             case  301:  
				return(ERR_BAD_HEAD); break;
             case  302:   
				return(ERR_COL_NUM); break;
             case  304:
             case  306:
				return(ERR_NODATA); break;
             case  307:  
             case  308: 
				return(ERR_BAD_PARAM); break;
             case  309:
             case  310: 
             case  311:  
             case  312:   
				return(ERR_BAD_TYPE); break;
             case  314:   
				return(ERR_NODATA); break;
             case  317:  
             case  320: 
             case  321: 
				return(ERR_BAD_PARAM); break;
             case  322:  
             case  323: 
				return(ERR_BAD_DESC); break;
             case  401:  
             case  402: 
             case  403:
             case  404:
             case  405:
             case  406: 
             case  407: 
             case  408: 
             case  409: 
             case  410: 
				return(ERR_BAD_TYPE); break;
             case  411:   
				return(ERR_BAD_PARAM); break;
             case  412: 
				return(ERR_OVERFLOW); break;
             case  501: 
             case  502:
             case  503:
				return(ERR_BAD_PARAM); break;
             case  505:   
				return(ERR_BAD_DESC); break;
			default : return(stat);
		}
			break;

	case MIDAS_FORMAT :
		switch(stat) {

			case -4 : return(ERR_NODATA); break;
			case -6 : return(ERR_NODATA); break;
			case  0 : return(OK); break;
			case  1 : return(ERR_NODESC); break;
			case  2 : return(ERR_OVERFLOW); break;
			case  6 : return(ERR_ACCESS); break;
			case  7 : return(ERR_BAD_PARAM); break;
			case  8 : return(ERR_OVERFLOW); break;
			case  9 : return(ERR_BAD_DESC); break;
			case 15 : return(ERR_BAD_CAT); break;
			case 20 : return(ERR_OVERFLOW); break;
			case 21 : return(ERR_ALLOC); break;
			case 22 : return(ERR_ALLOC); break;
			case 23 : return(ERR_OVERFLOW); break;
			case 24 : return(ERR_NOTBL); break;
			case 25 : return(ERR_COL_NUM); break;
			case 26 : return(ERR_ROW_NUM); break;
			case 27 : return(ERR_NOIDENT); break;
			case 28 : return(ERR_BAD_COL); break;
			case 29 : return(ERR_NOIMPL); break;
			case 31 : return(ERR_REN_TBL); break;
			case 32 : return(ERR_NOCOL); break;
			default : return(stat);
		}
		break;
	case STSDAS_FORMAT :
	case IRAF_FORMAT :
		switch(stat) {
			case -2 : return(ERR_EOF); break;
			case  0 : return(OK); break;
	        case  6 : return(ERR_BAD_PARAM); break;
	        case  7 : return(ERR_NB_PARAM); break;
	        case 10 : return(ERR_OPEN); break;
	        case 11 : return(ERR_CREAT); break;
	        case 12 : return(ERR_CREAT); break;
	        case 13 : return(ERR_NAXIS); break;
	        case 14 : return(ERR_NAXIS); break;
	        case 15 : return(ERR_BAD_TYPE); break;
	        case 16 : return(ERR_OFFSET); break;
	        case 17 : return(ERR_ACCESS); break;
	        case 18 : return(ERR_CLOSE); break;
	        case 19 : return(ERR_BAD_IMA); break;
	        case 20 : return(ERR_IMA_BOUND); break;
	        case 21 : return(ERR_IMA_EXT); break;
	        case 22 : return(ERR_IMA_BOUND); break;
	        case 23 : return(ERR_READ); break;
	        case 24 : return(ERR_WRIT); break;
	        case 25 : return(ERR_NODESC); break;
	        case 26 : return(ERR_NODESC); break;
	        case 40 : return(ERR_NODESC); break;
	        case 41 : return(ERR_BAD_HEAD);break;
	        case 42 : return(ERR_HEADER_SIZE); break;
	        case 43 : return(ERR_BAD_PARAM); break;
	        case 46 : return(ERR_BAD_PARAM); break;
	        case 47 : return(ERR_NB_PARAM); break;
	        case 48 : return(ERR_DEL_DESC); break;
	        case 49 : return(ERR_DEL_DESC); break;
	        case 50 : return(ERR_NOIMA); break;
	        case 51 : return(ERR_DEL_IMA); break;
	        case 52 : return(ERR_REN_IMA); break;
	        case 53 : return(ERR_IMA_HEAD); break;
	        case 54 : return(ERR_BAD_DESC); break;
	        case 70 : return(ERR_GRAPH_DEV); break;
	        case 101 : return(ERR_ALLOC); break;
	        case 102 : return(ERR_FREE); break;
	        case 112 : return(ERR_BAD_SIZE); break;
			default : return(stat); break;
		}
		break;
	case TIGER_FORMAT :
		return(stat);
		break;
	}
	return(ERR_FORMAT);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!
!.func                   get_tiger_errmsg()
!
!.purp     returns the error message corresponding to the given code
!
!.desc
! void get_tiger_errmsg(stat,msg)
! int stat;             error status code 
! char *msg;            corresponding error message
!
!.ed			
-------------------------------------------------------------------- */
void 
get_tiger_errmsg(int stat, char *msg)
{
  if(!TK)
    {
	switch(stat) {
	case OK : strcpy(msg,"Successful return\n"); break;
	case UNKNOWN : msg=NULL; break;
	case ERR_EOF : strcpy(msg,"EOF reached\n"); break;
        case ERR_BAD_PARAM : strcpy(msg,"Input element is bad\n"); break;
        case ERR_NB_PARAM : strcpy(msg,"Bad input number of elements\n"); break;
        case ERR_BAD_TYPE : strcpy(msg,"Invalid data type\n"); break;
        case ERR_BAD_SIZE : strcpy(msg,"Field width not wide enough\n"); break;
        case ERR_OPEN : strcpy(msg,"Error opening file\n"); break;
        case ERR_CREAT : strcpy(msg,"Error opening new file\n"); break;
        case ERR_READ : strcpy(msg,"Error reading file\n"); break;
        case ERR_WRIT : strcpy(msg,"Error writing to file\n"); break;
        case ERR_CLOSE : strcpy(msg,"Error closing file\n"); break;
        case ERR_ACCESS : strcpy(msg,"Invalid image access mode\n"); break;
        case ERR_NAXIS : strcpy(msg,"Invalid NAXIS parameter\n"); break;
        case ERR_NOIDENT : strcpy(msg,"Identifier not found\n"); break;
        case ERR_OFFSET : strcpy(msg,"Error returning offset to data\n"); break;
	case ERR_NODATA : strcpy(msg,"No data available\n"); break;
        case ERR_BAD_IMA : strcpy(msg,"Not an image !\n"); break;
        case ERR_IMA_BOUND : strcpy(msg,"Bad section specification for image\n"); break;
        case ERR_IMA_EXT : strcpy(msg,"Bad extension for image\n"); break;
        case ERR_NOIMA : strcpy(msg,"Image does not exist\n"); break;
        case ERR_DEL_IMA : strcpy(msg,"Error deleting image\n"); break;
        case ERR_REN_IMA : strcpy(msg,"Error renaming image\n"); break;
        case ERR_IMA_HEAD : strcpy(msg,"Illegal image header\n"); break;
        case ERR_BAD_TBL : strcpy(msg,"Not a table !\n"); break;
        case ERR_TBL_EXT : strcpy(msg,"Bad extension for table\n"); break;
        case ERR_NOTBL : strcpy(msg,"Table does not exist\n"); break;
        case ERR_DEL_TBL : strcpy(msg,"Error deleting table\n"); break;
        case ERR_REN_TBL : strcpy(msg,"Error renaming table\n"); break;
        case ERR_TBL_HEAD : strcpy(msg,"Illegal table header\n"); break;
        case ERR_BAD_COL : strcpy(msg,"Error in column format\n"); break;
        case ERR_NOCOL : strcpy(msg,"Column does not exist\n"); break;
        case ERR_COL_NUM : strcpy(msg,"Wrong column number\n"); break;
        case ERR_ROW_NUM : strcpy(msg,"Wrong row number\n"); break;
        case ERR_NODESC : strcpy(msg,"Header parameter not found\n"); break;
        case ERR_BAD_HEAD : strcpy(msg,"Illegal data type for header parameter\n"); break;
        case ERR_DEL_DESC : strcpy(msg,"Cannot delete descriptor\n"); break;
        case ERR_BAD_DESC : strcpy(msg,"Descriptor bad\n"); break;
        case ERR_HEADER_SIZE : strcpy(msg,"Out of space in header\n"); break;
        case ERR_BAD_CAT : strcpy(msg,"Not a catalog !\n"); break;
        case ERR_GRAPH_DEV : strcpy(msg,"Bad graphics device\n"); break;
        case ERR_ALLOC : strcpy(msg,"Error allocating dynamic memory\n"); break;
        case ERR_FREE : strcpy(msg,"Error freeing dynamic memory\n"); break;
	case ERR_NOIMPL : strcpy(msg,"Not yet implemented\n"); break;
	case ERR_OVERFLOW : strcpy(msg,"Overflow (column/frame)\n"); break;
	case ERR_FORMAT : strcpy(msg,"Unknown data format\n"); break;
	default : msg=NULL; break;
	}
    } else {
      	switch(stat) {
	case OK : strcpy(msg,"Successful return"); break;
	case UNKNOWN : msg=NULL; break;
	case ERR_EOF : strcpy(msg,"EOF reached"); break;
        case ERR_BAD_PARAM : strcpy(msg,"Input element is bad"); break;
        case ERR_NB_PARAM : strcpy(msg,"Bad input number of elements"); break;
        case ERR_BAD_TYPE : strcpy(msg,"Invalid data type"); break;
        case ERR_BAD_SIZE : strcpy(msg,"Field width not wide enough"); break;
        case ERR_OPEN : strcpy(msg,"Error opening file"); break;
        case ERR_CREAT : strcpy(msg,"Error opening new file"); break;
        case ERR_READ : strcpy(msg,"Error reading file"); break;
        case ERR_WRIT : strcpy(msg,"Error writing to file"); break;
        case ERR_CLOSE : strcpy(msg,"Error closing file"); break;
        case ERR_ACCESS : strcpy(msg,"Invalid image access mode"); break;
        case ERR_NAXIS : strcpy(msg,"Invalid NAXIS parameter"); break;
        case ERR_NOIDENT : strcpy(msg,"Identifier not found"); break;
        case ERR_OFFSET : 
			strcpy(msg,"Error returning offset to data"); break;
		case ERR_NODATA : strcpy(msg,"No data available"); break;
        case ERR_BAD_IMA : strcpy(msg,"Not an image !"); break;
        case ERR_IMA_BOUND : 
			strcpy(msg,"Bad section specification for image"); break;
        case ERR_IMA_EXT : strcpy(msg,"Bad extension for image"); break;
        case ERR_NOIMA : strcpy(msg,"Image does not exist"); break;
        case ERR_DEL_IMA : strcpy(msg,"Error deleting image"); break;
        case ERR_REN_IMA : strcpy(msg,"Error renaming image"); break;
        case ERR_IMA_HEAD : strcpy(msg,"Illegal image header"); break;
        case ERR_BAD_TBL : strcpy(msg,"Not a table !"); break;
        case ERR_TBL_EXT : strcpy(msg,"Bad extension for table"); break;
        case ERR_NOTBL : strcpy(msg,"Table does not exist"); break;
        case ERR_DEL_TBL : strcpy(msg,"Error deleting table"); break;
        case ERR_REN_TBL : strcpy(msg,"Error renaming table"); break;
        case ERR_TBL_HEAD : strcpy(msg,"Illegal table header"); break;
        case ERR_BAD_COL : strcpy(msg,"Error in column format"); break;
        case ERR_NOCOL : strcpy(msg,"Column does not exist"); break;
        case ERR_COL_NUM : strcpy(msg,"Wrong column number"); break;
        case ERR_ROW_NUM : strcpy(msg,"Wrong row number"); break;
        case ERR_NODESC : strcpy(msg,"Header parameter not found"); break;
        case ERR_BAD_HEAD : 
			strcpy(msg,"Illegal data type for header parameter"); break;
        case ERR_DEL_DESC : strcpy(msg,"Cannot delete descriptor"); break;
        case ERR_BAD_DESC : strcpy(msg,"Descriptor bad"); break;
        case ERR_HEADER_SIZE : strcpy(msg,"Out of space in header"); break;
        case ERR_BAD_CAT : strcpy(msg,"Not a catalog !"); break;
        case ERR_GRAPH_DEV : strcpy(msg,"Bad graphics device"); break;
        case ERR_ALLOC : strcpy(msg,"Error allocating dynamic memory"); break;
        case ERR_FREE : strcpy(msg,"Error freeing dynamic memory"); break;
	case ERR_NOIMPL : strcpy(msg,"Not yet implemented"); break;
	case ERR_OVERFLOW : strcpy(msg,"Overflow (column/frame)"); break;
	case ERR_FORMAT : strcpy(msg,"Unknown data format\n"); break;
	default : msg = NULL; break;
	}
    }
}
