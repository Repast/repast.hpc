SOURCES = main.cpp \
          model.cpp \
          rscpp_mpi_tests.cpp
         

local_dir := test/mpi
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)
bin_dir = "bin/mpi_unit_tests"
program = "mpi_unit_test"



