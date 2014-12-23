SOURCES = AgentId.cpp \
NCDataSetBuilder.cpp \
SharedNetwork.cpp \
AgentImporterExporter.cpp \
NCDataSet.cpp \
spatial_math.cpp \
AgentRequest.cpp \
NetworkBuilder.cpp \
SRManager.cpp \
AgentStatus.cpp \
Properties.cpp \
SVDataSetBuilder.cpp \
Graph.cpp \
Random.cpp \
SVDataSet.cpp \
GridComponents.cpp \
RepastErrors.cpp \
Utilities.cpp \
GridDimensions.cpp \
RepastProcess.cpp \
ValueLayer.cpp \
initialize_random.cpp \
Schedule.cpp \
Variable.cpp \
io.cpp \
SharedBaseGrid.cpp \
logger.cpp \
SharedContext.cpp


local_dir := src/repast_hpc
local_src := $(addprefix $(local_dir)/, $(SOURCES))
cxx_flags := -fPIC
bin_dir := ./bin/lib
program := librepast_hpc-2.0.so

sources += $(local_src)
