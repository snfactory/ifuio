topdir = .
include $(topdir)/makedefs

all : std_io gen checklib

std_io gen :
	$(MAKE) -C $@

checklib : std_io gen
	$(MAKE) -C $@

# rmdir $(O) is here because it is written to from multiple subdirs
clean :
	cd std_io && $(MAKE) clean
	cd gen && $(MAKE) clean
	cd checklib && $(MAKE) clean
	-rmdir $(O)

.PHONY : checklib std_io gen all clean
