SOURCES = main.cpp \
          RumorModel.cpp
         
local_dir := src/rumor_model
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)
bin_dir = "bin/rumor"
program = "rumor_model"



