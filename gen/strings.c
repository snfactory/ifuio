/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT   (c) 1993 Observatoire de Lyon - St Genis Laval (France)
! IDENT       strings.c
! LANGUAGE    C
! AUTHOR      A. Rousset
! KEYWORDS    None
! PURPOSE     Facilities for character strings manipulation
! VERSION     4.0  1993-May-07 : Creation, AR    
! VERSION     4.1  2004-Nov-22 : last_char_before, YC
---------------------------------------------------------------------*/

#include <strings.h>
#include <string.h>
#include <ctype.h>

/*-----------------------------------------------------------------------------
!
!.blk                  Miscellaneous routines to handle strings
!
!.func                             first_blk()
!
!.purp           returns (sets end of string at) the position of the first 
!                        blank character in string
!.desc
! int first_blk(string)
!
! char *string;         string 
!.ed
------------------------------------------------------------------------------*/

int 
first_blk(char *string)
{
  long n;

  n = strcspn(string, " \t\n");
  if (n >= 0 && n < strlen(string))	
    string[n] = '\0';
  return(n);
}

/*-----------------------------------------------------------------------------
!
!.func                             last_char()
!
!.purp			returns the position of the last non white character in string
!				and sets end of string
!.desc
! int last_char(string)
!
! char *string;         string 
!.ed
------------------------------------------------------------------------------*/

int 
last_char(char *string)
{
  long    n;

  for (n=strlen(string)-1;string[n] == ' ' && n>=0; n--)
    string[n] = '\0';
  return(n+1);
}

/*-----------------------------------------------------------------------------
!
!.func                             last_char_before()
!
!.purp			returns the position of the last non white character before first 
!                               specified character in string and sets end of string
!.desc
! int last_char_before_eq(string)
!
! char *string;         string 
!.ed
------------------------------------------------------------------------------*/

int 
last_char_before(char *string, char c)
{
  char *stop;
  long n;

  if ((stop = strchr(string,c)) != NULL) 
    *stop = '\0';                            /* Cut at 1st occurence of char if any */
  for (n = strlen(string)-1; string[n] == ' ' && n>=0; n--); /* Look at last blank... */
  string[++n] = '\0';                        /* ... and cut there */
  return(n+1);                               /* Return new string length */
}

/*-----------------------------------------------------------------------------
!
!.func                             fill_blk()
!
!.purp                append blank characters to string
!.desc
! int fill_blk(string,nb_char)
!
! char *string;         string 
! int  nb_char;         number of char. in returned string
!.ed
------------------------------------------------------------------------------*/

int 
fill_blk(char *string, int nb_blk)
{
  long    i,n;

  n = strlen(string);
  for (i=0; (i+n) <nb_blk; i++)
    string[n+i] = ' ';
  return(0);
}

/*-----------------------------------------------------------------------------
!
!.func                             string_compar()
!
!.purp          returns the common sub-string length of two strings
!.desc
! int string_compar(string1,string2)
!
! char *string1;        string 
! char *string2;        string 
!.ed
------------------------------------------------------------------------------*/

int 
string_compar(char *string1, char *string2)
{
  char *pt1, *pt2;
  long n;

  for (n = 0, pt1 = string1, pt2 = string2; *pt1 == *pt2 && *pt1 != '\0'; 
       pt1++, pt2++, n++);
  return(n);
}

/*-----------------------------------------------------------------------------
!
!.blk                    Uppercase/lowercase conversion
!
!.func                            is_upper_string()
!
!.purp          returns true if all characters in string are uppercase char.
!.desc
! int is_upper_string(string)
!
! char *string;         string 
!.ed
------------------------------------------------------------------------------*/

int 
is_upper_string(char *string)
{
  char *pt;

  for (pt = string; *pt != '\0'; pt++) {
    if (islower(*pt)) return(0); 
  }
  return(1);
}


/*-----------------------------------------------------------------------------
!
!.func                            is_lower_string()
!
!.purp          returns true if all characters in string are lowercase char.
!.desc
! int is_lower_string(string)
!
! char *string;         string 
!.ed
------------------------------------------------------------------------------*/

int 
is_lower_string(char *string)
{
  char   *pt;

  for (pt = string; islower(*pt) && *pt != '\0'; pt++);
  if (*pt != '\0') return(0);
  else             return(1);
}

/*-----------------------------------------------------------------------------
!
!.func                             lower_strg()
!
!.purp              converts given string in lowercase format
!.desc
! void lower_strg(string)
!
! char *string;         string to convert
!.ed
------------------------------------------------------------------------------*/

void 
lower_strg(char *string)
{
  int len, i;

  if (is_lower_string(string)) return;

  len = strlen(string);
  for (i=0; i<len; i++) string[i] = tolower(string[i]);

  return;
}

/*-----------------------------------------------------------------------------
!
!.func                             upper_strg()
!
!.purp              converts given string in uppercase format
!.desc
! void upper_strg(string)
!
! char *string;         string to convert
!.ed
------------------------------------------------------------------------------*/

void 
upper_strg(char *string)
{
  int len, i;

  if (is_upper_string(string)) return;

  len = strlen(string);
  for (i=0; i<len; i++) string[i] = toupper(string[i]);

  return;
}
/*--------------------------------------------------------------------
!
!.func                     is_numeric()
!
!.purp          check is string is numeric or alphanumeric
!.desc
! is_numeric (string)
!
! char *string;        string
!.ed
----------------------------------------------------------------------*/

int 
is_numeric(char *str) {

  int len, i, status = 1;
  char *pt = str;

  len = strlen(str);

  for (i=0; i< strlen(str) -1; i++, pt++) {
    if (!isdigit(*pt)) {
      if (*pt != '.') {
        status = 0;
        break;
      }
    }
  }
  return(status);
}
