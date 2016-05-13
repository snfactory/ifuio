topdir = ..
include $(topdir)/makedefs   
#

EXE = $(E)/check_spec_io $(E)/check_ima_io $(E)/check_table_io \
      $(E)/check_catalog_io $(E)/check_E3D_io $(E)/check_tiger_io \
      $(E)/check_cp_desc $(E)/check_args_parsing \
      $(E)/check_descr_list $(E)/check_cube_io \
      $(E)/check_file_format $(E)/check_flip_ima $(E)/check_selection \
      $(E)/check_ima_ext_io $(E)/check_cplx2D_io

OUT : dirbin dirobj $(EXE)

#
# EXECUTABLES
#

$(E)/check_flip_ima : $(O)/check_flip_ima.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_flip_ima.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_descr_list : $(O)/check_descr_list.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_descr_list.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_tiger_io : $(O)/check_tiger_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_tiger_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_E3D_io : $(O)/check_E3D_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_E3D_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_catalog_io : $(O)/check_catalog_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_catalog_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_spec_io : $(O)/check_spec_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_spec_io.o $(IOLIB) $(ADD_LIBS) 
	$(STRIP) $@

$(E)/check_ima_io : $(O)/check_ima_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_ima_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_ima_ext_io : $(O)/check_ima_ext_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_ima_ext_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_cube_io : $(O)/check_cube_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_cube_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_table_io : $(O)/check_table_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_table_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_args_parsing : $(O)/check_args_parsing.o $(STAND_INC) $(IOLIB) 
	$(LD) $(LDF) -o $@ $(O)/check_args_parsing.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_cp_desc : $(O)/check_cp_desc.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_cp_desc.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_file_format : $(O)/check_file_format.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_file_format.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_selection : $(O)/check_selection.o $(STAND_INC) $(TIGLIB) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_selection.o $(TIGLIB) $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@

$(E)/check_cplx2D_io : $(O)/check_cplx2D_io.o $(STAND_INC) $(IOLIB)
	$(LD) $(LDF) -o $@ $(O)/check_cplx2D_io.o $(IOLIB) $(ADD_LIBS)
	$(STRIP) $@
