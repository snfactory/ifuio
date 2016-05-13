/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!  COPYRIGHT 	(c) 1997 Observatoire de Lyon - St Genis Laval (FRANCE)
!  IDENT	check_descr_list.c
!  LANGUAGE 	C
!
!  AUTHOR	A. Pecontal
!		
!  KEYWORDS	Simple programme to list file descriptors
!  PURPOSE 	
!  COMMENT      
!  VERSION 	1.0 13-10-1996 : Creation JLV
_________________________________________________________________*/

#include <IFU_io.h>

int main(int argc,char **argv)
{
  Anyfile *pt_obj;
  char **label,**param, **descr_list;
  char *file;
  int File_Type;
  int status, i;
  int nb_desc;

  set_arglist("-file none");
  init_session(argv,argc,&label,&param);

  file = param[0];
  
  File_Type = file_type(file);

  switch (File_Type)
    {
    case T_TIGER: 
	{
		E3D_file object;
		status = open_tiger_frame(&object,file,"I");
		pt_obj = (Anyfile *)&object;
		break;
	}

    case T_IMA1D: 
	{
		SPECTRUM object;
		status = header_spec(&object,file,"I");
		pt_obj = (Anyfile *)&object;
		break;	
	}

    case T_TABLE:
	{
		TABLE object;
		status = open_table(&object,file,"I");
		pt_obj = (Anyfile *)&object;
		break;	
	}
      
    case T_IMA2D:
	{
		IMAGE2D object;
		status = header_frame(&object,file,"I");
		pt_obj = (Anyfile *)&object;
		break;	
	}
      
    default:  
		status = -1;
    }
  
	if (status) {
		printf("Error opening file %s\n",param[0]);
		exit_session(ERR_OPEN);
	}

	nb_desc = get_all_desc(pt_obj,&descr_list);
	for (i=0; i<nb_desc; i++)
		printf("%s\n",descr_list[i]);

  switch (File_Type)
    {
    case T_TIGER: 
		status = close_tiger_frame((E3D_file *)pt_obj);
		break;

    case T_IMA1D: 
		status = close_spec((SPECTRUM *)pt_obj);
		break;	

    case T_TABLE:
		status = close_table((TABLE *)pt_obj);
		break;	
      
    case T_IMA2D:
		status = close_frame((IMAGE2D *)pt_obj);
		break;	
      
    default:  
		status = -1;
    }
	if (status) {
		printf("Error closing file %s\n",param[0]);
	}
  

  exit_session(0);   
  return(0);
}
