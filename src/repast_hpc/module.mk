SOURCES = AgentId.cpp \
AgentImporterExporter.cpp \
AgentRequest.cpp \
AgentStatus.cpp \
CartesianTopology.cpp \
Graph.cpp \
GridComponents.cpp \
GridDimensions.cpp \
initialize_random.cpp \
io.cpp \
logger.cpp \
NCDataSet.cpp \
NCDataSetBuilder.cpp \
NetworkBuilder.cpp \
Properties.cpp \
Random.cpp \
RelativeLocation.cpp \
RepastErrors.cpp \
RepastProcess.cpp \
Schedule.cpp \
SharedBaseGrid.cpp \
SharedContext.cpp \
spatial_math.cpp \
SRManager.cpp \
SVDataSet.cpp \
SVDataSetBuilder.cpp \
Utilities.cpp \
ValueLayer.cpp \
ValueLayerND.cpp \
Variable.cpp \
ValueLayer.cpp \
Variable.cpp 

local_dir := repast_hpc
local_src := $(addprefix $(local_dir)/, $(SOURCES))

repast_hpc_src += $(local_src)
