SOURCES = context_test.cpp \
          grid_comp_test.cpp \
          grid_test.cpp \
          main.cpp \
          properties_test.cpp \
          random_test.cpp \
          schedule_test.cpp \
          value_layer_tests.cpp 

local_dir := test/core
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)
bin_dir = "bin/unit_tests"
program = "unit_test"



