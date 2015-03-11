SOURCES = main.cpp \
          RumorModel.cpp
         
local_dir := rumor_model
local_src := $(addprefix $(local_dir)/, $(SOURCES))

rumor_src += $(local_src)



