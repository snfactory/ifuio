topdir = ..
include $(topdir)/makedefs

OBJ = $(O)/dyn_alloc.o $(O)/strings.o $(O)/files.o $(O)/julian.o

LIB = $(O)/libgen.a 

OUT : dirobj $(LIB)

$(LIB) : $(OBJ)
	ar rv $(LIB) $(OBJ)
	$(RANLIB) $@
