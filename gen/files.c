/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1993 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        files.c
! LANGUAGE     C
! AUTHOR       A. Rousset
! KEYWORDS     None
! PURPOSE      Facilities for files handling
! VERSION      4.0  07-May-1993 AP Creation    
!              4.1  16-Sep-2005 YC remove_path: strings may not overlap in strcpy 
______________________________________________________________________________*/

#include <stdlib.h>
#include <gendef.h>
#include <unistd.h>
#include <sys/stat.h>

/*-----------------------------------------------------------------------------
!
!.blk           Miscellaneous routines to handle files and directories
!
!.func                             exist()
!
!.purp              checks existence of file or directory
!.desc
! int exist(file)
!
! char *file;           filename
!.ed
------------------------------------------------------------------------------*/

int 
exist(char *file)
{
	struct stat statbuf;
	char filename[512], *pt_sep;
#ifdef FITS
	int exist_extension(char *);
#endif
	strcpy(filename,file);
#ifdef FITS
	pt_sep = strchr(filename,'[');
	if (pt_sep != NULL) {	/* Fits file extension */
		return (exist_extension(file));
	}
	else {
#endif
		return (stat(filename, &statbuf) < 0 ? 0 : 1);
#ifdef FITS
	}
#endif
}

/*-----------------------------------------------------------------------------
!
!.func                             get_path()
!
!.purp                       remove path from filename
!.desc
! char *get_path(file)
!
! char *file;           filename
!.ed
------------------------------------------------------------------------------*/

char *get_path(char *file) {

	char *pt_slash, *path;
	char buffer[512];

	strcpy(buffer,file);
	pt_slash = strrchr(buffer,'/');
	if (pt_slash == NULL)	/* no path */
		strcpy(buffer,".");
	else 
		*pt_slash = 0;
	path = (char *)malloc((strlen(buffer)+1)*sizeof(char));
	strcpy(path,buffer);
	return(path);
}

/*-----------------------------------------------------------------------------
!
!.func                             remove_path()
!
!.purp                       remove path from filename
!.desc
! int remove_path(file)
!
! char *file;           filename
!.ed
------------------------------------------------------------------------------*/

int remove_path(char *file) {

	char *pt_slash, *basename;

	pt_slash = strrchr(file,'/');
	if (pt_slash == NULL)	/* no path */
		return 0;
	basename = strdup(pt_slash+1);  /* make a copy... */
	strcpy(file,basename);          /* ... as strings may not overlap */
	free(basename);    
	return 0;
}

/*-----------------------------------------------------------------------------
!
!.func                             f_lines()
!
!.purp              count number of lines in file
!.desc
! int f_lines(file)
!
! char *file;           filename
!.ed
------------------------------------------------------------------------------*/

#define BSIZ 4096

int f_lines(char *file) 
{
    int l_cnt=0,cnt;

    FILE *fd;
    unsigned long tcnt = 0;
    unsigned long was_sp = 1;
    unsigned char *pp, *pe;

    unsigned long ws[256];
    char buff[BSIZ];

 /* Fill tables */
    for (cnt = 0; cnt < 256; cnt++) ws[cnt] = 0;

    /* also: ws['\r']=ws['\v']=ws['\f']= */
    ws[' ']=ws['\t']=ws['\n']=1;
    ws['\n']=65536 + 1;

    /* Main loop */
    fd = fopen(file,"r");
    if (fd == NULL) return(-1);
    while((cnt=fread(buff,1,BSIZ,fd))) {
	if (cnt < 0) break;

        tcnt = 0;

	pp = buff;
	pe = buff + cnt;

	while(pp < pe) {
	    tcnt += ws[*pp] ^ was_sp;
	    was_sp = ws[*pp] & 0xFFFF;
	    pp ++;
	}
	l_cnt += tcnt >> 16;
    }
    fclose(fd);
    return l_cnt;
}

/*-----------------------------------------------------------------------------
!
!.func                        read_DOS()
!
!.purp               reads a numerical value from DOS binaries
!.desc
! int read_DOS(imno,data,size)
!
! int imno;             file number
! (type *) data;        pointer on data
! int size;             size of item (in bytes)
!.ed
------------------------------------------------------------------------------*/
int read_DOS(int imno, char *data, int size)
{
	int fd_no = imno;
	long lsize = size;
	char ctmp;
	long i, nb_w;

	read(fd_no,data,lsize);
	if (size < 2)
		return(-1);
	for (i=0;i<size;i+=2) {
		ctmp = data[i];
		data[i] = data[i+1];
		data[i+1] = ctmp;
	}
	nb_w = size/2;
	for (i=0;i<nb_w/2;i++) {
		ctmp = data[i*2];
		data[i*2] = data[(nb_w-1-i)*2];
		data[(nb_w-1-i)*2] = ctmp;
		ctmp = data[i*2+1];
		data[i*2+1] = data[(nb_w-1-i)*2+1];
		data[(nb_w-1-i)*2+1] = ctmp;
	}
	return(0);
}

/*-----------------------------------------------------------------------------
!
!.func                        write_DOS()
!
!.purp               write a numerical value to DOS binaries
!.desc
! int write_DOS(imno,data,size)
!
! int imno;             file number
! (type *) data;        pointer on data
! int size;             size of item (in bytes)
!.ed
------------------------------------------------------------------------------*/
int write_DOS(int imno, char *truedata, int size)
{
	int fd_no = imno;
	long lsize = size;
	char ctmp, data[8];
	long i, nb_w;

	memcpy(data,(char *)truedata,size);

	if (size < 2)
		return(-1);
	for (i=0;i<size;i+=2) {
		ctmp = data[i];
		data[i] = data[i+1];
		data[i+1] = ctmp;
	}
	nb_w = size/2;
	for (i=0;i<nb_w/2;i++) {
		ctmp = data[i*2];
		data[i*2] = data[(nb_w-1-i)*2];
		data[(nb_w-1-i)*2] = ctmp;
		ctmp = data[i*2+1];
		data[i*2+1] = data[(nb_w-1-i)*2+1];
		data[(nb_w-1-i)*2+1] = ctmp;
	}
	write(fd_no,data,lsize);
	return(0);
}

