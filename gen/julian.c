/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c) CFHT 
! IDENT        julian.c
! LANGUAGE     C
! AUTHOR       A. Pecontal
! KEYWORDS     None
! PURPOSE      Julian date conversion routines from CFHT
! VERSION      1.0  2005-Apr-11 : Creation, AP
______________________________________________________________________________*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>		  /* for getenv() */

/*
 * machine has only finite precision, so we use this to avoid translating
 * 119.999 seconds into (1 minute, 60 seconds) instead of (2 minutes 0 seconds)
 */
#define EPSILON 0.0

/*-----------------------------------------------------------------------------
!
!.blk           Miscellaneous routines to handle julian date
!
!.func                             julian()
!
!.purp      return a double which is the Julian date of the date passed in
!.desc
!
! double jd = julian(year, month, day, hour, minute, sec)
!
! int year;     calendar year (for example, 1990)
! int month;    month of year, [1,12]
! int day;      day of month, [1,31]
! int hour;     hour of day, [0,23]
! int minute;   minute of hour, [0,59]
! double sec;   second of minute, [0.0,59.999]
!
! double jd;    julian date, including fractional part for seconds into day
!.ed
------------------------------------------------------------------------------*/

double julian(int year,int month,int day,int hour,int minute,double sec)
{
/*
*    This routine will return a double which is the Julian date of the date
*    passed in, something on the order of 2400000.  Remember to input
*    UT values and not HST values.  Note that years procede ...-2, -1, 0,
*    1, 2,...  Calendar reform (i.e. switch from julian calendar to
*    gregorian calendar) is accounted for.
*/

    int a,b;
    int y,m;
    double jd;

    if (month <= 2) {
        y = year - 1;
        m = month + 12;
    } else {
        y = year;
        m = month;
    }

    /*
     * figure the julian date based on the julian calendar
     */
    jd = 1720994.5;        /* -1 Oct 30 0:00:00 */
    jd += (double)( (int)((double)y * 365.25 - (y<0 ? 0.75 : 0.0)) +
                (int)((double)(m+1) * 30.6001                ) + day);
    jd += (((double)hour*3600.0) + ((double)minute*60.0) + sec) / 86400.0;

    /*
     * If we are in the gregorian calendar and not the julian
     * calendar, we need to adjust.
     */
    if ((year > 1582) || 
        ((year == 1582) && (month > 10)) ||
        ((year == 1582) && (month == 10) && (day >= 15))) {
        a = y / 100;
        b = 2 - a + (a / 4);
        jd += (double)b;
    }

    return (jd);
}

/*-----------------------------------------------------------------------------
!
!.func                             fromjulian()
!
!.purp      converts from a julian date back into y/m/d/h/m/s form
!.desc
!
! fromjulian(jd, year, month, day, hour, minute, sec)
!
! double jd;     julian date to be converted
! int *year;     calendar year (for example, 1990)
! int *month;    month of year, [1,12]
! int *day;      day of month, [1,31]
! int *hour;     hour of day, [0,23]
! int *minute;   minute of hour, [0,59]
! double *sec;   second of minute, [0.0,59.999]
!
!.ed
------------------------------------------------------------------------------*/

void fromjulian(double jd,int *year,int *month,int *day,int *hour,int *minute,double *sec)
{
    long z,a,alpha,b,c,d,e;
    double f,q;

    if (jd < 0) {
        *year = *month = *day = *hour = *minute = 0;
        *sec = 0.0;
    } else {
        jd += 0.5;
        z = (long)jd;
        f = jd - (double)z;
        if (z < 2299161) {
            a = z;
        } else {
            alpha = (long)(((double)z - 1867216.25) / 36524.25);
            a = z + 1 + alpha - (alpha/4);
        }
        b = a + 1524;
        c = (long)(((double)b - 122.1) / 365.25);
        d = (long)((double)c * 365.25);
        e = (long)(((double)b - (double)d) / 30.6001);
        q = (double)(b - d - (long)((double)e * 30.6001)) + f;
        *month = (e<14 ? (int)(e-1) : (int)(e-13));
        *year = (*month<3 ? (int)(c-4715) : (int)(c-4716));
        *day = (int)q;
        q = (q - (double)*day) * 86400.0;
        *hour = (int)((q / 3600.0) + EPSILON);
        q -= (double)*hour * 3600.0;
        *minute = (int)((q / 60.0) + EPSILON);
        *sec = q - ((double)*minute * 60.0);
    }
}

/*-----------------------------------------------------------------------------
!
!.func                             Mjulian()
!
!.purp      return a string of the current modified julian date suitable for stamping
!.desc
!
! result_string = Mjulian();
!
! char *result_string;  pointer to string containing date
!
!.ed
------------------------------------------------------------------------------*/

#define     E_TZONE     "TZ"

char *Mjulian(long *clockp)
{

    double mjd;
    long t;
    struct tm *tmd;
    static char rtn[12];

    if(clockp == (long *) NULL) { /* if not passed in - get */
        clockp = &t;
        time(clockp);
    }
    tmd = gmtime(clockp);

/* 
*    the modified julian date is defined as:
*    Mod julian date = julian date - 2400000.5
*/

    mjd = julian(1900+tmd->tm_year,
                      tmd->tm_mon+1    ,
                      tmd->tm_mday     ,
                      tmd->tm_hour     ,
                      tmd->tm_min      ,
                      (double) tmd->tm_sec ) - 2400000.5;

    sprintf(rtn,"%.6f", mjd); 
    return(rtn);
}

