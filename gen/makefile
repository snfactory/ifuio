topdir = ..
include $(topdir)/makedefs

OBJ = $(O)/dyn_alloc.o $(O)/strings.o $(O)/files.o $(O)/julian.o

all : $(GENLIB) 

$(GENLIB) : $(OBJ)
	$(AR) rv $(GENLIB) $(OBJ) 
	$(RANLIB) $@

$(OBJ) : $(O)/%.o : %.c $(HEADERS) $(O)
	$(C_COMPILE)

$(O) :
	mkdir $(O)

clean :
	$(RM) $(OBJ) $(GENLIB)

.PHONY : clean all
