topdir = ..
include $(topdir)/makedefs    
#

OBJ = $(O)/iolib.o $(O)/io_error.o $(O)/io_utils.o $(O)/io_misc.o \
      $(O)/parse_arg.o $(O)/decode.o $(O)/convert.o $(O)/version.o \
      $(O)/fits_primary_hd.o $(O)/parse_wcs.o $(O)/complex_io.o

LIB = $(O)/libio.a

OUT : dirobj $(LIB) 

$(LIB) : $(OBJ)
	ar rv $(LIB) $(OBJ) 
	$(RANLIB) $@     
