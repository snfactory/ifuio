topdir = ..
include $(topdir)/makedefs   

SRC = check_spec_io.c check_ima_io.c check_table_io.c \
      check_catalog_io.c check_E3D_io.c check_tiger_io.c \
      check_cp_desc.c check_args_parsing.c \
      check_descr_list.c check_cube_io.c \
      check_file_format.c check_flip_ima.c check_selection.c \
      check_ima_ext_io.c check_cplx2D_io.c

OBJ = $(patsubst %.c, $(O)/%.o, $(SRC))
EXE = $(patsubst %.c, $(E)/%, $(SRC))

all : $(EXE)

$(O)/%.o : %.c $(HEADERS)
	$(C_COMPILE)

$(E)/% : $(O)/%.o $(LIB) $(E)
	$(C_LINK)
	$(STRIP) $@

$(E) :
	mkdir $(E)

clean :
	$(RM) $(OBJ) $(EXE)
	-rmdir $(E)

.PHONY : all clean
