/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (France)
! IDENT        IFU_datatypes.h
! LANGUAGE     C
! AUTHOR       A. Rousset
! PURPOSE      Data type definitions for array/memory allocations
! COMMENT      
! VERSION      4.0  1992-Month-Day : Creation    
! VERSION      4.1  2004-11-18 (YC): use enum for debug purpose, test _ALLOC_H
! VERSION      4.2  2004-12-22 (AP): renamed to avoid conflict with C++ includes
______________________________________________________________________________*/

#ifndef _IFU_DATATYPES_H
#define _IFU_DATATYPES_H

#ifdef DO_NOT_USE_ENUM
#define CHAR	(short)99  /* 'c' */
#define SHORT	(short)115 /* 's' */
#define USHORT	(short)117 /* 'u' */
#define INT  	(short)105 /* 'i' */
#define LONG	(short)108 /* 'l' */
#define FLOAT   (short)102 /* 'f' */
#define DOUBLE  (short)100 /* 'd' */
#define STRING	(short)83  /* 'S' */
#else
enum data_format {
  CHAR   = 99,                               /* 'c' */
  SHORT  = 115,                              /* 's' */
  USHORT = 117,                              /* 'u' */
  INT    = 105,                              /* 'i' */
  LONG   = 108,                              /* 'l' */
  FLOAT  = 102,                              /* 'f' */
  DOUBLE = 100,                              /* 'd' */
  STRING = 83                                /* 'S' */
};
#endif

#endif /* _IFU_DATATYPES_H */
