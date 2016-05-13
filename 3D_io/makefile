topdir = ..
include $(topdir)/makedefs
#    

LIB = $(O)/libio.a 

OBJ = $(O)/3D_iolib.o $(O)/compat.o

OUT : dirobj $(LIB)

$(LIB) : $(OBJ)
	ar rv $(LIB) $(OBJ)
	$(RANLIB) $@
