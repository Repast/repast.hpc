program :=
sources :=
libraries :=
extra_clean :=
bin_dir :=

CXX = /usr/local/bin/mpicxx
BOOST_INCLUDE = /usr/local/include/boost/
LIBS = -lboost_mpi-xgcc42-mt-1_39 -lboost_serialization-xgcc42-mt-1_39 -lboost_filesystem-xgcc42-mt-1_39 -lboost_system-xgcc42-mt-1_39 -lnetcdf -lcurl -lnetcdf_c++ -lmpi -lgtest

objects = $(subst .cpp,.o,$(sources))
dependencies = $(subst .cpp,.d,$(sources))

include_dirs := $(BOOST_INCLUDE)  src
CPPFLAGS += $(addprefix -I, $(include_dirs)) -O0 -g3 -Wall -c -fmessage-length=0
#vpath %.h $(include_dirs)

RM := rm -f
SED := sed
MV := mv -f

#%.o : %.cpp
#	$(CC) $(CPPFLAGS) -o "$@" "$<"

include src/repast_hpc/module.mk

ifeq "$(MAKECMDGOALS)" "unit_tests"
 include test/core/module.mk
endif

ifeq "$(MAKECMDGOALS)" "mpi_unit_tests"
 include test/mpi/module.mk
endif

ifeq "$(MAKECMDGOALS)" "rumor_model"
 include src/rumor_model/module.mk
endif

ifeq "$(MAKECMDGOALS)" "zombies"
 include src/relogo/module.mk
 include src/zombies/module.mk
endif

ifeq "$(MAKECMDGOALS)" "relogo_unit_tests"
 include src/relogo/module.mk
 include test/relogo/module.mk
endif

ifneq "$(MAKECMDGOALS)" "clean"
 include $(dependencies)
endif

.PHONY: unit_tests mpi_unit_tests relogo_unit_tests rumor_model zombies

rumor_model: $(objects)
	-mkdir -p $(bin_dir)
	$(CXX) -o $(bin_dir)/$(program) $^ $(LIBS)
	
zombies: $(objects)
	-mkdir -p $(bin_dir)
	$(CXX) -o $(bin_dir)/$(program) $^ $(LIBS)

unit_tests: $(objects)
	-mkdir -p $(bin_dir)
	$(CXX) -o $(bin_dir)/$(program) $^ $(LIBS)
	
mpi_unit_tests: $(objects)
	-mkdir -p $(bin_dir)
	$(CXX) -o $(bin_dir)/$(program) $^ $(LIBS)

relogo_unit_tests: $(objects)
	-mkdir -p $(bin_dir)
	$(CXX) -o $(bin_dir)/$(program) $^ $(LIBS)
	
.PHONY: all
all: $(objects)
	$(CXX) -o unit_test $^ $(LIBS)
	
.PHONY: clean
clean:
	$(RM) $(objects) $(program) $(libraries) \
	$(dependencies) $(extra_clean)


ifeq "$(MAKECMDGOALS)" "clean"
 include test/relogo/module.mk
 include test/core/module.mk
 include test/mpi/module.mk
 include src/relogo/module.mk
 include src/rumor_model/module.mk
 include src/zombies/module.mk
endif

%.d: %.cpp
	$(CXX) $(CPPFLAGS) -M $< | \
	$(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	$(MV) $@.tmp $@
	
#$(CXX) $(CPPFLAGS) -v -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"  "$<"
