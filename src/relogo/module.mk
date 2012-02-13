SOURCES = Observer.cpp \
          Patch.cpp \
          Turtle.cpp \
          RelogoAgent.cpp \
          AbstractRelogoAgent.cpp \
          RelogoGridAdder.cpp \
          RelogoSpaceAdder.cpp \
          WorldDefinition.cpp \
          WorldCreator.cpp \
          RelogoLink.cpp \
          utility.cpp \
          creators.cpp \
          RandomMove.cpp
         

local_dir := src/relogo
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)



