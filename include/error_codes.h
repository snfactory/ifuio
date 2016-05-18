/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        error_codes.h
! LANGUAGE     C
!
! AUTHOR       A.Rousset
!
! KEYWORDS     
! PURPOSE      general definitions about errors handling
! COMMENT      
! VERSION      1.0  1990-Jan-03 : Creation,   AR 
!
---------------------------------------------------------------------*/

/* code used to define programmer's preferred control level for error */

#define FATAL   0 /* default value */
#define WARNING 1
#define NONE    2

/* -------------------------- Tiger Error codes --------------------- */

#define OK               0  /* Successful return                      */
#define UNKNOWN         -1  /* Unknown                                */
#define ERR_EOF         -2  /* EOF indicator                          */
#define ERR_BAD_PARAM   -3  /* Input element is bad                   */
#define ERR_NB_PARAM    -4  /* Bad input number of elements           */
#define ERR_BAD_TYPE    -5  /* Invalid data type                      */
#define ERR_BAD_SIZE    -6  /* Field width not wide enough            */
#define ERR_OPEN        -10 /* Error opening existing file            */
#define ERR_CREAT       -11 /* Error opening new file                 */
#define ERR_READ        -12 /* Error reading file                     */
#define ERR_WRIT        -13 /* Error writing to file                  */
#define ERR_CLOSE       -14 /* Error closing existing file            */
#define ERR_ACCESS      -15 /* Invalid file access mode               */
#define ERR_NAXIS       -20 /* Invalid NAXIS parameter                */
#define ERR_NOIDENT     -21 /* Identifier not found                   */
#define ERR_OFFSET      -31 /* Error returning offset to data         */
#define ERR_NODATA      -32 /* No data available                      */
#define ERR_BAD_IMA     -41 /* Not an image !                         */
#define ERR_IMA_BOUND   -42 /* Bad section specification for image    */
#define ERR_IMA_EXT     -43 /* Bad extension for image                */
#define ERR_NOIMA       -44 /* Image does not exist                   */
#define ERR_DEL_IMA     -45 /* Error deleting image                   */
#define ERR_REN_IMA     -46 /* Error renaming image                   */
#define ERR_IMA_HEAD    -47 /* Illegal image header                   */
#define ERR_BAD_TBL     -51 /* Not a table !                          */
#define ERR_TBL_EXT     -53 /* Bad extension for table                */
#define ERR_NOTBL       -54 /* Table does not exist                   */
#define ERR_DEL_TBL     -55 /* Error deleting table                   */
#define ERR_REN_TBL     -56 /* Error renaming table                   */
#define ERR_TBL_HEAD    -57 /* Illegal table header                   */
#define ERR_BAD_COL     -59 /* Error in column format                 */
#define ERR_NOCOL       -60 /* Column does not exist                  */
#define ERR_COL_NUM     -62 /* Wrong column number                    */
#define ERR_ROW_NUM     -63 /* Wrong row number                       */
#define ERR_NODESC      -80 /* Header parameter not found             */
#define ERR_BAD_HEAD    -81 /* Illegal data type for header parameter */
#define ERR_DEL_DESC    -82 /* Cannot delete descriptor               */
#define ERR_BAD_DESC    -83 /* Descriptor bad                         */
#define ERR_HEADER_SIZE -85 /* Out of space in header                 */
#define ERR_BAD_CAT     -90 /* Not a catalog !                        */
#define ERR_GRAPH_DEV  -100 /* Bad graphics device                    */
#define ERR_ALLOC      -110 /* Error allocating dynamic memory        */
#define ERR_FREE       -111 /* Error freeing dynamic memory           */
#define ERR_OVERFLOW   -112 /* Overflow (column/frame)                */
#define ERR_FORMAT     -113 /* Unknown data format                    */
#define ERR_NOIMPL     -130 /* Not yet implemented                    */
