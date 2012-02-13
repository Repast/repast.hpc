SOURCES = main.cpp \
          ObserverTests.cpp \
          agent_set_tests.cpp
         
         

local_dir := test/relogo
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)
bin_dir = "bin/relogo_unit_tests"
program = "relogo_unit_test"



