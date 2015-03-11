SOURCES = AbstractRelogoAgent.cpp \
AgentSet.cpp \
creators.cpp \
Observer.cpp \
Patch.cpp \
RandomMove.cpp \
RelogoAgent.cpp \
RelogoContinuousSpaceAdder.cpp \
RelogoDiscreteSpaceAdder.cpp \
RelogoLink.cpp \
Turtle.cpp \
utility.cpp \
WorldCreator.cpp \
WorldDefinition.cpp

         

local_dir := relogo
local_src := $(addprefix $(local_dir)/, $(SOURCES))

relogo_src += $(local_src)



