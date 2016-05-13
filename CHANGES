** version 6.0d

14/05/2004 AP Changed file convert.c to handle noise spectra when converting datacubes
25/05/2004 YC Changed file convert.c: added noise interpolation for Tiger format export
26/05/2004 AP Changed column name for data quality (Euro3D format)
11/06/2004 AP fix bug of file opening mode when retrieving associated table to datacube
14/06/2004 AP added close_primary_hd to release memory/file descriptor associated to file
22/06/2004 AP added test for file opening mode when trying to write a descriptor
22/06/2004 AP fix bug in spectrum end computation in function open_E3D_file
30/06/2004 AP small fix in reading keyword so that hierachical keywords are now handle also
01/07/2004 AP fix WR_desc when keywords have synonyms
16/07/2004 AP get_assoc_table_name removed
16/07/2004 AP new functions get_coord_table_name and get_science_table_name
27/07/2004 AP fix bug in close_E3D_file (stat not initialised may cause problem)
27/07/2004 AP lg_name increased to 256 (to allow long filenames)
30/07/2004 AP included IFU I/O tcl wrapper
03/08/2004 AP fixed bug in file_format to detect Euro3D files
05/08/2004 AP fixed bug in reading FITS format : CRPIX may contain real values (not only integer)
09/08/2004 AP lg_name turned to 80 (create bugs when reading tigerfile). To be changed later on
01/09/2004 AP create_tiger_frame now sets table_name before tigerfile header writing
02/09/2004 AP get_col_ref : removed some crossref to E3D columns
02/09/2004 AP fix bug in translating Euro3D datacube to tiger format
09/09/2004 AP genlib renamed to libgen to comply to unix standard (needed by some tools)
14/09/2004 AP configure now checks for includes files before compiling wrappers
17/09/2004 AP included IFU I/O python wrapper
17/09/2004 AP changed wrappers path
20/09/2004 AP added typemaps for python wrapper (WR_desc, RD_desc, WR_tbl, RD_tbl)
23/09/2004 AP added numarray-flavor access to data in the python wrapper
24/09/2004 AP fix bug in RD_desc typemap  for python several returns needed at once
24/09/2004 AP fix bug RD_col typemap for python wrapper
24/09/2004 AP Euro3D keyword CRDELTS renamed to CDELTS to comply to format document 1.2
24/09/2004 AP fix Euro3D GROUP_N column format
27/09/2004 AP fix exist_statistical_error for Euro3D datacubes
27/09/2004 AP added selection capabilitiy to E3D datacubes
30/09/2004 AP added new fclasses (Snifs requirement)
30/09/2004 AP soft_version.h removed. Version is now inheritated from autoconf VERSION definition
01/10/2004 AP tiger_def.h removed. Not relevant to that package
04/10/2004 AP lg_name increased to 256 (fixed for Tiger datacubes)
04/10/2004 AP added wcslib
08/10/2004 AP fixed bug in python numarray wrapper (dimensions swapped)
08/10/2004 AP wcs parsing included in spectrum, 2D and 3D frames

** version 6.1

11/10/2004 AP updated cfitsio version to 2.5
15/10/2004 AP added wcs parsing for Euro3D datacube
15/10/2004 AP E3D_file I/O added to tcl and python wrappers
25/10/2004 AP fixed bug into get_E3D_frame (wrong step value returned)
26/10/2004 AP update of the installation guide (v1.0)
26/10/2004 AP fixed bug into get_E3D_frame when noise isn't available
26/10/2004 AP fixed bug in writing hierarchical keywords
27/10/2004 AP MacOS compatibility for dynamic loading (for wrappers)

** version 6.1a

27/10/2004 AP configure.in converted to configure.ac 
27/10/2004 AP configure now create IFU_io_config.h including preprocessor definitions
27/10/2004 AP added flex detection in configure.ac (needed by wcslib)
28/10/2004 AP midas disabled for E3D if not specified as a configure option (--enable-midas/--disable midas)
24/11/2004 YC added function last_char_before
24/11/2004 YC added enum definitions for data type
24/11/2004 YC fixed memory allocation problem in CP_non_std_desc
06/12/2004 AP fixed bug in create_E3D_file, passing table_name from create_tiger_frame
08/12/2004 AP added synonym for open_fits_primary_hd (open_primary_hd)
08/12/2004 AP fix bug writing FITS string into columns
13/12/2004 AP removed quotes around io library version number
13/12/2004 AP in get_path replace ./ by . (no leading / in path)
15/12/2004 AP fixed bug for wcslib : flex prefered to lex when available
17/12/2004 AP fixed bug in python wrapper : RD_tbl for double

** version 6.2

22/12/2004 AP alloc.h renamed to IFU_datatypes to avoid conflict with C++ standard includes
22/12/2004 AP now STAND_INC only stands for IFU_io.h which make include renaming transparent for other packages (no more changes in makedefs)

** version 6.3

03/01/2005 AJ python includes detection improved in configure.ac
05/01/2005 AP improved file_format
06/01/2005 AP & YC new functions open_anyfile and close_anyfile
06/01/2005 AP suppressed generic types IMA_TYPE and TBL_TYPE
10/01/2005 AP iolib compiled in -O2 as default
20/01/2005 AP added open_primary_hd and close_primary_hd prototypes
21/01/2005 AP added --disable-wrappers option to configure.ac
28/01/2005 AP added --enable-static-link option to configure.ac
02/02/2005 AP malloc.h no more included in gendef.h for darwin systems
02/02/2005 AP values.h no more included for darwin systems
03/02/2005 AP group numbers are always coded as int32
09/02/2005 AP avoid FITS keyword duplication using WR_desc
10/02/2005 AP added new 2D complex image type stored as two FITS extensions
02/03/2005 AP C arrays python wrapping
11/03/2005 AP python include path properly handled in python_wrapper makefile
14/03/2005 YC configure.ac update (messages handling)
12/04/2005 AP added routines for julian date handling
13/04/2005 AP fixed bug in get_science_tablename (path forgot)
14/04/2005 YC fixed bug in file deletion confirmation
09/05/2005 AP fixed bug for conflicting NINT definition
09/05/2005 AP fixed bug while printing error (quiet mode)
20/06/2005 AP fixed bug when saving hierarchical keyword in tiger files
06/09/2005 AP fixed bug when getting groups from an opened tiger file
16/09/2005 YC fixed memory alloc bugs in interpolate noise and remove_path
16/09/2005 AP fixed bug: file_type was not set for datacubes
10/10/2005 AP fixed bug: CP_non_std_desc now handle Euro3D files
15/11/2005 AP when building without midas I/O don't include midas I/O dir when compiling
16/11/2005 AP fixed compilation warning on python wrapper (WR_qslice)
21/11/2005 AP fixed bug printing software version in quiet mode

** version 6.4

18/01/2006 AP added type unsigned short for fits frames
