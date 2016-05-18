# This makefile actually works on both this directory and 3D_io.

topdir = ..
include $(topdir)/makedefs    

SRC = iolib.c \
      io_error.c \
      io_utils.c \
      io_misc.c \
      parse_arg.c \
      decode.c \
      convert.c \
      version.c \
      fits_primary_hd.c \
      parse_wcs.c \
      complex_io.c

3D_SRC = $(topdir)/3D_io/3D_iolib.c $(topdir)/3D_io/compat.c

OBJ = $(patsubst %.c, $(O)/%.o, $(SRC))
3D_OBJ = $(patsubst ../3D_io/%.c, $(O)/%.o, $(3D_SRC))

all : $(IOLIB) 

$(IOLIB) : $(OBJ) $(3D_OBJ)
	$(AR) rv $(IOLIB) $(OBJ) $(3D_OBJ)
	$(RANLIB) $@

$(OBJ) : $(O)/%.o : %.c $(HEADERS) $(O)
	$(C_COMPILE)

$(3D_OBJ) : $(O)/%.o : ../3D_io/%.c $(HEADERS) $(O)
	$(C_COMPILE)

$(O) :
	mkdir $(O)

clean :
	$(RM) $(OBJ)
	$(RM) $(IOLIB)

.PHONY : clean all
